#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>

#define BIT(n) (1 << (n))

struct cache_line {
  int valid_bit;
  int tag;
  int counter;
};

struct cache_line** cache;

int hit_count, miss_count, eviction_count;

int set_num, line_num, block_num, verbose;

const char* sd;

/**
 * parse the argument 
 * 
 * s: number of set
 * E: number of line
 * b: number of block
 * v: verbose flag
 * t: trace file
 */
void parse_argument(int argc, char *argv[]) {

  char opt;
    
  while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = 1;
        break;
      case 's':
        set_num = atoi(optarg);
        break; 
      case 'E':
        line_num = atoi(optarg);
        break;
      case 'b':
        block_num = atoi(optarg);
        break;
      case 't':
        sd = optarg;
        break;
      default:
        fprintf(stderr, "Error: not defined argument");
        exit(1);
    }
  }
}


/**
 * Initialize the cache
 */
void initialize_cache() {
  cache = (struct cache_line**)calloc(BIT(set_num+1), sizeof(struct cache_line*));
  for (int i = 0; i < BIT(set_num+1); ++i) {
    cache[i] = (struct cache_line*)calloc(line_num, sizeof(struct cache_line));
  }
}

// evict the cache
void lru(int tag, int set, int counter) {
  int min_count = INT_MAX, index = 0, i;
  for (i = 0; i < line_num; ++i) {
    if (cache[set][i].valid_bit == 0) {
      index = i;
      break;
    }

    if (cache[set][i].counter < min_count) {
      min_count = cache[set][i].counter;
      index = i;
    }
  }

  miss_count++;
  if (cache[set][index].valid_bit == 1) {
    eviction_count++;
  }

  cache[set][index].valid_bit = 1;
  cache[set][index].tag = tag;
  cache[set][index].counter = counter;
}

// count the hit/miss
void count(int tag, int set, int counter) {
  for (int i = 0; i < line_num; ++i) {
    if (cache[set][i].valid_bit == 1 && cache[set][i].tag == tag) {
      hit_count++;
      cache[set][i].counter = counter;
      return;
    }
  }

  lru(tag, set, counter);
}

int get_set_mask() {
  int mask = 0, i;
  for (i = 0; i < set_num; ++i) {
    mask = (mask << 1) | 1; 
  } 
  return mask;
}

int get_set(int address, int mask, int b) {
  return (address >>= b) & mask;
}

int get_tag(int address, int s, int b) {
  return address >> (b + s);
}

void free_memory() {
  int sets = 2 << set_num;
  for (int i = 0; i < sets; ++i) {
    free(cache[i]);
  }
}

// get trace from file
void read_trace() {
  FILE *fp;
  if ((fp = fopen(sd, "r")) == NULL) {
    fprintf(stderr, "Error: not valid file");
    exit(1);
  }
   
  int mask = get_set_mask(), counter = 1;
  char access_type;
  unsigned long address;
  int size;
  while (fscanf(fp, " %c %lx,%d", &access_type, &address, &size) > 0) {
    int tag = get_tag(address, set_num, block_num);
    int set = get_set(address, mask, block_num);
    switch(access_type) {
      case 'L':
        count(tag, set, counter++);
        break;
      case 'S':
        count(tag, set, counter++);
        break;
      case 'M':
        count(tag, set, counter++);
        // store, no need to do for second time
        hit_count++;
        break;
      default:
        break;
    }
  }

  fclose(fp);
}

int main(int argc, char *argv[])
{
  parse_argument(argc, argv);
  initialize_cache();
  read_trace();
  free_memory(cache);
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}

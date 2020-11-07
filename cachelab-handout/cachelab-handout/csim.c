#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct cache_line {
  int valid_bit;
  int tag;
  int counter;
}

struct node {
  
}

int hit_count, miss_count, eviction_count;

// Number of lines per set
int num_of_line;

// Numebr of block bits
int num_of_block;

// Number of set index bits
int num_of_set;

int verbose;

const char* sd;

const char* HELP_INFO = "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n  -h         Print this help message.\n  -v         Optional verbose flag.\n  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\n\nExamples:\n  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n";

// parse argument values
void parse_argument(int argc, char *argv[]) {

    char opt;
    
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stdout, "%s", HELP_INFO);
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                num_of_set = atoi(optarg);
                break;
            case 'E':
                num_of_line = atoi(optarg);
                break;
            case 'b':
                num_of_block = atoi(optarg);
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



// check argument input 
void check_input() {
    if (num_of_line <= 0 || num_of_set <= 0 || num_of_block <= 0) {
        fprintf(stderr, "Error: not valid input\n");
        exit(1);
    }
}

void initialize_cache(cache_line* cache) {
   check_input(); 


}

// get trace from file
void read_trace() {
    FILE *fp;
    if ((fp = fopen(sd, "r")) == NULL) {
      fprintf(stderr, "Error: not valid file");
      exit(1);
    }
    
    char access_type;
    unsigned long address;
    int size;
    while (fscanf(fp, " %c %lx,%d", &access_type, &address, &size) > 0) {
      // todo
    }

    fclose(fp);
}

// 


int main(int argc, char *argv[])
{
    parse_argument(argc, argv);

    cache_line* cache;

    initialize_cache(cache);

    read_trace();

    // test
    printf("%d, %d, %d\n", num_of_set, num_of_line, num_of_block);
       
    printSummary(0, 0, 0);
    return 0;
}

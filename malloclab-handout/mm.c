/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "J",
    /* First member's full name */
    "Junoth",
    /* First member's email address */
    "test@test.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*
 * This solution can still be improved
 * 1. For small blocks, we can remove pred/next pointer to reduce memory overhead
 * 2. For realloc method, we should have fine-grained strategy instead of simply calling malloc and free
 */

/* Basic constants and macros */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

/* Max ad Min function */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Convert address to value */
#define ADDR_VAL(addr)   ((unsigned long)addr)

/* Read and write a word at address p */
#define GET(p)        (*(unsigned long*)(p))
#define PUT(p, val)   (*(unsigned long*)(p) = (val))

/* Write/Read the size and allocated fields from address p */
#define GET_SIZE(p)         (GET(p) & ~0x7)
#define GET_ALLOC(p)        (GET(p) & 0x1)
#define GET_PREV_ALLOC(p)   (GET(p) & 0x2)
#define SET_PREV_ALLOC(p)   (PUT(p, GET(p) | 0x2))
#define UNSET_PREV_ALLOC(p) (PUT(p, GET(p) & ~0x2))

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)      ((char *)(bp) - WSIZE) 
#define FTRP(bp)      ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block header ptr, compute address of its ptr bp */
#define BP(header)    ((char *)(header) + WSIZE)

/* Given block header ptr, compute address of next and previous blocks */
#define NEXT_HD(header)     ((char *)(header) + GET_SIZE((char *)(header)))
#define PREV_HD(header)     ((char *)(header) - GET_SIZE(((char *)(header) - WSIZE))) 

/* Set/Get pointer to prev or next free block */
#define GET_PREV(header)            ((void *)(GET((char *)header + WSIZE)))
#define GET_NEXT(header)            ((void *)(GET((char *)header + DSIZE)))
#define SET_PREV(header, p)         (PUT((char *)header + WSIZE, (unsigned long)p))
#define SET_NEXT(header, p)         (PUT((char *)header + DSIZE, (unsigned long)p))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* node number in the header list */
#define HEADER_LENGTH 11

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) 

char *heap_head;

struct header_node {
  void *header;
  void *prev;
  void *next;
  void *dummy;
};

/* get the index of the header list with given size */
size_t get_index(size_t size)
{
  size_t i, n;

  i = 8; n = 1;
  while (size > i && n < HEADER_LENGTH) {
    n++;
    i <<= 1;
  }

  return n - 1;
}

/*
 * get the header node pointer with the size 
 */
struct header_node* get_header_node(size_t size) 
{
  return (struct header_node*)(heap_head + get_index(size) * sizeof(struct header_node)); 
}

/*
 * insert the free block into one list
 */
void list_insert(void *header)
{
  size_t size;
  struct header_node* node;

  /* Get the header node ptr */
  size = GET_SIZE(header);
  node = get_header_node(size);

  /* Insert the block into the node list */
  void *next = node->next;
  if (next != NULL) {
    SET_PREV(next, header);
  }
  SET_NEXT(header, next);
  SET_PREV(header, node);
  SET_NEXT(node, header);
}

/*
 * remove the free block from current list
 */
void list_remove(void *header)
{
  SET_NEXT(GET_PREV(header), GET_NEXT(header));
  if (GET_NEXT(header) != NULL) {
    SET_PREV(GET_NEXT(header), GET_PREV(header));
  }
}

/*
 * coalesce - coalesce free blocks
 */
static void *coalesce(void *ptr)
{
  void *header;
  int prev_alloc, next_alloc;
  size_t size;

  header = HDRP(ptr);
  prev_alloc = GET_PREV_ALLOC(header);
  next_alloc = GET_ALLOC(NEXT_HD(header));
  size = GET_SIZE(header);

  /* Case 1 */
  if (prev_alloc && next_alloc) {
    PUT(HDRP(ptr), PACK(size, 2));
    PUT(FTRP(ptr), PACK(size, 0));
    UNSET_PREV_ALLOC(NEXT_HD(HDRP(ptr)));
    list_insert(header);
  }
   
  /* Case 2 */
  else if (prev_alloc && !next_alloc) {
    void *next = NEXT_HD(header);
    size += GET_SIZE(next);
    list_remove(next);
    PUT(HDRP(ptr), PACK(size, 2));
    PUT(FTRP(ptr), PACK(size, 0));
    list_insert(header);
  }

  /* Case 3 */
  else if (!prev_alloc && next_alloc) {
    void *prev = PREV_HD(header);
    size += GET_SIZE(prev);
    list_remove(prev);
    PUT(prev, PACK(size, GET_PREV_ALLOC(prev)));
    PUT(FTRP(ptr), PACK(size, 0));     
    list_insert(prev);
    ptr = BP(prev);
    UNSET_PREV_ALLOC(NEXT_HD(HDRP(ptr)));
  }

  /* Case 4 */
  else {
    void *prev = PREV_HD(header), *next = NEXT_HD(header);
    size += GET_SIZE(prev) + GET_SIZE(next);
    list_remove(prev);
    list_remove(next);
    PUT(prev, PACK(size, GET_PREV_ALLOC(prev)));
    PUT(FTRP(BP(next)), PACK(size, 0));
    list_insert(prev); 
    ptr = BP(prev);
  }

  return ptr;
}

/*
 * extend_heap - extend the heap with CHUNKSIZE bytes
 */
static void *extend_heap(size_t words)
{
  char *ptr;
  size_t size;

  /* Allocate an even number of words to maintain alignment */
  size = (words & 1) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long)(ptr = mem_sbrk(size)) == -1) {
    return NULL;
  }

  char* old_ep = ptr - WSIZE;
  int prev_alloc = GET_PREV_ALLOC(old_ep);

  /* Initialize free block header/footer and the epilogue header */
  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  PUT(NEXT_HD(HDRP(ptr)), PACK(0, 1));
  if (prev_alloc) {
    SET_PREV_ALLOC(HDRP(ptr));
  }

  /* Coalesce if the previous block was free */
  return coalesce(ptr);
}

/*
 * find_fit - find a fit free block
 */
static void *find_fit(size_t asize) 
{
  struct header_node *node;
  size_t index;

  index = get_index(asize);
  node = get_header_node(asize);
  while (index < HEADER_LENGTH) {
    void *head = node->next;
    while (head != NULL) {
      if (GET_SIZE(head) >= asize) {
        return BP(head);
      }
      head = GET_NEXT(head);
    }
    node = (struct header_node*)((char *)node + sizeof(struct header_node));
    index++;
  }

  return NULL;  /* No fit */
}

/*
 * place - place the ptr in a free block 
 */
static void place(void *ptr, size_t asize)
{
  size_t csize = GET_SIZE((HDRP(ptr)));
    
  list_remove(HDRP(ptr));

  if ((csize - asize) >= (2 * DSIZE)) {
    PUT(HDRP(ptr), PACK(asize, 3));
    ptr = BP(NEXT_HD(HDRP(ptr)));
    PUT(HDRP(ptr), PACK(csize - asize, 2));
    PUT(FTRP(ptr), PACK(csize - asize, 0));
    list_insert(HDRP(ptr));
  } else {
    PUT(HDRP(ptr), PACK(csize, 3));
    SET_PREV_ALLOC(NEXT_HD(HDRP(ptr)));
  }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  /* Create the initial empty heap */
  size_t header_size = HEADER_LENGTH * sizeof(struct header_node);
  if ((heap_head = mem_sbrk(header_size + 2 * DSIZE)) == (void *) - 1) {
    return -1;
  }
  PUT(heap_head, 0);
  PUT(heap_head + (1 * WSIZE), PACK(header_size + DSIZE, 1));
  for (size_t i = 0; i < HEADER_LENGTH; ++i) {
    struct header_node *node = (struct header_node*)(heap_head + 2 * WSIZE + i * sizeof(struct header_node));
    node->header = NULL;
    node->next = NULL;
    node->prev = NULL;
  }

  heap_head += DSIZE;
  PUT(heap_head + header_size, PACK(header_size + DSIZE, 1));
  PUT(heap_head + WSIZE + header_size, PACK(0, 3));

  /* Extend the empty heap with a free block of CHUNKSIZE bytes */
  if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
    return -1;
  }

  return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  size_t asize;                            /* Adjusted block size */
  size_t extendsize;                       /* Amount to extend heap if no fit */
  char *ptr;

  /* Ignore spurious requestes */
  if (size == 0) {
    return NULL;
  }

  /* Adjust block size to include overhead and alignment reqs. */
  if (size <= DSIZE) {
    asize = 2 * DSIZE; 
  } else {
    asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
  }

  /* Search the free list for a fit */
  if ((ptr = find_fit(asize)) != NULL) {
    place(ptr, asize);
    return ptr;
  } 

  /* No fit found. Get more memory and place the block */
  extendsize = MAX(asize, CHUNKSIZE);
  if ((ptr = extend_heap(extendsize / WSIZE)) == NULL) {
    return NULL;
  }
  place(ptr, asize);
  return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  size_t oldsize;
  void *newptr;
             
  if(size == 0) {
    mm_free(ptr);
    return 0;                     
  }
                                    
  if(ptr == NULL) {
    return mm_malloc(size);
  }
                                                     
  newptr = mm_malloc(size);
                                                             
  if(!newptr) {
    return 0;                 
  }
                                                                               
  oldsize = GET_SIZE(HDRP(ptr));
  if (size < oldsize) oldsize = size;
  memcpy(newptr, ptr, oldsize);

  mm_free(ptr);
  return newptr;                                                     
}

void mm_check() {
  // block level check

  // list level check

  // heap level check
}


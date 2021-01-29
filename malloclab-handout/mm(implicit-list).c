/*
simple implicit list with next-fit  Score: 84/100
simple implicit list with optional footer(footer can be used as payload)
and next-fit Score: 83/100(Don't understand why this one get worse performance
in the memory utilization even with theoretical less internal fragmentation)

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
    "ateam",
    /* First member's full name */
    "Jianghua Wang",
    /* First member's email address */
    "jianghuaw0714@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

int mm_check(int lineno);

static void *extend_heap(size_t size,int allocated);

static void *coalesce(void *bp);

static void place(char* bp,size_t size);

static void *find_fit(size_t size,int *last_allocated);

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 /*Word and header/footer size (bytes) */

#define DSIZE 8 /* Double word size (bytes) */

#define TSIZE 12 /* Triple word size (bytes) */

#define QSIZE 16 /* Quadruple word size (bytes) */

#define PREV_ALLOC 2 /* the bit representing that the previous block is allocated*/

#define CHUNKSIZE (1<<12) /*Extend heap by this amount(bytes) */

#define MAX(x,y) ((x)>(y)?(x):(y))

#define MIN(x,y) ((x)<(y)?(x):(y))

/* Pack a size and allocated bit in to a word */
#define PACK(size,alloc,prev) (size | alloc | prev)

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)

#define PUT_PREV_ALLOC(p) ((*(unsigned int *)(p))|=0x2)
#define REMOVE_PREV_ALLOC(p)((*(unsigned int *)(p))&=(~0x2))
/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/*Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

/*check the heap*/
// #define heapchecker(lineno) (mm_check(lineno))
#define heapchecker(lineno)

static char *heap_listp;
static char *curr_pos;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1,PREV_ALLOC));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1,PREV_ALLOC));
    PUT(heap_listp+(3*WSIZE),PACK(0,1,0));
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNSIZE bytes*/
    if((heap_listp=extend_heap(CHUNKSIZE,PREV_ALLOC)) == NULL)
        return -1;
    curr_pos=heap_listp;
    return 0;
}
/* extend the size of heap */
static void *extend_heap(size_t size,int allocated){
    char *bp;
    /* Allocated an even number of words to maintain alignment */
    size=ALIGN(size);
    if((long)(bp=mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp),PACK(size,0,allocated)); //free block header
    PUT(FTRP(bp),PACK(size,0,allocated)); //free blocker footer
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1,0)); //new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


static void place(char* bp,size_t size){
    size_t curr_size=GET_SIZE(HDRP(bp));
    if((curr_size-size)<QSIZE){
        PUT(HDRP(bp),PACK(curr_size,1,PREV_ALLOC));
        PUT_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    }
    else{
        PUT(FTRP(bp),PACK((curr_size-size),0,PREV_ALLOC));
        PUT(HDRP(bp),PACK(size,1,PREV_ALLOC));
        PUT(HDRP(NEXT_BLKP(bp)),PACK((curr_size-size),0,PREV_ALLOC));
    }
    heapchecker(__LINE__);
}
static void *find_fit(size_t size,int *last_allocated){
    size_t curr_size;
    void *prev=curr_pos;
    while((curr_size=GET_SIZE(HDRP(curr_pos)))!=0){
        if(!GET_ALLOC(HDRP(curr_pos))&&(curr_size>=size))return curr_pos;
        if(!GET_ALLOC(HDRP(curr_pos))&&!GET_SIZE(HDRP(NEXT_BLKP(curr_pos))))
            *last_allocated=0;
        curr_pos=NEXT_BLKP(curr_pos);
    }
    curr_pos=heap_listp;
    while(curr_pos!=prev){
        curr_size=GET_SIZE(HDRP(curr_pos));
        if(!GET_ALLOC(HDRP(curr_pos))&&(curr_size>=size))return curr_pos;
        curr_pos=NEXT_BLKP(curr_pos);
    }
    return NULL;
}
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    /* Ignore spurious requests*/
    if(size == 0) return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if(size<=TSIZE) asize=QSIZE;
    else asize=8+ALIGN(size-4);
    
    int last_allocated=PREV_ALLOC;
    /* Search the free list for a fit*/
    if((bp=find_fit(asize,&last_allocated)) != NULL){
        place(bp,asize);
        return bp;
    }

    /* No fitr found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if((bp = extend_heap(extendsize,last_allocated)) == NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

static void *coalesce(void *bp){
    size_t prev_alloc=GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    /* Case1: both previous block and next block are alocated. */
    if(prev_alloc && next_alloc)
        return bp;
    /* Case2: the next block is free. */
    else if(prev_alloc && !next_alloc){
        if(NEXT_BLKP(bp)==curr_pos)curr_pos=bp;
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0,PREV_ALLOC));
        PUT(FTRP(bp),PACK(size,0,PREV_ALLOC));
    }
    /* Case3: the previous block is free. */
    else if(!prev_alloc && next_alloc){
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0,PREV_ALLOC));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0,PREV_ALLOC));
        bp=PREV_BLKP(bp);
    }
    /* Case4: both the previous and next block are free.*/
    else{
        if(NEXT_BLKP(bp)==curr_pos)curr_pos=PREV_BLKP(bp);
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0,PREV_ALLOC));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0,PREV_ALLOC));
        bp=PREV_BLKP(bp);
    }
    return bp;
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size=GET_SIZE(HDRP(ptr));
    unsigned int prev_allocated=GET_PREV_ALLOC(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0,prev_allocated));
    PUT(FTRP(ptr),PACK(size,0,prev_allocated));
    void *bp=coalesce(ptr);
    if(curr_pos==ptr)curr_pos=bp;
    REMOVE_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    heapchecker(__LINE__);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if(!size){
        mm_free(ptr);
        return NULL;
    }
    if(!ptr)return mm_malloc(size);
    size_t asize=8+ALIGN((size-4));
    size_t freeSize=GET_SIZE(HDRP(ptr));
    if(freeSize>=asize)return ptr;
    void *bp=mm_malloc(size);
    if(!bp)return NULL;
    memcpy(bp,ptr,MIN(size,GET_SIZE(HDRP(ptr))));
    mm_free(ptr);
    return bp;
}

int mm_check(int lineno){
    char *heap_start=heap_listp;
    int prev=1;
    int curr;
    int error=0;
    while(GET_SIZE(HDRP(heap_start))!=0){
        curr=GET_ALLOC(HDRP(heap_start));
        //check if the adjacent free block coalesce
        if(curr==0 && prev==0){
            printf("line: %d, Adjacent free blocks do not coalescing.\n",lineno);
            error=1;
        }
        //check if the previous block state is set appropriately in the current block header
        if(prev!=((GET_PREV_ALLOC(HDRP(heap_start)))>>1)){
            printf("line: %d,the previous state is conflict.\n",lineno);
            error=1;
        }
        prev=curr;
        heap_start=NEXT_BLKP(heap_start);
    }
    return !error;   
}
/*
 * 1) segregated list with total 10 list(smallest size 16 and largest size infinite),
 * first-fit,and LIFO insertion policy. Score:83
 * 2) address-order insertion policy and other small improve on the insertion. Score: 86
 * 3) change chunk size from 2^12 to 2^6 gives score 88.(huge improvement on trace4)
 * 4) In realloc, if the next block is free and large enough, do not do segmentation on the block. Score: 92
 * 5) In realloc, if the next block is the epilogue then we will extend the heap to avoid memory copy. Score:93
 * (huge improvement in trace 9 and 10, especially 10)
 * 6) insert block into the free-list based on block size(in ascending order, and head points to the smallest block).
 * given this order, the first-fit will always be best-fit since the first block we meet is the smallest one that
 * is larger than the required size. However, this one do not increase the memory utilization and I get the same 93 as before.
 * 
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

size_t get_index(size_t size);

static void *extend_heap(size_t size);

static void *coalesce(void *bp);

static void place(char* bp,size_t size);

static void *find_fit(size_t size);

static void insert_block(void *bp);

static void delete_block(void *bp);

inline size_t get_index(size_t size);

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 /*Word and header/footer size (bytes) */

#define DSIZE 8 /* Double word size (bytes) */

#define QSIZE 16 /* Quadruple word size (bytes) */

// #define PREV_ALLOC 2 /* the bit representing that the previous block is allocated*/

#define CHUNKSIZE (1<<6) /*Extend heap by this amount(bytes) */

#define MAX(x,y) ((x)>(y)?(x):(y))

#define MIN(x,y) ((x)<(y)?(x):(y))

/* Pack a size and allocated bit in to a word */
#define PACK(size,alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/*Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* !!!important: set the predecessor or successor given the address and the value.
 *    (miss this one previously)
 */
#define SETP(p,bp) (*(unsigned int *)(p)=(unsigned int)(bp))

/*Get predecessor and successor pointer(point to address storing pred/succ)*/
#define PREDP(bp) ((char*)(bp))
#define SUCCP(bp) ((char*)(bp)+WSIZE)

/*Get predecesssor and successor free block in the explicit Linkedlist */
#define PRED(bp) (*(char**)(bp))
#define SUCC(bp) (*(char**)(SUCCP(bp)))

#define HEADP(idx) ((char **)arr+idx)
#define HEAD(idx) (*(char **)(HEADP(idx)))

/*check the heap*/
// #define heapchecker(lineno) (mm_check(lineno))
#define heapchecker(lineno)

static char **arr;
static char *heap_listp;
static size_t arr_size=12;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *bp;
    //create space for the array and prologue header/footer and epilogue header
    if((bp=mem_sbrk((arr_size+4)*WSIZE))==NULL)
        return -1;
    arr=(char**)bp;
    //Intialize the array
    for(size_t i=0;i<arr_size;i++){
        SETP(bp,NULL);
        bp+=WSIZE;
    }
    PUT(bp,0); 
    PUT(bp+WSIZE,PACK(8,1)); // prologue header
    PUT(bp+(WSIZE*2),PACK(8,1)); // prologue footer
    PUT(bp+(WSIZE*3),PACK(0,1)); // epilogue header
    if((heap_listp=extend_heap(CHUNKSIZE))==NULL)
        return -1;
    return 0;
}

static void *extend_heap(size_t size)
{
    char * bp;
    size_t asize = ALIGN(size);
    if((long)(bp=mem_sbrk(asize))==-1)
        return NULL;
    // set the free block header(replace the original epilogue header)
    PUT(HDRP(bp),PACK(asize,0));
    PUT(FTRP(bp),PACK(asize,0)); // set the free block footer
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); // set the epilogue header
    return coalesce(bp);
}

static void place(char *bp,size_t size)
{
    size_t curr_size=GET_SIZE(HDRP(bp));
    size_t left_size=curr_size-size;
    //delete block from the free list
    delete_block(bp);
    if(left_size<QSIZE){
        //the left size is not enough to be a block,so gives it to the user directly
        PUT(HDRP(bp),PACK(curr_size,1));
        PUT(FTRP(bp),PACK(curr_size,1));
    }
    else{
        //split the block into two part, and insert the later part into the free list
        PUT(FTRP(bp),PACK(left_size,0));
        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(left_size,0));
        insert_block(NEXT_BLKP(bp));
    }
}

static void *find_fit(size_t size)
{
    // get the index of the first list that we want to search
    size_t idx = get_index(size);
    size_t curr_size;
    char *bp;
    while(idx<arr_size){
        bp=HEAD(idx);
        while(bp){
            curr_size=GET_SIZE(HDRP(bp));
            if(curr_size>=size)return bp;
            bp=SUCC(bp);
        }
        // if not find large enough block in the current free list,
        // search the next list containing larger blocks.
        idx++;
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
    
    //ignore spurious request
    if(size==0)return NULL;
    //the minimum size of the block is 16 (1 header,1 footer,1 pred,1 succ)
    if(size<=DSIZE)asize=QSIZE;
    else asize=8+ALIGN(size);
    
    if((bp=find_fit(asize))!=NULL){
        place(bp,asize);
        return bp;
    }
    //if not find usable free block, then ask for larger block
    extendsize=MAX(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize))==NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

static void *coalesce(void *bp)
{
    size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    if(prev_alloc && next_alloc){
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(prev_alloc && !next_alloc){
        //the index of the list that next block is currently in
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        //the index of the list that the newly combined block will be inserted
        //if the index are the same, then the old one do not need to be delete from the list,
        //we can easily combined them since it will still be in address-order.
        delete_block(NEXT_BLKP(bp));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(!prev_alloc && next_alloc){
        //same logic as before
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_block(PREV_BLKP(bp));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    else{
        //same as before.
        size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(HDRP(PREV_BLKP(bp))));
        delete_block(NEXT_BLKP(bp));
        delete_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    insert_block(bp);
    return bp;
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
    if(!size){
        mm_free(ptr);
        return NULL;
    }
    if(!ptr)return mm_malloc(size);
    char *bp;
    size_t asize=8+ALIGN(size);
    size_t free_size=GET_SIZE(HDRP(ptr));
    //if the actually size of the block is larger than the 
    //request size, then we can return directly
    if(free_size>=asize)return ptr;
    //if the next block is free and current size + next block size is larger
    //then requried size, or the next block is epilogue(we will extend the heap)
    //we do not need to copy memeory content and can return the origin address.
    if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))||!GET_SIZE(HDRP(NEXT_BLKP(ptr)))){
        free_size+=GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        //if the size is still not enough and the next block is the epilogue
        if(free_size<asize&&(!GET_SIZE(HDRP(NEXT_BLKP(ptr))))){
            //extend the heap
            if((bp=extend_heap(MAX(asize-free_size,CHUNKSIZE)))==NULL)
                return NULL;
            free_size+=MAX(asize-free_size,CHUNKSIZE);
        }
        //if the current size and next block size is enough
        if((GET_SIZE(HDRP(ptr))+GET_SIZE(HDRP(NEXT_BLKP(ptr))))>=asize){
            delete_block(NEXT_BLKP(ptr));
            PUT(HDRP(ptr),PACK(free_size,1));
            PUT(FTRP(ptr),PACK(free_size,1));
            return ptr;
        }
    }
    //get another memory by calling malloc
    bp=mm_malloc(size);
    memcpy(bp,ptr,MIN(size,GET_SIZE(HDRP(ptr))));
    mm_free(ptr);
    return bp;
}

size_t get_index(size_t size){
    //log2 is not provided so I use the stupid method.
    if(size<=16)
        return 0;
    else if(size<=32)
        return 1;
    else if(size<=64)
        return 2;
    else if(size<=128)
        return 3;
    else if(size<=256)
        return 4;
    else if(size<=512)
        return 5;
    else if(size<=1024)
        return 6;
    else if(size<=2048)
        return 7;
    else if(size<=4096)
        return 8;
    else if(size<=8192)
        return 9;
    else if(size<=16384)
        return 10;
    else
        return 11;
}

//insert in ascending order(block size)
static void insert_block(void *bp){
    size_t size=GET_SIZE(HDRP(bp));
    size_t idx=get_index(size);
    char *head=HEAD(idx);
    //if the head is NULL or the new block has smallest size
    if((!head)||(size<=GET_SIZE(HDRP(head)))){
        SETP(SUCCP(bp),head);
        SETP(PREDP(bp),NULL);
        if(head)SETP(PREDP(head),bp);
        SETP(HEADP(idx),bp);
        return;
    }
    while(head){
        //if the head has smaller size than the new block
        if(GET_SIZE(HDRP(head))<=size){
            //if we reach the end of the list or the next block
            //in the list has larger size than the new block,
            //then we can place the block.
            if((!SUCC(head))||(GET_SIZE(HDRP(SUCC(head)))>size)){
                SETP(PREDP(bp),head);
                SETP(SUCCP(bp),SUCC(head));
                if(SUCC(head))SETP(PREDP(SUCC(head)),bp);
                SETP(SUCCP(head),bp);
                return;
            }
        }
        head=SUCC(head);
    }
}

static void delete_block(void *bp){
    size_t size=GET_SIZE(HDRP(bp));
    size_t idx=get_index(size);
    char *head=HEAD(idx);
    if(head==bp)
        SETP(HEADP(idx),SUCC(bp));
    else
        SETP(SUCCP(PRED(bp)),SUCC(bp));
    if(SUCC(bp))
        SETP(PREDP(SUCC(bp)),PRED(bp));
    SETP(PREDP(bp),NULL);
    SETP(SUCCP(bp),NULL);
}
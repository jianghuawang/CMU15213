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

void mm_heapchecker(int lineno);

static void *extend_heap(size_t size,int allocated);

static void *coalesce(void *bp);

static void place(char* bp,size_t size);

static void find_fit(size_t size,int* last_allocated_ptr);

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 /*Word and header/footer size (bytes) */

#define DSIZE 8 /* Double word size (bytes) */

#define PREV_ALLOC 2

#define CHUNKSIZE (1<<12) /*Extend heap by this amount(bytes) */

#define MAX(x,y) ((x)>(y)?(x):(y))

/* Pack a size and allocated bit in to a word */
#define PACK(size,alloc,prev) (size | alloc | prev)

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p)& ~0x7)
#define GET_ALLOC(p) (GET(p)& 0x1)
#define GET_PREV_ALLOC(p) (GET(p)& 0x2)

#define PUT_PREV_ALLOC(p) ((*(unsigned int *)(p))|=0x2)
/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/*Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

/*Get precessor and successor free block in the explicit Linkedlist */
#define PRECESSOR(bp) (*((char**)bp))
#define SUCCESSOR(bp) (*(((char**)(bp))+1))
/*check the heap*/
#define heapchecker(lineno) (mm_heapchecker(lineno))
//#define heapchecker(lineno)

static void *head=NULL;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *ptr;
    if((ptr=mem_sbrk(4*WSIZE))==-1)
        return -1;
    PUT(ptr,0);
    PUT(ptr+WSIZE*1,PACK(DSIZE,1,0));
    PUT(ptr+WSIZE*2,PACK(DSIZE,1,0));
    PUT(ptr+WSIZE*3,PACK(0,1,0));
    if((head=extend_heap(CHUNKSIZE,PREV_ALLOC))==NULL)
        return -1;
    PRECESSOR(head)=NULL;
    SUCCESSOR(head)=NULL;
    return 0;
}
/* extend the size of heap */
static void *extend_heap(size_t size,int allocated){
    char *bp,newBp;
    size_t size;
    size=ALIGN(size);
    if((long)(bp=mem_sbrk(size))==-1)
        return NULL;
    PUT(HDRP(bp),PACK(size,0,allocated));
    PUT(FTRP(bp),PACK(size,0,allocated));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1,PREV_ALLOC));
    return coalesce(bp);
}

static void place(char* bp,size_t size){

}
static void *find_fit(size_t size,int* last_allocated_ptr){
    char* temp=(char*)head;
    size_t curr_size;
    while(temp!=NULL){
        curr_size=GET_SIZE(HDRP(temp));
        if(curr_size>=size)return temp;
        temp=SUCCESSOR(temp);
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

    if(size==0)return NULL;

    if(size<=DSIZE)asize=2*DSIZE;
    else asize=DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    int last_allocated=PREV_ALLOC;
    /*Search the free list for a fit*/
    if((bp=find_fit(asize,&last_allocated))!=NULL){
        place(bp,asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize=MAX(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize,last_allocated))==NULL)
        return NULL;
    place(bp,asize);
    return bp;

}

static void *coalesce(void *bp){
    size_t prev_alloc=GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    
    if(prev_alloc && next_alloc)return bp;
    else if(prev_alloc && !next_alloc){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        SUCCESSOR(PRECESSOR(PREV_BLKP(bp)))=SUCCESSOR(PREV_BLKP(bp));
        PRECESSOR(SUCCESSOR(PREV_BLKP(bp)))=PRECESSOR(PREV_BLKP(bp));
        PUT(HDRP(bp),PACK(size,0,PREV_ALLOC));
        PUT(FTRP(bp),PACK(size,0,PREV_ALLOC));
    }
    else if(!prev_alloc && next_alloc){
        size+=GET_SIZE(FTRP(PREV_BLKP(bp)));
        SUCCESSOR(PRECESSOR(NEXT_BLKP(bp)))=SUCCESSOR(NEXT_BLKP(bp));
        PRECESSOR(SUCCESSOR(NEXT_BLKP(bp)))=PRECESSOR(NEXT_BLKP(bp));
        PUT(FTRP(bp),PACK(size,0,PREV_ALLOC));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0,PREV_ALLOC));
        bp=PREV_BLKP(bp);
    }
    else{
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(FTRP(PREV_BLKP(bp)));
        SUCCESSOR(PRECESSOR(PREV_BLKP(bp)))=SUCCESSOR(PREV_BLKP(bp));
        PRECESSOR(SUCCESSOR(PREV_BLKP(bp)))=PRECESSOR(PREV_BLKP(bp));
        SUCCESSOR(PRECESSOR(NEXT_BLKP(bp)))=SUCCESSOR(NEXT_BLKP(bp));
        PRECESSOR(SUCCESSOR(NEXT_BLKP(bp)))=PRECESSOR(NEXT_BLKP(bp));
        PUT(HDRP((PREV_BLKP(bp))),PACK(size,0,PREV_ALLOC));
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
    //get the original size of the block
    size_t size=GET_SIZE(HDRP(bp));
    void *newBp=coalesce(ptr);
    //check if coalesce happens
    if(GET_SIZE(newBp)==size){
        //if not happen then we need to set the header and footer info
        PUT(HDRP(newBp),PACK(size,0,PREV_ALLOC));
        PUT(FTRP(newBp),PACK(size,0,PREV_ALLOC));
    }
    //set prev_alloc field in the next block 
    PUT_PREV_ALLOC(HDRP(NEXT_BLKP(newBp)));
    //insert the block in the front of the free list
    PRECESSOR(newBp)=NULL;
    SUCCESSOR(newBp)=head;
    head=bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
}















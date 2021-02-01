/*
 * 1) segregated list with total 10 list(smallest size 16 and largest size infinite),
 * first-fit,and LIFO insertion policy. Score:83
 * 2) address-order insertion policy and other small improve on the insertion. Score: 86
 * 3)
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

// #define TSIZE 12 /* Triple word size (bytes) */

#define QSIZE 16 /* Quadruple word size (bytes) */

// #define PREV_ALLOC 2 /* the bit representing that the previous block is allocated*/

#define CHUNKSIZE (1<<12) /*Extend heap by this amount(bytes) */

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
// #define GET_PREV_ALLOC(p) (GET(p)& 0x2)

// #define PUT_PREV_ALLOC(p) ((*(unsigned int *)(p))|=0x2)
// #define REMOVE_PREV_ALLOC(p)((*(unsigned int *)(p))&=(~0x2))
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
static size_t arr_size=10;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *bp;
    if((bp=mem_sbrk((arr_size+4)*WSIZE))==NULL)
        return -1;
    arr=(char**)bp;
    for(size_t i=0;i<arr_size;i++){
        SETP(bp,NULL);
        bp+=WSIZE;
    }
    PUT(bp,0);
    PUT(bp+WSIZE,PACK(8,1));
    PUT(bp+(WSIZE*2),PACK(8,1));
    PUT(bp+(WSIZE*3),PACK(0,1));
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
    PUT(HDRP(bp),PACK(asize,0));
    PUT(FTRP(bp),PACK(asize,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    return coalesce(bp);
}

static void place(char *bp,size_t size)
{
    size_t curr_size=GET_SIZE(HDRP(bp));
    size_t left_size=curr_size-size;
    delete_block(bp);
    if(left_size<QSIZE){
        PUT(HDRP(bp),PACK(curr_size,1));
        PUT(FTRP(bp),PACK(curr_size,1));
    }
    else{
        PUT(FTRP(bp),PACK(left_size,0));
        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(left_size,0));
        insert_block(NEXT_BLKP(bp));
    }
}

static void *find_fit(size_t size)
{
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
    
    if(size==0)return NULL;
    
    if(size<=DSIZE)asize=QSIZE;
    else asize=8+ALIGN(size);
    
    if((bp=find_fit(asize))!=NULL){
        place(bp,asize);
        return bp;
    }

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
    size_t req_insrt=1;
    if(prev_alloc && next_alloc){
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(prev_alloc && !next_alloc){
        size_t orig_idx=get_index(GET_SIZE(HDRP(NEXT_BLKP(bp))));
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        size_t new_idx=get_index(size);
        if(new_idx==orig_idx){
            req_insrt=0;
            SETP(PREDP(bp),PRED(NEXT_BLKP(bp)));
            SETP(SUCCP(bp),SUCC(NEXT_BLKP(bp)));
            if(HEAD(new_idx)==NEXT_BLKP(bp))
                SETP(HEADP(new_idx),bp)
            else
                SETP(SUCCP(PRED(NEXT_BLKP(bp))),bp);
            if(SUCC(NEXT_BLKP(bp)))
                SETP(PREDP(SUCC(NEXT_BLKP(bp))),bp);
        }else 
            delete_block(NEXT_BLKP(bp));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(!prev_alloc && next_alloc){
        size_t orig_idx=get_index(GET_SIZE(HDRP(PREV_BLKP(bp))));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        size_t new_idx=get_index(size);
        if(orig_idx==new_idx)
            req_insrt=0;
        else
            delete_block(PREV_BLKP(bp));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    else{
        size_t orig_idx=get_index(GET_SIZE(HDRP(PREV_BLKP(bp))));
        size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(HDRP(PREV_BLKP(bp))));
        size_t new_idx=get_index(size);
        delete_block(NEXT_BLKP(bp));
        if(new_idx==orig_idx)
            req_insrt=0;
        else
            delete_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    if(req_insrt)insert_block(bp);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{  
    coalesce(ptr);
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
    size_t asize=8+ALIGN(size);
    size_t free_size=GET_SIZE(HDRP(ptr));
    if(free_size>=asize)return ptr;
    free_size+=(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))))?GET_SIZE(HDRP(NEXT_BLKP(ptr))):0;
    if(free_size>=asize){
        size_t left_size=free_size-asize;
        delete_block(NEXT_BLKP(ptr));
        if(left_size<QSIZE){
            PUT(HDRP(ptr),PACK(free_size,1));
            PUT(FTRP(ptr),PACK(free_size,1));
        }
        else{
            PUT(HDRP(ptr),PACK(asize,1));
            PUT(FTRP(ptr),PACK(asize,1));
            PUT(HDRP(NEXT_BLKP(ptr)),PACK(left_size,0));
            PUT(FTRP(NEXT_BLKP(ptr)),PACK(left_size,0));
            insert_block(NEXT_BLKP(ptr));
        }
        return ptr;
    }
    void *bp=mm_malloc(size);
    memcpy(bp,ptr,MIN(size,GET_SIZE(HDRP(ptr))));
    mm_free(ptr);
    return bp;
}

size_t get_index(size_t size){
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
    else
        return 9;
}

static void insert_block(void *bp){
    size_t size=GET_SIZE(HDRP(bp));
    size_t idx=get_index(size);
    char *head=HEAD(idx);
    if((!head)|(head>bp)){
        SETP(SUCCP(bp),head);
        SETP(PREDP(bp),NULL);
        if(head)SETP(PREDP(head),bp);
        SETP(HEADP(idx),bp);
        return;
    }
    while(head){
        if(head<bp){
            if((!SUCC(head))|(SUCC(head)>bp)){
                SETP(PREDP(bp),head);
                SETP(SUCCP(bp),SUCC(head));
                if(SUCC(head))SETP(PREDP(SUCC(head)),bp);
                SETP(SUCCP(head),bp);
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
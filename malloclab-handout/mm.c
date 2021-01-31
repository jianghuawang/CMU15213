/*
 * 1) Explicit list with first-fit,compulsory footer. Besides, when coalescence
 * happens, if either previous or next block is in the free list, the new one will
 * not be inserted in the front of the list. Score: 83
 * 2) Only change first-fit to next fit. Score: 82
 * 3) Always put the new free block on the front of the list(first-fit). Score: 83
 * (the previous method have better space utilization for every test but result is not shown in the score.)
 * 4) Given first-fit and the first coalsece method, improve the realloc function(if the next block is free, then
 * combine the current one with next one, and return the current ptr). have large improve on the last two test case
 * but not great improve on the score. Score: 85
 * 5) Given first-fit and the first coalesce method, use optional footer downgrade the space utilization. Score: 85
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

static void *extend_heap(size_t size,size_t allocated);

static void *coalesce(void *bp);

static void place(char* bp,size_t size);

static void *find_fit(size_t size,size_t *allocated);

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
#define PACK(size,alloc) ((size) | (alloc))
#define PACK_ALLOC(size,alloc,prev) ((size) | (alloc) | (prev)) 

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p)& 0x2)

#define PUT_PREV_ALLOC(p) ((*(unsigned int *)(p))|=0x2)
#define REMOVE_PREV_ALLOC(p)((*(unsigned int *)(p))&=(~0x2))

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

/*check the heap*/
// #define heapchecker(lineno) (mm_check(lineno))
#define heapchecker(lineno)

//the head of the free list
static char *head;
static char *heap_listp;
// static char *curr_pos;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *ptr;
    if((ptr=mem_sbrk(4*WSIZE))==(void*)-1)
        return -1;
    PUT(ptr,0);
    PUT(ptr+(WSIZE*1),PACK_ALLOC(DSIZE,1,PREV_ALLOC));
    PUT(ptr+(WSIZE*2),PACK(DSIZE,1));
    PUT(ptr+(WSIZE*3),PACK_ALLOC(0,1,PREV_ALLOC));
    if((head=extend_heap(CHUNKSIZE,PREV_ALLOC))==NULL)
        return -1;
    SETP(PREDP(head),NULL);
    SETP(SUCCP(head),NULL);
    heap_listp=head;
    // curr_pos=head;
    return 0;
}
/* extend the size of heap */
static void *extend_heap(size_t size,size_t allocated){
    char *bp;
    size_t asize=ALIGN(size);
    if((long)(bp=mem_sbrk(asize))==-1)
        return NULL;
    PUT(HDRP(bp),PACK_ALLOC(asize,0,allocated));  //the free block header
    PUT(FTRP(bp),PACK(asize,0));  //the free block footer
    PUT(HDRP(NEXT_BLKP(bp)),PACK_ALLOC(0,1,0)); //the epilogue header
    return coalesce(bp);
}

static void place(char* bp,size_t size){
    size_t curr_size=GET_SIZE(HDRP(bp));
    size_t left_size=curr_size-size;
    if(left_size<QSIZE){
        PUT(HDRP(bp),PACK_ALLOC(curr_size,1,PREV_ALLOC));
        // PUT(FTRP(bp),PACK(curr_size,1));
        PUT_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
        if(head==bp)head=SUCC(bp);
        // curr_pos=SUCC(bp);
        if(PRED(bp)!=NULL)
            SETP(SUCCP(PRED(bp)),SUCC(bp));
        if(SUCC(bp)!=NULL)
            SETP(PREDP(SUCC(bp)),PRED(bp));
    }
    else{
        PUT(FTRP(bp),PACK(left_size,0));
        PUT(HDRP(bp),PACK_ALLOC(size,1,PREV_ALLOC));
        PUT(HDRP(NEXT_BLKP(bp)),PACK_ALLOC(left_size,0,PREV_ALLOC));
        SETP(PREDP(NEXT_BLKP(bp)),PRED(bp));
        SETP(SUCCP(NEXT_BLKP(bp)),SUCC(bp));
        // curr_pos=NEXT_BLKP(bp);
        if(head==bp)head=NEXT_BLKP(bp);
        if(PRED(bp)!=NULL)
            SETP(SUCCP(PRED(bp)),NEXT_BLKP(bp));
        if(SUCC(bp)!=NULL)
            SETP(PREDP(SUCC(bp)),NEXT_BLKP(bp));
    }
    heapchecker(__LINE__);
}
static void *find_fit(size_t size,size_t *allocated_ptr){
    // char* prev=curr_pos;
    char* bp=head;
    size_t curr_size;
    while(bp){
        curr_size=GET_SIZE(HDRP(bp));
        if(curr_size>=size)return bp;
        if(!GET_SIZE(HDRP(NEXT_BLKP(bp))))*allocated_ptr=0;
        bp=SUCC(bp);
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

    if(size<=TSIZE)asize=QSIZE;
    else asize=8+ALIGN(size-4);

    size_t allocated=PREV_ALLOC;
    /*Search the free list for a fit*/
    if((bp=find_fit(asize,&allocated))!=NULL){
        place(bp,asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize=MAX(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize,allocated))==NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

static void *coalesce(void *bp){
    size_t prev_alloc=GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    if(prev_alloc && next_alloc){
        SETP(PREDP(bp),NULL);
        SETP(SUCCP(bp),head);
        if(head)
            SETP(PREDP(head),bp);
        head=bp;
    }
    else if(prev_alloc && !next_alloc){
        // if(curr_pos==NEXT_BLKP(bp))curr_pos=bp;
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        SETP(PREDP(bp),PRED(NEXT_BLKP(bp)));
        SETP(SUCCP(bp),SUCC(NEXT_BLKP(bp)));
        if(head==NEXT_BLKP(bp))head=(char*)bp;
        if(PRED(NEXT_BLKP(bp))!=NULL)
            SETP(SUCCP(PRED(NEXT_BLKP(bp))),bp);
        if(SUCC(NEXT_BLKP(bp))!=NULL)
            SETP(PREDP(SUCC(NEXT_BLKP(bp))),bp);
        PUT(HDRP(bp),PACK_ALLOC(size,0,PREV_ALLOC));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(!prev_alloc && next_alloc){
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK_ALLOC(size,0,PREV_ALLOC));
        bp=PREV_BLKP(bp);
    }
    else{
        // if(curr_pos==NEXT_BLKP(bp))curr_pos=PREV_BLKP(bp);
        size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(HDRP(PREV_BLKP(bp))));
        if(head==NEXT_BLKP(bp))head=SUCC(NEXT_BLKP(bp));
        if(PRED(NEXT_BLKP(bp))!=NULL)
            SETP(SUCCP(PRED(NEXT_BLKP(bp))),SUCC(NEXT_BLKP(bp)));
        if(SUCC(NEXT_BLKP(bp))!=NULL)
            SETP(PREDP(SUCC(NEXT_BLKP(bp))),PRED(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK_ALLOC(size,0,PREV_ALLOC));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    REMOVE_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    return bp;
}

// static void *coalesce(void *bp){
//     size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
//     size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//     size_t size=GET_SIZE(HDRP(bp));
//     if(prev_alloc && next_alloc){
//         SETP(PREDP(bp),NULL);
//         SETP(SUCCP(bp),head);
//         if(head)
//             SETP(PREDP(head),bp);
//         head=bp;
//         return bp;
//     }
//     else if(prev_alloc && !next_alloc){
//         // if(curr_pos==NEXT_BLKP(bp))curr_pos=bp;
//         size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
//         if(head==NEXT_BLKP(bp)){
//             SETP(PREDP(bp),NULL);
//             SETP(SUCCP(bp),SUCC(NEXT_BLKP(bp)));
//             if(SUCC(NEXT_BLKP(bp))!=NULL)
//                 SETP(PREDP(SUCC(NEXT_BLKP(bp))),bp);
//         }
//         else{
//             SETP(SUCCP(PRED(NEXT_BLKP(bp))),SUCC(NEXT_BLKP(bp)));
//             if(SUCC(NEXT_BLKP(bp))!=NULL)
//                 SETP(PREDP(SUCC(NEXT_BLKP(bp))),PRED(NEXT_BLKP(bp)));
//             SETP(PREDP(bp),NULL);
//             SETP(SUCCP(bp),head);
//             if(head)
//                 SETP(PREDP(head),bp);
//         }
//         head=bp;
//         PUT(HDRP(bp),PACK(size,0));
//         PUT(FTRP(bp),PACK(size,0));
//     }
//     else if(!prev_alloc && next_alloc){
//         size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
//         PUT(FTRP(bp),PACK(size,0));
//         PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
//         bp=PREV_BLKP(bp);
//         if(head!=bp){
//             SETP(SUCCP(PRED(bp)),SUCC(bp));
//             if(SUCC(bp)!=NULL)
//                 SETP(PREDP(SUCC(bp)),PRED(bp));
//             SETP(PREDP(bp),NULL);
//             SETP(SUCCP(bp),head);
//             if(head)
//                 SETP(PREDP(head),bp);
//             head=bp;
//         }
//     }
//     else{
//         // if(curr_pos==NEXT_BLKP(bp))curr_pos=PREV_BLKP(bp);
//         size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(HDRP(PREV_BLKP(bp))));
//         // if(head==NEXT_BLKP(bp))head=SUCC(NEXT_BLKP(bp));
//         if(head==NEXT_BLKP(bp)){
//             head=SUCC(head);
//             if(head && head==PREV_BLKP(bp))
//                 head=SUCC(head);
//         }
//         else if(head==PREV_BLKP(bp)){
//             head=SUCC(head);
//             if(head && head==NEXT_BLKP(bp))
//                 head=SUCC(head);
//         }
//         if(PRED(NEXT_BLKP(bp))!=NULL)
//             SETP(SUCCP(PRED(NEXT_BLKP(bp))),SUCC(NEXT_BLKP(bp)));
//         if(SUCC(NEXT_BLKP(bp))!=NULL)
//             SETP(PREDP(SUCC(NEXT_BLKP(bp))),PRED(NEXT_BLKP(bp)));
//         if(PRED(PREV_BLKP(bp))!=NULL)
//             SETP(SUCCP(PRED(PREV_BLKP(bp))),SUCC(PREV_BLKP(bp)));
//         if(SUCC(PREV_BLKP(bp))!=NULL)
//             SETP(PREDP(SUCC(PREV_BLKP(bp))),PRED(PREV_BLKP(bp)));
//         PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
//         PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
//         bp=PREV_BLKP(bp);
//         SETP(PREDP(bp),NULL);
//         SETP(SUCCP(bp),head);
//         if(head)
//             SETP(PREDP(head),bp);
//         head=bp;
//     }
//     return bp;
// }
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size=GET_SIZE(HDRP(ptr));
    //clear the allocated bit
    PUT(HDRP(ptr),PACK_ALLOC(size,0,GET_PREV_ALLOC(HDRP(ptr))));
    PUT(FTRP(ptr),PACK(size,0));
    SETP(PREDP(ptr),NULL);
    SETP(SUCCP(ptr),NULL);
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
        if(left_size<QSIZE){
            if(head==NEXT_BLKP(ptr))head=SUCC(head);
            if(PRED(NEXT_BLKP(ptr)))
                SETP(SUCCP(PRED(NEXT_BLKP(ptr))),SUCC(NEXT_BLKP(ptr)));
            if(SUCC(NEXT_BLKP(ptr)))
                SETP(PREDP(SUCC(NEXT_BLKP(ptr))),PRED(PREV_BLKP(ptr)));
            PUT(HDRP(ptr),PACK_ALLOC(free_size,1,GET_PREV_ALLOC(HDRP(ptr))));
        }
        else{
            if(head==NEXT_BLKP(ptr))head=ptr+asize;
            char *pred=PRED(NEXT_BLKP(ptr));
            char *succ=SUCC(NEXT_BLKP(ptr));
            PUT(HDRP(ptr),PACK_ALLOC(asize,1,GET_PREV_ALLOC(HDRP(ptr))));
            PUT(HDRP(NEXT_BLKP(ptr)),PACK_ALLOC(left_size,0,PREV_ALLOC));
            PUT(FTRP(NEXT_BLKP(ptr)),PACK(left_size,0));
            SETP(PREDP(NEXT_BLKP(ptr)),pred);
            SETP(SUCCP(NEXT_BLKP(ptr)),succ);
            if(PRED(NEXT_BLKP(ptr)))
                SETP(SUCCP(PRED(NEXT_BLKP(ptr))),NEXT_BLKP(ptr));
            if(SUCC(NEXT_BLKP(ptr)))
                SETP(PREDP(SUCC(NEXT_BLKP(ptr))),NEXT_BLKP(ptr));
        }
        return ptr;
    }
    void *bp=mm_malloc(size);
    memcpy(bp,ptr,MIN(size,GET_SIZE(HDRP(ptr))));
    mm_free(ptr);
    return bp;
}
/*
* mm_check
1) Is every block in the free list marked as free?
*/
int mm_check(int lineno){
    char* heap_start=head;
    int prev=1,curr;
    int heap_counter=0,list_counter=0;
    char *lo=mem_heap_lo();
    char *hi=mem_heap_hi();
    while(heap_start){
        list_counter++;
        if(GET_ALLOC(HDRP(heap_start))){
            printf("some blocks on the free list are marked as allocated\n");
        }
        if(PRED(heap_start)!=NULL&&((PRED(heap_start)<lo)|(PRED(heap_start)>hi)))
            printf("predecessor points to invalid address.\n");
        if(SUCC(heap_start)!=NULL&&((SUCC(heap_start)<lo)|(SUCC(heap_start)>hi)))
            printf("successor points to invalid address.\n");
        heap_start=SUCC(heap_start);
    }
    heap_start=heap_listp;
    while(GET_SIZE(HDRP(heap_start))!=0){
        curr=GET_ALLOC(HDRP(heap_start));
        if(!curr){
            heap_counter++;
            int found=0;
            for(char* bp=head;bp!=NULL;bp=SUCC(bp)){
                if(bp==heap_start){
                    found=1;
                    break;
                }
            }
            if(!found){
                printf("free block not in free list\n");
            }
        }
        if(curr==0 && prev==0)
            printf("adjacent free blocks are not coalesced\n");
        prev=curr;
        heap_start=NEXT_BLKP(heap_start);
    }
    // printf("heap counter: %d, list counter: %d\n",heap_counter,list_counter);
    return 1;
}












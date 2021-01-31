/*
 * 1) segregated list
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

static void *extend_heap(size_t size);

static void *coalesce(void *bp);

static void place(char* bp,size_t size);

static void *find_fit(size_t size);

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

/*check the heap*/
// #define heapchecker(lineno) (mm_check(lineno))
#define heapchecker(lineno)

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

}

static void *extend_heap()
{

}

static void place(char *bp,size_t size)
{

}

static void *find_fit(size_t size)
{

}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

}

static void *coalesce(void *bp)
{

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

}
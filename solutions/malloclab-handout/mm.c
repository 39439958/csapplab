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
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* some define about using free list*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE 1024
#define CLASSSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
//处理头部或尾部
#define PACK(size, alloc) ((size) | (alloc))
//获取p处的指；将p处的指改为val
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
//获取头部的size和alloc字段
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
//根据bp获取它的头部指针或尾部指针
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
//根据bp获取它前一个结点的bp或后一个结点的bp
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
//获取空闲块的前驱后继
#define GET_PREV(bp) ((unsigned int *)(long)GET(bp))
#define GET_NEXT(bp) ((unsigned int *)(long)GET((char *)bp + WSIZE))  

/* 堆的头：指向序言段的后面 */
static char *heap_listp;

//获取序号为i的链表头结点
#define GET_HEAD(num) ((unsigned int *)(long)(GET(heap_listp + WSIZE * num)))

/* 定义函数 */
static void* extend_heap(size_t words);
static void* coalesce(void *bp);
static void* find_fit(size_t asize);
static void* place(void* bp, size_t asize);
static int search(size_t size);
static void insert(void *bp);
static void delete(void *bp);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *bp);
void *mm_realloc(void *ptr, size_t size);

/* 
 * extend_heap - 开辟一块新空间，返回 
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;//双字对齐
    if ((long)(bp = mem_sbrk(size)) == (-1))
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));//旧的结尾块此时变成了新申请的空闲块的头部
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));//新结尾块

    return coalesce(bp);//合并前面的空闲块（如果有）
}

/*
 * coalesce - 合并bp附近的空闲块
 */
static void* coalesce(void *bp) {
    int prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //case1 - 前面和后面块都分配了
    if (prev_alloc && next_alloc) {
        insert(bp);
        return bp; 
    }
        

    //case2 - 前面块没分配,后面块分配了
    else if (!prev_alloc && next_alloc) {
        delete(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert(bp);
    }

    //case3 - 后面块没分配，前面块分配了
    else if (prev_alloc && !next_alloc) {
        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));//因为头部被设置过了，所以此时调用FTRP获取到的是后一个块的尾部，他们以及合并了
        insert(bp);
    }

    //case4 - 都没分配
    else {
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert(bp);
    }
    
    return bp;
}

/*
 * find_fit - 寻找合适的空闲块，这里使用first fit匹配策略
 */
static void* find_fit(size_t asize) {
    int num = search(asize);
    char *bp;

    while (num < CLASSSIZE) {
        bp = GET_HEAD(num);
        while (bp) {
            if (GET_SIZE(HDRP(bp)) > asize)
                return (void *)bp;
            bp = GET_NEXT(bp);
        }
        num ++;
    }
    return NULL;
}


/*
 * place - 设置bp为已分配
 */
static void* place(void* bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= 2 * DSIZE) {
        delete(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insert(bp);
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
    } else {
        delete(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    return bp;
}

/* 
 * search - 返回size大小属于哪个组 
 * 组别设置为:{16},{17~32},...,{2^18+1~INF}
 */
static int search(size_t size) {
    for (int i = 4; i <= 18; i++) {
        int tmp = 1 << i;
        if (size <= tmp) {
            return i - 4;
        }
    }
    return 15; 
}

/* 
 * insert - 将bp插入空闲链表
 */
static void insert(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int num = search(size);

    if (GET_HEAD(num) == NULL) {
        PUT(heap_listp + WSIZE * num, bp);
        PUT(bp, NULL);
        PUT((unsigned int *)bp + 1, NULL);
    } else {
        PUT((unsigned int *)bp + 1, GET_HEAD(num));
        PUT(bp, NULL);
        PUT(GET_HEAD(num), bp);
        PUT(heap_listp + WSIZE * num, bp);
    }
}

/* 
 * delete - 将bp从空闲链表中删除
 */
static void delete(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int num = search(size);

    if (GET_PREV(bp) == NULL && GET_NEXT(bp) == NULL) {
        PUT(heap_listp + WSIZE * num, NULL);
    }
    else if (GET_PREV(bp) != NULL && GET_NEXT(bp) == NULL) {
        PUT(GET_PREV(bp) + 1, NULL);
    }
    else if (GET_NEXT(bp) != NULL && GET_PREV(bp) == NULL) {
        PUT(heap_listp + WSIZE * num, GET_NEXT(bp));
        PUT(GET_NEXT(bp), NULL);
    }
    else if (GET_PREV(bp) != NULL && GET_NEXT(bp) != NULL){
        PUT(GET_PREV(bp) + 1, GET_NEXT(bp));
        PUT(GET_NEXT(bp), GET_PREV(bp));
    }

}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk((4 + CLASSSIZE)* WSIZE)) == (void *)-1)
        return -1;
    
    //设置起始空块
    PUT(heap_listp, 0);
    heap_listp += WSIZE;
    //设置空闲块头指针
    for (int i = 0; i < CLASSSIZE; i++) {
        PUT(heap_listp + i * WSIZE, NULL);
    }
    //设置序言块
    PUT(heap_listp + CLASSSIZE * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (1 + CLASSSIZE) * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (2 + CLASSSIZE) * WSIZE, PACK(0, 1));
    
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    char *bp;
    size_t asize;//对齐调整后的size
    size_t extendsize;

    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * (((size) + (DSIZE) + (DSIZE - 1)) / DSIZE);//按八字节向上取整，例如size = 13，则asize = （13 + 8 + 7）/ 8 * 8 = 24；   
    
    if ((bp = find_fit(asize)) != NULL) {
        bp = place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {
        return NULL;
    }
    bp = place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) 
        return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    size_t psize = GET_SIZE(HDRP(ptr));
    //空间不变
    if (psize == size) 
        return ptr;

    int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    
    if (size > psize && !next_alloc && next_size + psize >= size) {
        delete(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK(next_size + psize, 1));
        PUT(FTRP(ptr), PACK(next_size + psize, 1));
        return ptr;
    }
    else if (!next_size && size >= psize) {
        size_t extend_size = (size - psize);
        if((long)(mem_sbrk(extend_size)) == -1)            
            return NULL;                 
        PUT(HDRP(ptr), PACK(psize + extend_size, 1));        
        PUT(FTRP(ptr), PACK(psize + extend_size, 1));        
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
        return ptr;
    }
    else {
        char * new_ptr = mm_malloc(size); 
        if (new_ptr == NULL)
            return NULL;
        memcpy(new_ptr, ptr, MIN(size, psize));
        mm_free(ptr);
        return new_ptr;
    }
}
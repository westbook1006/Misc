/*
 * Utility functions for the hashtable
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stddef.h>

//typedef uintptr_t ptr_t;

#define KEY_LEN 9 
#define VALUE_LEN 9
#define BUCKET_SIZE 8

typedef struct _node {
    char key[KEY_LEN];
    char value[VALUE_LEN];
    struct _node *next;
} __attribute__((packed)) node;

typedef struct _bucket {
    node *head;
    node *tail;
} __attribute__((packed)) bucket;

typedef struct _table {
    bucket lf_buckets[BUCKET_SIZE];
    uint32_t size;
} __attribute__((packed)) LF_hashtable;

//#define MARK_OF(x) ((x) & 1) 
//#define PTR_MASK(x) ((x) & ~ (marked_ptr_t)1)
//#define PTR_OF(x) ((node*)PTR_MASK(x))
//#define CONSTRUCT(ptr, mark) (PTR_MASK((marked_ptr_t)ptr) | (mark))
//

#define CAS(ADDR, OLDV, NEWV)  \
    __sync_bool_compare_and_swap((ADDR), (OLDV), (NEWV))

#define INC(ADDR, VAL) __sync_fetch_and_add(ADDR, VAL)

uint32_t jenkins_hash(const void *key, size_t length);

#endif

/*
 * Utility functions for the hashtable
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

#define KEY_LEN 11
//#define VALUE_LEN 64
#define VALUE_LEN 1024
#define BUCKET_SIZE 64
#define ITEM_SIZE 4096
#define HT_SIZE 4096

typedef struct _node {
    char key[KEY_LEN];
    char value[VALUE_LEN];
    int data_item;
    struct _node *next;
} __attribute__((packed)) node;

typedef struct _bucket {
    node *head;
    node *tail;
} __attribute__((packed)) bucket;

typedef struct _item {
    void *data;
    struct _node *node;
    uint64_t freq;
} __attribute__((packed)) item;

typedef struct _table {
    bucket lf_buckets[BUCKET_SIZE];
    uint32_t size;
} __attribute__((packed)) LF_hashtable;

typedef struct _memory {
    item data_item[HT_SIZE];
    int free_list[HT_SIZE];
    volatile int alloc_stack_pt;
    pthread_mutex_t alloc_lock;

    void (*push)(int);
    int (*pop)();
} __attribute__((packed)) LF_memory;

#define CAS(ADDR, OLDV, NEWV)  \
    __sync_bool_compare_and_swap((ADDR), (OLDV), (NEWV))

#define INC(ADDR, VAL) __sync_fetch_and_add(ADDR, VAL)

uint32_t jenkins_hash(const void *key, size_t length);

#endif

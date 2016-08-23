/*
 * A lock-free hashtable implementation
 *
 * Reference: A paper from SPAA'02
 *     High Performance Dynamic Lock-Free Hash Tables and List-Based Sets
 */
#include <stdio.h>
#include <stdint.h>
#include "util.h"

#define KEY_LEN 9 
#define VALUE_LEN 17

typedef uintptr_t marked_ptr_t;

typedef struct _node {
    char key[KEY_LEN];
    char value[VALUE_LEN];
    marked_ptr_t next;
} __attribute__((packed)) node;

typedef struct _table {
    marked_ptr_t buckets[1024];
    uint32_t size;
} __attribute__((packed)) LF_hashtable;

LF_hashtable lf_table;

int 
hashtable_init()
{
    printf("Hashtable init\n");
    return 0;
}

int
hashtable_insert(char *key, char *value)
{
    uint32_t index = jenkins_hash(key, KEY_LEN);
    marked_ptr_t new_node, prev, cur, head;

    while (1) {
    }

    //printf("HT INSERT key: %s value: %s\n", key, value);
    return 0;
}

int 
hashtable_search(char *key)
{
    printf("HT SEARCH key: %s\n", key);
    return 0;
}

int 
hashtable_delete(char *key)
{
    printf("HT DELETE key: %s\n", key);
    return 0;
}

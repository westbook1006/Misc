/*
 * A lock-free hashtable implementation
 *
 * Reference: A paper from SPAA'02
 *     High Performance Dynamic Lock-Free Hash Tables and List-Based Sets
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define KEY_LEN 9 
#define VALUE_LEN 17
#define BUCKET_SIZE 8

//#define HT_DEBUG

typedef struct _node {
    char key[KEY_LEN];
    char value[VALUE_LEN];
    struct _node *next;
} __attribute__((packed)) node;

typedef struct _table {
    node* buckets[BUCKET_SIZE];
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
    node *new_node, **prev, *cur;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;

    new_node = (node *)malloc(sizeof(node));
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);

    while (1) {
        cur = lf_table.buckets[index];
        prev = &lf_table.buckets[index];

        //printf("keep adding stuffs\n");
        while ((cur != NULL) && (strcmp(cur->key, key) > 0)) {
            prev = &cur->next;
            cur = cur->next;
        }

        new_node->next = *prev;
        if (__sync_val_compare_and_swap(prev, cur, new_node) == cur) {
            break;
        }
    }

    __sync_fetch_and_add(&lf_table.size, 1);

#ifdef HT_DEBUG
    printf("HT INSERT key: %s value: %s bucket index: %d\n", key, value, index);
#endif
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

int
hashtable_dump()
{
    printf("Hashtable size is %d\n", lf_table.size);
    for (int i = 0; i < BUCKET_SIZE; i++) {
        node *head = lf_table.buckets[i];

        uint32_t cnt = 0;
        while (head != NULL) {
            cnt++;
            head = head->next;
        }
        printf("Bucket %d has %d items\n", i, cnt);

#if 0
        printf("Bucket %d BEGIN: ", i);
        while (head != NULL) {
            printf("<key: %s, value: %s> --> ", head->key, head->value);
            head = head->next;
        }
        printf("END\n");
#endif
    }

    return 0;
}

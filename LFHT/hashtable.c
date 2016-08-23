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

#define HT_DEBUG

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

#ifdef HT_DEBUG
    printf("HT INSERT key: %s value: %s bucket index: %d\n", key, value, index);
#endif

    new_node = (node *)malloc(sizeof(node));
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);

    while (1) {
        cur = lf_table.buckets[index];
        prev = &lf_table.buckets[index];

        while ((cur != NULL) && (strcmp(cur->key, key) > 0)) {
            prev = &cur->next;
            cur = cur->next;
        }

        // Update when the key has been saved.
        if ((cur != NULL) && (!strcmp(cur->key, key))) {
            strcpy(cur->value, value);
            free(new_node);
            return 0;
        }

        new_node->next = *prev;
        if (__sync_val_compare_and_swap(prev, cur, new_node) == cur) {
            break;
        }
    }

    __sync_fetch_and_add(&lf_table.size, 1);

    return 0;
}

char* 
hashtable_search(char *key)
{
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
    node *cur, **prev, **next;
    char *cur_key, *cur_value;

#ifdef HT_DEBUG
    printf("HT SEARCH key: %s bucket index: %d\n", key, index);
#endif

    while (1) {
        prev = &lf_table.buckets[index];
        cur = lf_table.buckets[index];

        while (1) {
            if (!cur)
                return NULL;

            next = &cur->next;
            cur_key = cur->key;
            cur_value = cur->value;

            if (*prev != cur)
                break;

            if (!strcmp(cur_key, key))
                return cur_value;

            prev = next;
            cur = *next;
        }
    }

    return NULL;
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

#if 1 
        head = lf_table.buckets[i];
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

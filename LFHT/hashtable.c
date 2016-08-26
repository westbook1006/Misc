/*
 * A lock-free hashtable implementation
 *
 * Reference:
 *  [1] A Pragmatic Implementation of Non-Blocking Linked-Lists,
 *      Timothy L. Harris, DISC, 2001
 *  [2] High Performance Dynamic Lock-Free Hash Tables and List-Based sets,
 *      Maged M. Michael, SPAA, 2002
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

//#define HT_DEBUG

LF_hashtable lf_table;

// Memory management
/*
 * int ht_mm_init();
 * int ht_mm_alloc();
 * int ht_mm_free();
 */

static inline int 
is_marked_reference(node* ptr)
{
    return (uintptr_t)ptr & 1;
}

static inline node* 
get_unmarked_reference(node *ptr)
{
    return (node *)((uintptr_t)ptr & (~ (uintptr_t)1));
}

static inline node*
get_marked_reference(node *ptr)
{
    return (node *)((uintptr_t)ptr | (uintptr_t)1);
}

int 
hashtable_init()
{
#ifdef HT_DEBUG
    printf("HT INIT\n");
#endif

    for (int i = 0; i < BUCKET_SIZE; i++) {
        lf_table.lf_buckets[i].head = (node*)malloc(sizeof(node));
        lf_table.lf_buckets[i].tail = (node*)malloc(sizeof(node));
        lf_table.lf_buckets[i].head->next = lf_table.lf_buckets[i].tail;
    }

    return 0;
}

static node*
internal_hashtable_search(char *key, node **left_node, int index)
{
    node *left_node_next, *right_node, *head, *tail;

    head = lf_table.lf_buckets[index].head;
    tail = lf_table.lf_buckets[index].tail;

    while (1) {
        node *t = head;
        node *t_next = head->next;

        /* Step 1: Find left_node and right_node */
        do {
            if (!is_marked_reference(t_next)) {
                *left_node = t;
                left_node_next = t_next;
            }
            t = get_unmarked_reference(t_next);
            if (t == tail) 
                break;
            t_next = t->next;
        } while (is_marked_reference(t_next) || (strcmp(t->key, key) < 0));

        right_node = t;
    
        /* Step 2: Check nodes are adjacent */
        if (left_node_next == right_node) {
            if ((right_node != tail) && is_marked_reference(right_node->next))
                continue;
            else
                return right_node;
        }

        /* Step 3: Remove one or more marked nodes */
        if (CAS(&((*left_node)->next), left_node_next, right_node)) {
            // TODO: push back to the free pool
            free(left_node_next);
            INC(&lf_table.size, -1);

            if ((right_node != tail) && is_marked_reference(right_node->next))
                continue;
            else
                return right_node;
        }
    }
}

int 
hashtable_insert(char *key, char *value)
{
    node *new_node, *right_node, *left_node;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
#ifdef HT_DEBUG
    printf("HT INSERT key: %s value: %s bucket index: %d\n", key, value, index);
#endif
    //TODO: allocate from the free pool
    new_node = (node *)malloc(sizeof(node));
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);

    do {
        right_node = internal_hashtable_search(key, &left_node, index);

        if ((right_node != lf_table.lf_buckets[index].tail) && 
                (!strcmp(right_node->key, key))) {
            strcpy(right_node->value, value);
            free(new_node);
            return 0;
        }

        new_node->next = right_node;

        if (CAS(&(left_node->next), right_node, new_node)) {
            INC(&lf_table.size, 1);
            return 0;
        }
    } while (1);

    return 0;
}

int 
hashtable_delete(char *key)
{
    node *right_node, *right_node_next, *left_node, *tail;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
    tail = lf_table.lf_buckets[index].tail;

#ifdef HT_DEBUG
    printf("HT DELETE key: %s bucket index: %d\n", key, index);
#endif

    do {
        right_node = internal_hashtable_search(key, &left_node, index);
        if ((right_node == tail) || (strcmp(right_node->key, key)))
            return 0;

        right_node_next = right_node->next;
        if (!is_marked_reference(right_node_next)) {
            if (CAS((&right_node->next), right_node_next, 
                        get_marked_reference(right_node_next)))
                break;
        }
    } while (1);

    if (!CAS(&(left_node->next), right_node, right_node_next)) {
        right_node = internal_hashtable_search(key, &left_node, index);
    } else {
        //TODO: push back to the free pool
        free(right_node);
        INC(&lf_table.size, -1);
    }

    return 0;
}

char*
hashtable_find(char *key)
{
    node *right_node, *left_node, *tail;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
    tail = lf_table.lf_buckets[index].tail;

#ifdef HT_DEBUG
    printf("HT FIND key: %s bucket index: %d\n", key, index);
#endif

    right_node = internal_hashtable_search(key, &left_node, index);
    if ((right_node == tail) || (strcmp(right_node->key, key)))
        return NULL;
    else
        return right_node->value;
}

int
hashtable_dump()
{
    printf("Hashtable size is %d\n", lf_table.size);
    for (int i = 0; i < BUCKET_SIZE; i++) {
        node *begin = lf_table.lf_buckets[i].head->next;
        node *tail = lf_table.lf_buckets[i].tail;

        uint32_t cnt = 0;
        while (begin != tail) {
            cnt++;
            begin = begin->next;
        }
        printf("Bucket %d has %d items\n", i, cnt);

#if 0 
        begin = lf_table.lf_buckets[i].head->next;
        printf("Bucket %d BEGIN: ", i);
        while (begin != tail) {
            printf("<key: %s, value: %s> --> ", begin->key, begin->value);
            begin = begin->next;
        }
        printf("END\n");
#endif
    }

    return 0;
}

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
#include <pthread.h>
#include "util.h"

#define HT_DEBUG

LF_hashtable lf_table;
LF_memory lf_memory;

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

static int
malloc_hash_memory()
{
    return lf_memory.pop();
}

static int
free_hash_memory(int free_pt)
{
    lf_memory.push(free_pt);

    return 0;
}

/* 
 * Evict policy: LFU (least-frequently used). We choose this policy due to 
 * the Zipf distribution.
 */
int hashtable_delete(char *key);
static void
evict_hash_memory()
{
    uint64_t min, max;

    max = 0;
    min = UINT64_MAX;
    for (int i = 0; i < HT_SIZE; i++) {
        if (lf_memory.data_item[i].freq > max)
            max = lf_memory.data_item[i].freq;
        if (lf_memory.data_item[i].freq && 
                (lf_memory.data_item[i].freq < min)) 
            min = lf_memory.data_item[i].freq;
    }

    max = max / 10; // simulate log value
    min = (max > min) ? max : min;

    if ((!min) || (min == UINT64_MAX))
        return;

    for (int i = 0; i < HT_SIZE; i++) {
        if ((lf_memory.data_item[i].freq) &lf_memory.data_item[i].freq <= min) {
            hashtable_delete(lf_memory.data_item[i].node->key);
            lf_memory.push(i);
        }
    }
}

static void 
data_item_push(int pt)
{
    pthread_mutex_lock(&lf_memory.alloc_lock);
    lf_memory.data_item[pt].node = NULL;
    lf_memory.data_item[pt].freq = 0;
    lf_memory.free_list[lf_memory.alloc_stack_pt] = pt;
    lf_memory.alloc_stack_pt++;
    pthread_mutex_unlock(&lf_memory.alloc_lock);
}

static int
data_item_pop()
{
    int pt;

    if (!lf_memory.alloc_stack_pt)
        return -1;

    pthread_mutex_lock(&lf_memory.alloc_lock);
    if (!lf_memory.alloc_stack_pt) {
        pthread_mutex_unlock(&lf_memory.alloc_lock);
        return -1;
    }
    pt = lf_memory.alloc_stack_pt - 1;
    lf_memory.alloc_stack_pt--;
    pthread_mutex_unlock(&lf_memory.alloc_lock);

    if (!lf_memory.alloc_stack_pt)
        evict_hash_memory();

    return pt;
}

int 
hashtable_init()
{
#ifdef HT_DEBUG
    printf("HT INIT\n");
#endif

    // Init table
    for (int i = 0; i < BUCKET_SIZE; i++) {
        lf_table.lf_buckets[i].head = (node*)malloc(sizeof(node));
        lf_table.lf_buckets[i].tail = (node*)malloc(sizeof(node));
        lf_table.lf_buckets[i].head->next = lf_table.lf_buckets[i].tail;
    }

    // Init memory
    lf_memory.push = data_item_push;
    lf_memory.pop = data_item_pop;
    lf_memory.alloc_stack_pt = 0;
    if (pthread_mutex_init(&lf_memory.alloc_lock, NULL)) {
        printf("pthread_mutex_init failed\n");
        return 0;
    }
    for (int i = 0; i < HT_SIZE; i++) {
        lf_memory.data_item[i].data = malloc(sizeof(char) * ITEM_SIZE);
        lf_memory.data_item[i].node = NULL;
        lf_memory.data_item[i].freq = 0;
        lf_memory.push(i);
    }

    return 0;
}

int hashtable_dump();
static node*
internal_hashtable_search(char *key, node **left_node, int index)
{
    node *left_node_next, *right_node, *head, *tail;

    head = lf_table.lf_buckets[index].head;
    tail = lf_table.lf_buckets[index].tail;

    //hashtable_dump();

    while (1) {
        node *t = head;
        node *t_next = head->next;

        /* Step 1: Find left_node and right_node */
        do {
            //printf("delete key %s t->key %s t %p head %p tail %p tail->next %p\n", key, t->key, t, head, tail, tail->next);
            //data_item %d next %p\n", t->key, t->data_item, t->next);
            //printf("ok\n");
            if (!is_marked_reference(t_next)) {
                *left_node = t;
                left_node_next = t_next;
            }
            //printf("ok1\n");
            t = get_unmarked_reference(t_next);
            //printf("ok2 t %p t_next %p\n", t, t_next);
            if (t == tail) 
                break;
            //printf("ok3 %s t %p head %p tail %p\n", t->key, t, head, tail);
            t_next = t->next;
        } while (is_marked_reference(t_next) || (strcmp(t->key, key) < 0));
        //printf("test3\n");

        right_node = t;
    
        /* Step 2: Check nodes are adjacent */
        if (left_node_next == right_node) {
            if ((right_node != tail) && is_marked_reference(right_node->next))
                continue;
            else
                return right_node;
        }
        //printf("test4\n");

        /* Step 3: Remove one or more marked nodes */
        if (CAS(&((*left_node)->next), left_node_next, right_node)) {
            free_hash_memory(left_node_next->data_item);
            INC(&lf_table.size, -1);

            if ((right_node != tail) && is_marked_reference(right_node->next))
                continue;
            else
                return right_node;
        }
        //printf("test5\n");
    }
}

int 
hashtable_insert(char *key, char *value)
{
    int alloc_pt = -1;
    node *new_node, *right_node, *left_node;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
#ifdef HT_DEBUG
    //printf("HT INSERT key: %s value: %s bucket index: %d\n", key, value, index);
    //printf("HT INSERT key: %s bucket index: %d\n", key, index);
#endif
    while ((alloc_pt = malloc_hash_memory()) == -1)
        ;
    new_node = (node *)lf_memory.data_item[alloc_pt].data;

    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->data_item = alloc_pt;

    do {
        right_node = internal_hashtable_search(key, &left_node, index);

        if ((right_node != lf_table.lf_buckets[index].tail) && 
                (!strcmp(right_node->key, key))) {
            strcpy(right_node->value, value);
            free_hash_memory(alloc_pt);
            return 0;
        }

        new_node->next = right_node;

        if (CAS(&(left_node->next), right_node, new_node)) {
            lf_memory.data_item[alloc_pt].node = new_node;
            lf_memory.data_item[alloc_pt].freq++;
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
    printf("right_node->next %p right_node_next %p\n", right_node->next, right_node_next);
        if (!is_marked_reference(right_node_next)) {
            if (CAS((&right_node->next), right_node_next, 
                        get_marked_reference(right_node_next))) {
                break;
            }
        }
    } while (1);

    //printf("right_node->next %p right_node_next %p\n", right_node->next, right_node_next);

    if (!CAS(&(left_node->next), right_node, right_node_next)) {
        right_node = internal_hashtable_search(key, &left_node, index);
    } else {
        free_hash_memory(right_node->data_item);
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
    //printf("HT FIND key: %s bucket index: %d\n", key, index);
#endif

    right_node = internal_hashtable_search(key, &left_node, index);
    if ((right_node == tail) || (strcmp(right_node->key, key)))
        return NULL;
    else {
        lf_memory.data_item[right_node->data_item].freq++;
        return right_node->value;
    }
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

#if 1 
        begin = lf_table.lf_buckets[i].head->next;
        printf("Bucket %d BEGIN: ", i);
        while (begin != tail) {
            //printf("<key: %s, value: %s> --> ", begin->key, begin->value);
            printf("<key: %s> --> ", begin->key);
            begin = begin->next;
        }
        printf("END\n");
#endif
    }

    return 0;
}

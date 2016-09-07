/*
 * Description: A lock-free hashtable implementation.
 *
 * Note:
 * (1) The memory management is a locked one based on the free-list concept.
 * (2) This implementation has been tested on a Dell server, enclosing two Intel
 * Xeon X5650 processors (each has 6 cores) and 24G memory.
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

//#define HT_DEBUG

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

    // Try to evict more for future allocaitons
    for (int i = 0; i < HT_SIZE; i++) {
        if ((lf_memory.data_item[i].freq) && 
                (lf_memory.data_item[i].freq <= min))
            hashtable_delete(lf_memory.data_item[i].node->key);
    }
}

static void 
data_item_push(int pt)
{
    if (pthread_rwlock_wrlock(&lf_memory.data_item[pt].rw_lock)) {
        printf("pthread_rwlock_wrlock failed\n");
        return;
    }

    lf_memory.data_item[pt].node = NULL;
    lf_memory.data_item[pt].freq = 0;
    if (pthread_rwlock_unlock(&lf_memory.data_item[pt].rw_lock)) {
        printf("pthread_rwlock_unlock failed\n");
        return;
    }

    pthread_mutex_lock(&lf_memory.alloc_lock);
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
    pt = lf_memory.free_list[lf_memory.alloc_stack_pt - 1];
    lf_memory.alloc_stack_pt--;
    pthread_mutex_unlock(&lf_memory.alloc_lock);

    // Make sure evict_hash_memory can evict something
    while (!lf_memory.alloc_stack_pt)
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
        if (pthread_rwlock_init(&lf_memory.data_item[i].rw_lock,NULL)) {
            printf("pthread_rwlock_init failed\n");
            return 0;
        }
        lf_memory.push(i);
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
        int cmp_ret = 0;

        /* Step 1: Find left_node and right_node */
        do {
            //An internal debugging test
            //printf("internal key %s t->key %s t_next->key %s\n", 
            //key, t->key, t_next->key);
            if (!is_marked_reference(t_next)) {
                *left_node = t;
                left_node_next = t_next;
            }
            t = get_unmarked_reference(t_next);
            if (t == tail)
                break;

            if (t) {
                if (pthread_rwlock_rdlock(
                            &lf_memory.data_item[t->data_item].rw_lock)) {
                    printf("pthread_rwlock_rdlock failed\n");
                    return NULL;
                }
                t_next = t->next;
                cmp_ret = strcmp(t->key, key);
                if (pthread_rwlock_unlock(
                            &lf_memory.data_item[t->data_item].rw_lock)) {
                    printf("pthread_rwlock_unlock failed\n");
                    return NULL;
                }
            } else {
                t = head;
                t_next = head->next;
            }
        } while (is_marked_reference(t_next) || (cmp_ret < 0));

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
            free_hash_memory(left_node_next->data_item);
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
    int alloc_pt = -1;
    node *new_node, *right_node, *left_node;
    if (!key) return 0;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
#ifdef HT_DEBUG
    printf("HT INSERT key: %s bucket index: %d\n", key, index);
#endif
    while ((alloc_pt = malloc_hash_memory()) == -1) {
        ;
    }

    new_node = (node *)lf_memory.data_item[alloc_pt].data;

    // No need to hold the rw lock since it's not visible
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->data_item = alloc_pt;

    do {
        right_node = internal_hashtable_search(key, &left_node, index);

        if (pthread_rwlock_wrlock(
                    &lf_memory.data_item[right_node->data_item].rw_lock)) {
            printf("pthread_rwlock_wrlock failed\n");
            return 0;
        }
        if ((right_node != lf_table.lf_buckets[index].tail) && 
                (!strcmp(right_node->key, key))) {
            strcpy(right_node->value, value);
            if (pthread_rwlock_unlock(
                        &lf_memory.data_item[right_node->data_item].rw_lock)) {
                printf("pthread_rwlock_unlock failed\n");
                return 0;
            }
            free_hash_memory(alloc_pt);
            return 0;
        }
        new_node->next = right_node;
        if (pthread_rwlock_unlock(
                    &lf_memory.data_item[right_node->data_item].rw_lock)) {
            printf("pthread_rwlock_unlock failed\n");
            return 0;
        }

        if (CAS(&(left_node->next), right_node, new_node)) {
            // Visible point. Need to acquire a lock
            if (pthread_rwlock_wrlock(&lf_memory.data_item[alloc_pt].rw_lock)) {
                printf("pthread_rwlock_wrlock failed\n");
                return 0;
            }
            lf_memory.data_item[alloc_pt].node = new_node;
            if (pthread_rwlock_unlock(&lf_memory.data_item[alloc_pt].rw_lock)) {
                printf("pthread_rwlock_unlock failed\n");
                return 0;
            }
            INC(&lf_memory.data_item[alloc_pt].freq, 1);
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
    if (!key) return 0;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
    tail = lf_table.lf_buckets[index].tail;

#ifdef HT_DEBUG
    printf("HT DELETE key: %s bucket index: %d\n", key, index);
#endif

    do {
        right_node = internal_hashtable_search(key, &left_node, index);

        if (pthread_rwlock_rdlock(
                    &lf_memory.data_item[right_node->data_item].rw_lock)) {
            printf("pthread_rwlock_rdlock failed\n");
            return 0;
        }
        int cmp_ret = strcmp(right_node->key, key);

        if ((right_node == tail) || (cmp_ret)) {
            if (pthread_rwlock_unlock(
                        &lf_memory.data_item[right_node->data_item].rw_lock))
                printf("pthread_rwlock_unlock failed\n");

            return 0;
        }

        if (pthread_rwlock_unlock(
                    &lf_memory.data_item[right_node->data_item].rw_lock)) {
            printf("pthread_rwlock_unlock failed\n");
            return 0;
        }

        right_node_next = right_node->next;
        if (!is_marked_reference(right_node_next)) {
            if (CAS((&right_node->next), right_node_next, 
                        get_marked_reference(right_node_next))) {
                break;
            }
        }
    } while (1);

    if (!CAS(&(left_node->next), right_node, right_node_next)) {
        right_node = internal_hashtable_search(key, &left_node, index);
    } else {
        free_hash_memory(right_node->data_item);
        INC(&lf_table.size, -1);
    }

    return 0;
}

node*
hashtable_find(char *key)
{
    node *right_node, *left_node, *tail;
    if (!key) return NULL;
    uint32_t index = jenkins_hash(key, KEY_LEN) % BUCKET_SIZE;
    tail = lf_table.lf_buckets[index].tail;

#ifdef HT_DEBUG
    printf("HT FIND key: %s bucket index: %d\n", key, index);
#endif

    right_node = internal_hashtable_search(key, &left_node, index);

    if (pthread_rwlock_rdlock(
                &lf_memory.data_item[right_node->data_item].rw_lock)) {
        printf("pthread_rwlock_rdlock failed\n");
        return NULL;
    }
    int cmp_ret = strcmp(right_node->key, key);

    if ((right_node == tail) || (cmp_ret)) {
        if (pthread_rwlock_unlock(
                    &lf_memory.data_item[right_node->data_item].rw_lock))
            printf("pthread_rwlock_unlock failed\n");

        return NULL;
    }
    else {
        INC(&lf_memory.data_item[right_node->data_item].freq, 1);
        return right_node;
    }
}

int
hashtable_find_end(node *node)
{
    if (pthread_rwlock_unlock(&lf_memory.data_item[node->data_item].rw_lock))
        printf("pthread_rwlock_unlock failed\n");

    return 0;
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
            //printf("<key: %s, value: %s> --> ", begin->key, begin->value);
            printf("<key: %s Item %d> --> ", begin->key, begin->data_item);
            begin = begin->next;
        }
        printf("END\n");
#endif
    }

    return 0;
}

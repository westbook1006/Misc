/*
 * A concurrent hashtable header
 */
#ifndef _CONCURRENT_HT
#define _CONCURRENT_HT

#include <stddef.h>
#include <pthread.h>

#define INTEL

#ifdef INTEL
#define CACHE_ALIGN __attribute__((aligned(64)))
#else
#define CACHE_ALIGN __attribute__((aligned(128)))
#endif

typedef enum _ht_status {
    HT_FOUND,
    HT_KEY_DUPLICATED,
} ht_status;

typedef struct _concurrent_ht_t {
    size_t hash_power;          /* 2 ^ hash_power buckets in total */
    void *buckets;              /* pointer to the array of buckets */
    void *keyver_array;         /* an array of version counters */

#ifdef INTEL
    pthread_mutex_t lock;       /* single write lock */
#else
    // TODO:
#endif

} CACHE_ALIGN concurrent_ht_t;

/*
 * @brief Initialize the hash table
 *
 * @init_hash_power: the logarithm of the inital table size
 * @return: the handler to the hashtable on success, NULL on failure
 */
concurrent_ht_t* concur_hashtable_init(const size_t init_hashpower);

/*
 * @brief Clean up the hash table
 *
 * @ht: the handler of the hashtable
 */
void concur_hashtable_free(concurrent_ht_t *ht);

/*
 * @brief Insert the <key, val> pair into the hashtable
 *
 * @ht: the handler of the hashtable
 * @key: key to inserted
 * @val: value ot inserted
 * @return: the hashtable status
 */
ht_status concur_hashtable_insert(concurrent_ht_t *ht,
                                  const char *key,
                                  const char *val);

#endif /* _CONCURRENT_HT */

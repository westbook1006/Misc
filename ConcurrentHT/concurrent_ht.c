/*
 * A concurrent hashtable implementation, based on MemC3 (NSDI'13)
 *
 * Features:
 *   (1) CLOCK cache eviction;
 *   (2) concurrent cuckoo hashing;
 *   (3) single-wirte/multi-reader cuckoo hash, 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "concurrent_ht.h"
#include "city.h"

#define HASHPOWER_DEFAULT 16                    /* default hash table size */
#define BUCKET_SIZE       4                     /* 4-way set-associate     */
#define KEY_SIZE          32                    /* Maximum size of the key */
#define VALUE_SIZE        1024                  /* Maximum size of the value */
#define KEY_VER_SIZE      ((uint32_t) 1 << 13)  /* key version size */
#define KEY_VER_MASK      (KEY_VER_SIZE - 1)    /* key version mask */

typedef struct _key_t {
    size_t key_len;
    char key[KEY_SIZE];
} __attribute__((packed)) ht_key_t;

typedef struct _val_t {
    size_t value_len;
    char value[VALUE_SIZE];
} __attribute__((packed)) ht_val_t;

typedef struct _kv_t {
    ht_key_t key;
    ht_val_t value;
} __attribute__((packed)) ht_kv_t;

/* bucket type in the hashtable */
typedef struct _bucket_t {
    uint32_t tags[BUCKET_SIZE];
    ht_kv_t* kv_data[BUCKET_SIZE];
} CACHE_ALIGN ht_bucket_t;

#define HASH_SIZE(n)      ((uint32_t) 1 << n)
#define HASH_MASK(n)     (HASH_SIZE(n) - 1)

static inline uint32_t
get_key_hash(const char *key, size_t len)
{
    return CityHash32(key, len);
}

/* Hash of the first bucket */
static inline size_t
get_first_index (concurrent_ht_t *ht,
                 const uint32_t hv)
{
    return (hv & HASH_MASK(ht->hash_power));
}

/* Hash of the second bucket */
static inline size_t
get_second_index (concurrent_ht_t *ht,
                  const uint32_t hv,
                  const size_t index)
{
    // magic number (i.e. 0x5bd1e995) is the hash constant from MurmurHash2 
    uint32_t tag = hv >> 24;
    return (index ^ (tag * 0x5bd1e995)) & HASH_MASK(ht->hash_power);
}

static inline void
mylock_init (concurrent_ht_t *ht)
{
#ifdef INTEL
    pthread_mutex_init(&ht->lock, NULL);
#else
#endif
}

static inline void
mylock_destroy (concurrent_ht_t *ht)
{
#ifdef INTEL
    pthread_mutex_destroy(&ht->lock);
#else
#endif
}

static inline void
mylock_lock (concurrent_ht_t *ht)
{
#ifdef INTEL
    pthread_mutex_lock(&ht->lock);
#else
#endif
}

static inline void
mylock_unlock (concurrent_ht_t *ht)
{
#ifdef INTEL
    pthread_mutex_unlock(&ht->lock);
#else
#endif
}

static inline uint32_t
keylock_index (const uint32_t hv)
{
    return (hv & KEY_VER_MASK);
}

#define KVC_START_READ(ht, index) \
  __sync_fetch_and_add(&((uint32_t *)ht->keyver_array)[index & KEY_VER_MASK], 0)

#define KVC_END_READ(ht, index, ret) \
    do { \
        asm volatile ("" ::: "memory"); \
        ret = ((uint32_t *)ht->keyver_array)[index & KEY_VER_MASK]; \
    } while (0)

#define TABLE_TAG(ht, i, j) ((ht_bucket_t *)ht->buckets)[i].tags[j]
#define TABLE_KEY(ht, i, j) ((ht_bucket_t *)ht->buckets)[i].kv_data[j].key
#define TABLE_VAL(ht, i, j) ((ht_bucket_t *)ht->buckets)[i].kv_data[j].value

static bool
read_from_bucket(concurrent_ht_t *ht,
                 uint32_t tag,
                 size_t index,
                 const char *key,
                 ht_val_t *val_ptr)
{
    size_t i;

    for (i = 0; i < BUCKET_SIZE; i++) {
    }
}

static ht_status
cuckoo_find (concurrent_ht_t *ht,
             const char *key,
             ht_val_t *valptr,
             uint32_t val,
             size_t i1,
             size_t i2,
             uint32_t keylock)
{
    bool ret;
    uint32_t vc_start, vc_end;

    vc_start = KVC_START_READ(ht, keylock);


    return HT_FOUND;
}

/******************************************************************/

concurrent_ht_t* 
concur_hashtable_init(const size_t init_hashpower)
{
    size_t i, j, total_buckets;

    concurrent_ht_t *ht = (concurrent_ht_t *)malloc(sizeof(concurrent_ht_t));
    if ( !ht )
        goto err;

    ht->hash_power = (init_hashpower > 0) ? init_hashpower : HASHPOWER_DEFAULT;
    total_buckets = HASH_SIZE(ht->hash_power);

    ht->buckets = malloc(total_buckets * sizeof(ht_bucket_t) +
                         total_buckets * sizeof(ht_kv_t) * BUCKET_SIZE);
    if ( !ht->buckets )
        goto err;

    ht->keyver_array = malloc(KEY_VER_SIZE * sizeof(uint32_t));
    if ( !ht->keyver_array )
        goto err;

    mylock_init(ht);
    memset(ht->buckets, 0x00, total_buckets * sizeof(ht_bucket_t) +
                              total_buckets * sizeof(ht_kv_t) * BUCKET_SIZE);
    memset(ht->keyver_array, 0x00, KEY_VER_SIZE * sizeof(uint32_t));

    for (i = 0; i < total_buckets; i++) {
        for (j = 0; j < BUCKET_SIZE; j++) {
            ((ht_bucket_t *)(ht->buckets + i))->kv_data[j] = 
                ht->buckets + 
                total_buckets * sizeof(ht_bucket_t) + 
                (i * BUCKET_SIZE + j) * sizeof(ht_kv_t);
        }
    }

    return ht;

err:
    if (ht) {
        free(ht->buckets);
        free(ht->keyver_array);
    }

    free(ht);
    return NULL;
}

void
concur_hashtable_free(concurrent_ht_t *ht)
{
    mylock_destroy(ht);
    free(ht->buckets);
    free(ht->keyver_array);

    free(ht);
}

ht_status 
concur_hashtable_insert(concurrent_ht_t *ht, 
                        const char *key, 
                        const char *val)
{
    uint32_t hv = get_key_hash (key, strlen(key));
    size_t i1 = get_first_index(ht, hv);
    size_t i2 = get_second_index(ht, hv, i1);
    uint32_t key_lock = keylock_index(hv);

    ht_status ret;
    ht_val_t old_value;

    mylock_lock(ht);

    ret = cuckoo_find (ht, key, &old_value, hv, i1, i2, key_lock);
    if (ret == HT_FOUND) {
        mylock_unlock(ht);
        return  HT_KEY_DUPLICATED;
    }

    mylock_unlock(ht);

    return ret;
}

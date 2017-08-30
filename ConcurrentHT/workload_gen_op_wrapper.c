/*
 * Operation wrapper implementation
 */
#include <sys/time.h>
#include <assert.h>
#include "workload_gen_op_wrapper.h"
#include "concurrent_ht.h"

concurrent_ht_t *my_db;

void 
wl_gen_db_init()
{
    my_db = concur_hashtable_init(20);
    assert(my_db);
}

void
wl_gen_db_exit()
{
    concur_hashtable_report(my_db);
    concur_hashtable_free(my_db);
}

void 
wl_gen_set(wl_gen *gen, const char *key, const char *val)
{
#ifdef PROFILE
    struct timeval start, end;
    uint64_t elapsed = 0;

    gettimeofday(&start, NULL);
#endif

    concur_hashtable_insert(my_db, key, val);

#ifdef PROFILE
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000 +
              (end.tv_usec - start.tv_usec);
    __sync_fetch_and_add(&gen->set_tot_time, elapsed);
    __sync_fetch_and_add(&gen->set_tot_num, 1);
#endif
}

void 
wl_gen_get(wl_gen *gen, const char *key, char *val)
{
#ifdef PROFILE
    struct timeval start, end;
    uint64_t elapsed = 0;

    gettimeofday(&start, NULL);
#endif

    concur_hashtable_find(my_db, key, val);

#ifdef PROFILE
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000 +
              (end.tv_usec - start.tv_usec);
    __sync_fetch_and_add(&gen->get_tot_time, elapsed);
    __sync_fetch_and_add(&gen->get_tot_num, 1);
#endif
}

void 
wl_gen_del(wl_gen *gen, const char *key)
{
#ifdef PROFILE
    struct timeval start, end;
    uint64_t elapsed = 0;

    gettimeofday(&start, NULL);
#endif

    concur_hashtable_delete(my_db, key);

#ifdef PROFILE
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000 +
              (end.tv_usec - start.tv_usec);
    __sync_fetch_and_add(&gen->del_tot_time, elapsed);
    __sync_fetch_and_add(&gen->del_tot_num, 1);
#endif
}

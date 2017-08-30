/*
 * A generic workload generator
 */
#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "workload_gen.h"
#include "workload_gen_op_wrapper.h"

#define TOT_KEY_NUM 1000000    /*   1M */
#define TOT_REQ_NUM 1000000    /*   1M */
#define KEY_BASE    100000000  /* 100M */

wl_gen* 
init_wl_gen(size_t val_size, size_t thread_num,
            wl_dist dist, wl_type type)
{
    wl_gen *gen = (wl_gen *)malloc(sizeof(wl_gen));
    if ( !gen )
        return NULL;

    memset(gen, 0x00, sizeof(wl_gen));
    gen->val_size = (val_size > MAX_VAL_SIZE) ? MAX_VAL_SIZE: val_size;
    gen->thread_num = thread_num;
    gen->dist = dist;
    gen->type = type;
    gen->zdist = zipf_init(TOT_KEY_NUM);
    get_zipf(0.01, TOT_KEY_NUM, gen->zdist);
    gen->zipf_marker = (int *)malloc(sizeof(int) * gen->thread_num);
    memset(gen->zipf_marker, 0x00, sizeof(int) * gen->thread_num);
    wl_gen_db_init();

    return gen;
}

void 
free_wl_gen(wl_gen *gen)
{ 
    if (gen) {
        free(gen->zdist);
        free(gen->zipf_marker);
        free(gen);
    }
}

static void
preload_data(wl_gen *gen)
{
    int i;
    char key[KEY_SIZE], *val;
    
    memset(key, 0x00, gen->key_size + 1);
    val = (char *)malloc(sizeof(char) * (gen->val_size + 1));
    memset(val, 0x66, gen->val_size + 1);
    val[gen->val_size] = 0x00;

    for (i = 0; i < TOT_KEY_NUM; i++) {
        sprintf(key, "KEY%d", i + KEY_BASE);
        key[12] = '\0';

        wl_gen_set(gen, key, val);
    }

    free(val);
}

static int
get_next_req_id(wl_gen *gen, int cur, int tid)
{
    int next, max_num;

    switch (gen->dist) {
        case SEQUENTIAL: 
            next = cur + 1;
            break;

        case RANDOM: 
            next = rand();
            break;

        case ZIP:
            max_num = (int) (TOT_KEY_NUM * 
                             gen->zdist[gen->zipf_marker[tid]].prob);

            if (cur < max_num)
                next = cur + 1;
            else {
                next = 0;
                gen->zipf_marker[tid]++;
                if (gen->zipf_marker[tid] >= TOT_KEY_NUM)
                    gen->zipf_marker[tid] = TOT_KEY_NUM;
            }

            break;
    }

    return next;
}

static void
set_100_test(wl_gen *gen, char *key, const char *val, int tid)
{
    int i, cur_req_id;
    
    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        wl_gen_set(gen, key, val);
    }
}

static void
get_100_test(wl_gen *gen, char *key, char *val, int tid)
{
    int i, cur_req_id;

    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        wl_gen_get(gen, key, val);
    }
}

static void
del_100_test(wl_gen *gen, char *key, int tid)
{
    int i, cur_req_id;

    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        wl_gen_del(gen, key);
    }
}

static void
set_95_get_5_test(wl_gen *gen, char *key, char *val, int tid)
{
    int i, cur_req_id, prob;

    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        prob = rand() % 100;
        if (prob < 5) {
            wl_gen_get(gen, key, val);
        } else {
            wl_gen_set(gen, key, val);
        }
    }
}

static void
set_50_get_50_test(wl_gen *gen, char *key, char *val, int tid)
{
    int i, cur_req_id, prob;

    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        prob = rand() % 100;
        if (prob < 50) {
            wl_gen_get(gen, key, val);
        } else {
            wl_gen_set(gen, key, val);
        }
    }
}

static void
set_5_get_95_test(wl_gen *gen, char *key, char *val, int tid)
{
    int i, cur_req_id, prob;

    for (i = 0, cur_req_id = 0; i < TOT_REQ_NUM; i++) {
        cur_req_id =  get_next_req_id(gen, cur_req_id, tid);
        cur_req_id = cur_req_id % TOT_KEY_NUM;
        sprintf(key, "KEY%d", cur_req_id + KEY_BASE);
        key[12] = '\0';

        prob = rand() % 100;
        if (prob < 95) {
            wl_gen_get(gen, key, val);
        } else {
            wl_gen_set(gen, key, val);
        }
    }
}


static int thread_id = 0;

static void*
run_test(void *arg)
{
    int my_id;
    char key[KEY_SIZE], *val;
    wl_gen *gen = (wl_gen *)arg;

    memset(key, 0x00, gen->key_size + 1);
    val = (char *)malloc(sizeof(char) * (gen->val_size + 1));
    memset(val, 0x66, gen->val_size + 1);
    val[gen->val_size] = 0x00;

    my_id = thread_id;
    while (!__sync_bool_compare_and_swap(&thread_id, my_id, my_id + 1)) {
        my_id = thread_id;
    }

    switch (gen->type) {
        case SET_100: set_100_test(gen, key, val, my_id); break;
        case GET_100: get_100_test(gen, key, val, my_id); break;
        case DEL_100: del_100_test(gen, key, my_id); break;
        case SET_95_GET_5: set_95_get_5_test(gen, key, val, my_id); break;
        case SET_50_GET_50: set_50_get_50_test(gen, key, val, my_id); break;
        case SET_5_GET_95: set_5_get_95_test(gen, key, val, my_id); break;
        default: break;
    }

    free(val);

    return (void *)NULL;
}

void 
run_wl_gen(wl_gen *gen)
{
    int i, err, core_num;
    pthread_t *threads;

    threads = (pthread_t *)malloc(sizeof(pthread_t) * gen->thread_num);
    assert(threads);
    memset(threads, 0x00, sizeof(pthread_t) * gen->thread_num);
    core_num = sysconf(_SC_NPROCESSORS_ONLN);
    preload_data(gen);

    for (i = 0; i < gen->thread_num; i++) {
        err = pthread_create(&threads[i], NULL, &run_test, (void *)gen);
        assert(!err);

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i % core_num, &cpuset);
        pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
    }

    for (i = 0; i < gen->thread_num; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
}

void 
report_wl_gen(wl_gen *gen)
{
    wl_gen_db_exit();

#ifdef PROFILE
    printf("SET performance:\n");
    printf("Latency: %.2f us\n", (gen->set_tot_time + 0.0) /
                                 (gen->set_tot_num + 0.0));
    printf("Throughput: %.2f MOPS\n", (gen->set_tot_num + 0.0) * 1000000 /
                                      (gen->set_tot_time + 0.0));

    printf("GET performance: \n");
    printf("Latency: %.2f us\n", (gen->get_tot_time + 0.0) /
                                 (gen->get_tot_num + 0.0));
    printf("Throughput: %.2f MOPS\n", (gen->get_tot_num + 0.0) * 1000000 /
                                      (gen->get_tot_time + 0.0));

    printf("DEL performance: \n");
    printf("Latency: %.2f us\n", (gen->del_tot_time + 0.0) /
                                 (gen->del_tot_num + 0.0));
    printf("Throughput: %.2f MOPS\n", (gen->del_tot_num + 0.0) * 1000000 /
                                      (gen->del_tot_time + 0.0));
#endif
}

/*
 * A generic workload generator header
 */
#ifndef _WORKLOAD_GEN_H
#define _WORKLOAD_GEN_H

#include <stdint.h>
#include "zipf.h"

typedef enum _dist{
    SEQUENTIAL,
    RANDOM,
    ZIP,
} wl_dist;

typedef enum _type{
    SET_100,
    GET_100,
    DEL_100,
    SET_95_GET_5,
    SET_50_GET_50,
    SET_5_GET_95,
} wl_type;

#define KEY_SIZE 32
#define MAX_VAL_SIZE 1024

#define PROFILE

typedef struct _wl_gen {
    size_t key_size;
    size_t val_size;
    size_t key_num;
    size_t thread_num;
    wl_dist dist;
    wl_type type;
    struct probvals *zdist;
    int *zipf_marker;

#ifdef PROFILE
    uint64_t set_tot_num;
    uint64_t set_tot_time;
    uint64_t get_tot_num;
    uint64_t get_tot_time;
    uint64_t del_tot_num;
    uint64_t del_tot_time;
#endif
} __attribute__((aligned(64))) wl_gen;

/*
 * @brief Init the workload generator
 * 
 * @val_size: the size of hte value;
 * @thread_num: the number of the thread
 * @dist: the distribution
 * @type: the test type
 */
wl_gen* init_wl_gen(size_t val_size, size_t thread_num,
                    wl_dist dist, wl_type type);

/*
 * @brief Free the workload generator
 *
 * @gen: the handler of the workload generator
 */
void free_wl_gen(wl_gen *gen);

/*
 * @brief Start the workload generator 
 *
 * @gen: the handler of the workload generator
 */
void run_wl_gen(wl_gen *gen);

/*
 * @brekf Report the performance statistics
 *
 * @gen: the handler of the workload generator
 */
void report_wl_gen(wl_gen *gen);

#endif /* _WORKLOAD_GEN_H */

/*
 * Operation wrapper for the workload generator
 */
#ifndef _WORKLOAD_GEN_OP_WRAPPER_H
#define _WORKLOAD_GEN_OP_WRAPPER_H

#include <stddef.h>
#include "workload_gen.h"

/*
 * @brief: init the database
 */
void wl_gen_db_init();

/*
 * @brief: exit the database
 */
void wl_gen_db_exit();

/*
 * @brief: set operation wrapper
 *
 * @gen: the handler of the workload generator
 * @key: the set key
 * @val: the set val
 */
void wl_gen_set(wl_gen *gen, const char *key, const char *val);

/*
 * @brief: get operation wrapper
 *
 * @gen: the handler of the workload generator
 * @key: the get key
 * @val: the hanlder of the get value
 */
void wl_gen_get(wl_gen *gen, const char *key, char *val);

/*
 * @brief: del operation wrapper
 *
 * @gen: the handler of the worklaod generator
 * @key: the delete key
 */
void wl_gen_del(wl_gen *gen, const char *key);

#endif /* _WORKLOAD_GEN_OP_WRAPPER_H */

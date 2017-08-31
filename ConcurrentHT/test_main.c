/*
 * Test example about concurrent hashtable.
 */

#include <stdio.h>
#include <stdlib.h>
#include "concurrent_ht.h"
#include "workload_gen.h"

static void
small_scale_insert_only()
{
    size_t i;
    ht_status ret;
    char key[8], val[8];

    concurrent_ht_t *ht = concur_hashtable_init(1);

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);

        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    concur_hashtable_report(ht);
    concur_hashtable_free(ht);
}

static void
small_scale_insert_dup()
{
    size_t i;
    ht_status ret;
    char key[8], val[8];

    concurrent_ht_t *ht = concur_hashtable_init(1);

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);

        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);

        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    concur_hashtable_report(ht);
    concur_hashtable_free(ht);
}


static void
small_scale_insert_get()
{
    size_t i;
    ht_status ret;
    char key[8], val[8], get_val[8];

    concurrent_ht_t *ht = concur_hashtable_init(1);

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);
        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    // Get
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);

        ret = concur_hashtable_find(ht, key, get_val);

        if (ret == HT_FOUND) {
            printf("Get ID: %lu Key: %s Get_Value: %s\n", i, key, get_val);
        } else {
            printf("Get ID: %lu Key: %s Get_Value: NULL\n", i, key);
        }

        concur_hashtable_print_status(ret);
    }

    concur_hashtable_report(ht);
    concur_hashtable_free(ht);
}

static void
small_scale_insert_del()
{
    size_t i;
    ht_status ret;
    char key[8], val[8];

    concurrent_ht_t *ht = concur_hashtable_init(1);

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);
        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    // Del
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);

        ret = concur_hashtable_delete(ht, key);
        printf("Delete: ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    concur_hashtable_report(ht);
    concur_hashtable_free(ht);
}

static void
small_scale_insert_del_get()
{
    size_t i;
    ht_status ret;
    char key[8], val[8], get_val[8];

    concurrent_ht_t *ht = concur_hashtable_init(1);

    // Insert
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);
        sprintf(val, "%lu", i);

        ret = concur_hashtable_insert(ht, key, val);
        printf("Insert ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    // Del
    for (i = 0; i < 4; i++) {
        sprintf(key, "%lu", i);

        ret = concur_hashtable_delete(ht, key);
        printf("Delete: ID: %lu\n", i);
        concur_hashtable_print_status(ret);
    }

    // Gel
    for (i = 0; i < 16; i++) {
        sprintf(key, "%lu", i);

        ret = concur_hashtable_find(ht, key, get_val);

        if (ret == HT_FOUND) {
            printf("Get ID: %lu Key: %s Get_Value: %s\n", i, key, get_val);
        } else {
            printf("Get ID: %lu Key: %s Get_Value: NULL\n", i, key);
        }

        concur_hashtable_print_status(ret);
    }
}

static void
large_scale_insert_only()
{
    //wl_gen *gen = init_wl_gen(64, 1, SEQUENTIAL, SET_100);
    //wl_gen *gen = init_wl_gen(64, 1, RANDOM, SET_100);
    //wl_gen *gen = init_wl_gen(64, 1, ZIP, SET_100);

    //wl_gen *gen = init_wl_gen(1024, 1, SEQUENTIAL, SET_100);
    //wl_gen *gen = init_wl_gen(1024, 1, RANDOM, SET_100);
    wl_gen *gen = init_wl_gen(1024, 1, ZIP, SET_100);

    run_wl_gen(gen);
    report_wl_gen(gen);
    free_wl_gen(gen);
}

static void
large_scale_get_only()
{
    //wl_gen *gen = init_wl_gen(64, 1, SEQUENTIAL, GET_100);
    //wl_gen *gen = init_wl_gen(64, 1, RANDOM, GET_100);
    //wl_gen *gen = init_wl_gen(64, 1, ZIP, GET_100);

    //wl_gen *gen = init_wl_gen(1024, 1, SEQUENTIAL, GET_100);
    //wl_gen *gen = init_wl_gen(1024, 1, RANDOM, GET_100);
    wl_gen *gen = init_wl_gen(1024, 1, ZIP, GET_100);

    run_wl_gen(gen);
    report_wl_gen(gen);
    free_wl_gen(gen);
}

static void
large_scale_del_only()
{
    //wl_gen *gen = init_wl_gen(64, 1, SEQUENTIAL, DEL_100);
    //wl_gen *gen = init_wl_gen(64, 1, RANDOM, DEL_100);
    //wl_gen *gen = init_wl_gen(64, 1, ZIP, DEL_100);

    //wl_gen *gen = init_wl_gen(1024, 1, SEQUENTIAL, DEL_100);
    //wl_gen *gen = init_wl_gen(1024, 1, RANDOM, DEL_100);
    wl_gen *gen = init_wl_gen(1024, 1, ZIP, DEL_100);

    run_wl_gen(gen);
    report_wl_gen(gen);
    free_wl_gen(gen);
}

static void
large_scale_get_intensive_only()
{
    //wl_gen *gen = init_wl_gen(64, 24, SEQUENTIAL, SET_5_GET_95);
    //wl_gen *gen = init_wl_gen(64, 24, RANDOM, SET_5_GET_95);
    //wl_gen *gen = init_wl_gen(64, 24, ZIP, SET_5_GET_95);

    //wl_gen *gen = init_wl_gen(1024, 24, SEQUENTIAL, SET_5_GET_95);
    //wl_gen *gen = init_wl_gen(1024, 24, RANDOM, SET_5_GET_95);
    wl_gen *gen = init_wl_gen(1024, 24, ZIP, SET_5_GET_95);

    run_wl_gen(gen);
    report_wl_gen(gen);
    free_wl_gen(gen);
}

int main (int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s [scenario]\n", argv[0]);
        printf("\t0 -> small scale insert only\n");
        printf("\t1 -> small scale insert/get\n");
        printf("\t2 -> small scale insert/del\n");
        printf("\t3 -> small scale insert/get/del\n");
        printf("\t4 -> small scale insert duplicate\n");
        printf("\t5 -> large scale insert performance\n");
        printf("\t6 -> large scale get performance\n");
        printf("\t7 -> large scale del performance\n");
        printf("\t8 -> large scale get-intensive performance\n");

        return 0;
    }

    int scenario = atoi(argv[1]);

    switch (scenario) {
        case 0: small_scale_insert_only(); break;
        case 1: small_scale_insert_get(); break;
        case 2: small_scale_insert_del(); break;
        case 3: small_scale_insert_del_get(); break;
        case 4: small_scale_insert_dup(); break;
        case 5: large_scale_insert_only(); break;
        case 6: large_scale_get_only(); break;
        case 7: large_scale_del_only(); break;
        case 8: large_scale_get_intensive_only(); break;
        default: break;
    }

    return 0;
}

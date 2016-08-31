/*
 * Test program
 */
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

struct thread_info {
    pthread_t thread_id;
    int thread_num;
};

enum {INSERT, FIND, DELETE} ht_command;

#define KEY_LEN 11 
//#define VALUE_LEN 64
#define VALUE_LEN 1024
//#define TOTAL_REQUEST 1000000
#define TOTAL_REQUEST 10000

uint64_t total_find;
uint64_t hit_find;
uint64_t total_insert;
uint64_t total_delete;

uint64_t total_find_time;
uint64_t total_insert_time;
uint64_t total_delete_time;

static void *
ht_test(void *arg)
{
    struct timeval start, end;
    uint64_t elapsed_time;
    char key[KEY_LEN]; 
    char value[VALUE_LEN];
    struct thread_info *tinfo = arg;
    printf("Starting HashTable testing thread %d\n", tinfo->thread_num);

    memset(key, 0x0, KEY_LEN);
    memset(value, 0x64, VALUE_LEN);
    value[VALUE_LEN - 1] = 0x0;

    for (int i = 0; i < TOTAL_REQUEST; i++) {
        int cmd = random() % 3;
        int user_id = random() % 9000 + 1000;
        sprintf(key, "USERHT%d", user_id);

        switch(cmd) {
            case INSERT: {
                             gettimeofday(&start, NULL);
                             hashtable_insert(key, value); 
                             gettimeofday(&end, NULL);

                             elapsed_time = end.tv_sec * 1000000 + end.tv_usec -
                                 (start.tv_sec * 1000000 - start.tv_usec);
                             __sync_fetch_and_add(&total_insert_time, 
                                     elapsed_time);

                             __sync_fetch_and_add(&total_insert, 1);

                             break;
                         }
            case FIND: {
                             gettimeofday(&start, NULL);
                             char *get_value = hashtable_find(key);
                             gettimeofday(&end, NULL);

                             elapsed_time = end.tv_sec * 1000000 + end.tv_usec -
                                 (start.tv_sec * 1000000 - start.tv_usec);
                             __sync_fetch_and_add(&total_find_time, 
                                     elapsed_time); 

                             __sync_fetch_and_add(&total_find, 1);

                             if (get_value)
                                 __sync_fetch_and_add(&hit_find, 1);

                             break;
                         }
            case DELETE: {
                             gettimeofday(&start, NULL);
                             hashtable_delete(key);
                             gettimeofday(&end, NULL);

                             elapsed_time = end.tv_sec * 1000000 + end.tv_usec -
                                 (start.tv_sec * 1000000 - start.tv_usec);
                             __sync_fetch_and_add(&total_delete_time, 
                                     elapsed_time); 

                             __sync_fetch_and_add(&total_delete, 1);

                             break;
                         }
            default: break;
        }
    }

}

int 
main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: <%s> <number of threads>\n", argv[0]);
        return 0;
    }

    struct thread_info *tinfo;
    pthread_attr_t attr;

    // Initialize the hash table
    hashtable_init();

    // Start concurrent testing thread
    if (pthread_attr_init(&attr) != 0) {
        perror("pthread_attr_init failed");
        return 0;
    }

    tinfo = calloc(atoi(argv[1]), sizeof(struct thread_info));
    if (tinfo == NULL) {
        perror("calloc failed");
        return 0;
    }

    for (int i = 0; i < atoi(argv[1]); i++) {
        tinfo[i].thread_num = i;

        if (pthread_create(&tinfo[i].thread_id, &attr, &ht_test, &tinfo[i]) 
                != 0) {
            perror("pthread_create failed");
            return 0;
        }
    }

    if (pthread_attr_destroy(&attr) != 0) {
        perror("pthread_attr_destroy failed");
        return 0;
    }

    for (int i = 0; i < atoi(argv[1]); i++) {
        if (pthread_join(tinfo[i].thread_id, NULL) != 0) {
            perror("pthread_join failed");
            return 0;
        }
    }

    free(tinfo);

    hashtable_dump();

    printf("Hashtable execution statistics:\n");
    if (total_insert)
        printf("Hashtable insert time: %lf\n", 
                (total_insert_time + 0.0) / (total_insert + 0.0));
    if (total_delete)
        printf("Hashtable delete time: %lf\n",
                (total_delete_time + 0.0) / (total_delete + 0.0));
    if (total_find)
        printf("Hashtable find time: %lf\n",
                (total_find_time + 0.0) / (total_find + 0.0));
    if (total_find)
        printf("Test case hit rate is %lf\n", 
                (hit_find + 0.0) / (total_find + 0.0));

    return 0;
}

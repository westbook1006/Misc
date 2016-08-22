/*
 * Test program
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

struct thread_info {
    pthread_t thread_id;
    int thread_num;
};

enum {INSERT, SEARCH, DELETE} ht_command;

#define KEY_LEN 9 
#define VALUE_LEN 17
#define THREAD_NUM 2
#define TOTAL_REQUEST 10

static void *
ht_test(void *arg)
{
    char key[KEY_LEN]; 
    char value[VALUE_LEN];
    struct thread_info *tinfo = arg;
    printf("Starting HashTable testing thread %d\n", tinfo->thread_num);

    memset(key, 0x0, KEY_LEN);
    memset(value, 0x0, VALUE_LEN);

    for (int i = 0; i < TOTAL_REQUEST; i++) {
        int cmd = random() % 3;
        int user_id = random() % 90 + 10;
        sprintf(key, "USERHT%d", user_id);

        switch(cmd) {
            case INSERT: {hashtable_insert(key, value); break;}
            case SEARCH: {hashtable_search(key); break;}
            case DELETE: {hashtable_delete(key); break;}
            default: break;
        }
    }
}

int 
main(int argc, char **argv)
{
    struct thread_info *tinfo;
    pthread_attr_t attr;

    // Initialize the hash table
    hashtable_init();

    // Start concurrent testing thread
    if (pthread_attr_init(&attr) != 0) {
        perror("pthread_attr_init failed");
        return 0;
    }

    tinfo = calloc(THREAD_NUM, sizeof(struct thread_info));
    if (tinfo == NULL) {
        perror("calloc failed");
        return 0;
    }

    for (int i = 0; i < THREAD_NUM; i++) {
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

    for (int i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(tinfo[i].thread_id, NULL) != 0) {
            perror("pthread_join failed");
            return 0;
        }
    }

    free(tinfo);

    return 0;
}

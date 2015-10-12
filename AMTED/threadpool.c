#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

typedef struct _work_t{
	void (*routine) (void*);
	void *arg;
	struct _work_t *next;
} work_t;

typedef struct _threadpool_st{
	int num_threads;
	int qsize;
	pthread_t *threads;
	work_t* qhead;
	work_t* qtail;
	pthread_mutex_t qlock;
	pthread_cond_t q_not_empty;
	pthread_cond_t q_empty;
	int shutdown;
	int dont_accept;
}_threadpool;

static void* do_work(threadpool p)
{
	_threadpool *pool = (_threadpool *)p;
	work_t* cur;

	printf("One thread has been created!\n");
	while (1) {
		pthread_mutex_lock(&(pool->qlock));
		while (pool->qsize == 0) {
			if (pool->shutdown) {
				pthread_mutex_unlock(&(pool->qlock));
				pthread_exit(NULL);
			}

			pthread_cond_wait(&(pool->q_not_empty), &(pool->qlock));

			if (pool->shutdown) {
				pthread_mutex_unlock(&(pool->qlock));
				pthread_exit(NULL);
			}
		}

		cur = pool->qhead;
		pool->qsize--;
		if (pool->qsize == 0) {
			pool->qhead = NULL;
			pool->qtail = NULL;
		} else
			pool->qhead = cur->next;

		if ((pool->qsize == 0) && (!pool->shutdown))
			pthread_cond_signal(&(pool->q_empty));
		pthread_mutex_unlock(&(pool->qlock));

		(cur->routine)(cur->arg);
		free(cur);
	}
}

threadpool create_threadpool(int num_threads_in_pool)
{
	_threadpool *pool;
	int i;

	if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
		return NULL;

	pool = (_threadpool *)malloc(sizeof(_threadpool));
	if (!pool) {
		perror("malloc: create a thread pool");
		return NULL;
	}

	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads_in_pool);
	if (!pool->threads) {
		perror("malloc: create threads");
		return NULL;
	}
	pool->num_threads = num_threads_in_pool;
	pool->qsize = 0;
	pool->qhead = NULL;
	pool->qtail = NULL;
	pool->shutdown = 0;
	pool->dont_accept = 0;

	if (pthread_mutex_init(&(pool->qlock), NULL)) {
		perror("pthread_mutex_init");
		return NULL;
	}

	if (pthread_cond_init(&(pool->q_empty), NULL)) {
		perror("pthread_cond_init");
		return NULL;
	}

	if (pthread_cond_init(&(pool->q_not_empty), NULL)) {
		perror("pthread_cond_init");
		return NULL;
	}

	for (i = 0; i < num_threads_in_pool; i++) {
		if(pthread_create(&(pool->threads[i]), NULL, do_work, pool)) {
			perror("pthread_create");
			return NULL;
		}
	}

	printf("Thread pool create successfully!\n");

	return (threadpool)pool;
}

void dispatch(threadpool th_pool, dispatch_fn fn_dispatch, void *arg)
{
	_threadpool *pool = (_threadpool *)th_pool;
	work_t *cur;

	printf("Add a work\n");

	cur = (work_t*)malloc(sizeof(work_t));
	if (!cur)
		perror("malloc: create work_t");

	cur->routine = fn_dispatch;
	cur->arg = arg;
	cur->next = NULL;

	pthread_mutex_lock(&(pool->qlock));

	if (pool->dont_accept) {
		free(cur);
		return;
	}
	if (pool->qsize == 0) {
		pool->qhead = cur;
		pool->qtail = cur;
		pthread_cond_signal(&(pool->q_not_empty));
	} else {
		pool->qtail->next = cur;
		pool->qtail = cur;
	}
	pool->qsize++;
	pthread_mutex_unlock(&(pool->qlock));
}

void destroy_threadpool(threadpool th_pool)
{
	_threadpool *pool = (_threadpool *)th_pool;
	void *nothing;
	int i;

	pthread_mutex_lock(&(pool->qlock));
	pool->dont_accept = 1;
	printf("Test1\n");
	while (pool->qsize) {
		pthread_cond_wait(&(pool->q_empty), &(pool->qlock));
	}
	printf("Test3\n");
	pool->shutdown = 1;
	pthread_cond_broadcast(&(pool->q_not_empty));
	pthread_mutex_unlock(&(pool->qlock));
	
	printf("Test2\n");
	for (i = 0; i < pool->num_threads; i++)
		pthread_join(pool->threads[i], &nothing);
	
	free(pool->threads);
	pthread_mutex_destroy(&(pool->qlock));
	pthread_cond_destroy(&(pool->q_empty));
	pthread_cond_destroy(&(pool->q_not_empty));
	printf("Destory the threadpool\n");
}

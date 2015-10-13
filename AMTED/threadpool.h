#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "amted_server.h"

#define MAXT_IN_POOL 16

typedef void *threadpool;
typedef void (*dispatch_fn)(node*);

threadpool create_threadpool(int num_threads_in_pool);
void dispatch(threadpool th_pool, dispatch_fn fn_dispatch, node *arg);
void destroy_threadpool(threadpool th_pool);

#endif

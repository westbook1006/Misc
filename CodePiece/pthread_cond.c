/*
 * This code piece is from the thread.c in memcached.
 */
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

static void register_thread_initialized(void) {
    //...
    //...

    pthread_mutex_lock(&init_lock);
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

    //...
    //...
}

static void wait_for_thread_registration(int nthreads) {
    while (init_count < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
}

void memcached_thread_init(int nthreads, struct event_base *main_base) {

    //...
    //...

    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

    //...
    //...

    pthread_mutex_lock(&init_lock);
    wait_for_thread_registration(nthreads);
    pthread_mutex_unlock(&init_lock);
}

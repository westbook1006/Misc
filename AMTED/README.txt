/*
** Description: A simple asymmetric multi-threaded event driven file server (AMTED).
** Author: Ming Liu
** Linux APIs usage:
** (1) fcntl --> set a socket to non-blocking mode
** (2) socket, bind, listen, accept --> create a TCP connection socket, bind to an 
** address, listen to a new connection, accept a new connection from a client
** (3) read, write, close --> read from a file handler, write to a file handler,
** close a file hander
** (4) epoll_create, epoll_ctl, epoll_wait --> epoll mechanism to monitor a set of file descriptors
** (5) pthread_create, pthread_exit, pthread_join, pthread_mutex_init,  pthread_mutex_lock, 
** pthread_mutex_unlock, pthread_cond_init, pthread_cond_wait, pthread_cond_signal, 
** pthread_cond_broadcast, pthread_mutex_destroy,pthread_cond_destroy --> pthread management
** (6) access --> check the file existence
*/

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "amted_server.h"
#include "threadpool.h"

static int setNonBlocking(int sock)
{
	int status;
	if((status = fcntl(sock, F_GETFL)) < 0) {
		perror("fcntl get");
		return -1;
	}
	if (fcntl(sock, F_SETFL, status | O_NONBLOCK) < 0) {
		perror("fcntl set");
		return -1;
	}
	
	return 1;
}

int check_exist(char *path) {
	int str_len = strlen(path);
	path[str_len - 1] = '\0';

	if (access(path, F_OK))
		return 0;

	return 1;
}
/*static void readFile()
{
	printf("Hello world!\n");
}*/

int main(int argc, char **argv)
{
	int listenfd = 0, connfd = 0, epfd, i, nfds, sockfd, n;
	struct sockaddr_in serv_addr;
	char *recvBuff;
	struct epoll_event ev, events[EPOLL_NUM];
	threadpool pool;

	if (argc != 3) {
		printf("Usage: <%s> <IPv4 address> <port number>\n", argv[0]);
		return 0;
	}

	//Create a listening socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0x00, sizeof(serv_addr));
	recvBuff = (char *)malloc(PATH_LENGTH * sizeof(char));
	memset(recvBuff, 0x00, PATH_LENGTH * sizeof(char));

	//Setup a listening socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);

	//Bind and listen	
	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return 0;
	}
	if (listen(listenfd, MAX_CLIENT_NUM) < 0) {
		perror("listen");
		return 0;
	}
	//Set non-blocking
	if (setNonBlocking(listenfd) < 0)
		return 0;

	//Add to epoll
	if ((epfd = epoll_create(EPOLL_NUM)) < 0) {
		perror("epoll_create");
		return 0;
	}
	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
		perror("epoll_ctl");
		return 0;
	}

	//Create work thread pool
	//pool = create_threadpool(MAXT_IN_POOL);
	//for (i = 0; i < 10; i++)
	//	dispatch(pool, readFile,(void *)NULL);
	//destroy_threadpool(pool);

	for (;;) {
		if ((nfds = epoll_wait(epfd, events, EPOLL_NUM, -1)) < 0) {
			perror("epoll_wait");
			return 0;
		}

		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == listenfd) { // New connection
				if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0) {
					perror("accept");
					return 0;
				}
				if (setNonBlocking(connfd) < 0)
					return 0;

				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
					perror("epoll_ctl");
					return 0;
				}
			} else if (events[i].events & EPOLLIN) { // Read
				if ((sockfd = events[i].data.fd) < 0)
					continue;

				if ((n = read(sockfd, recvBuff, PATH_LENGTH)) < 0) {
					perror("read");
					close(sockfd);
				} else if (n == 0) {
					close(sockfd);
					events[i].data.fd = -1;
				} else {
					if (check_exist(recvBuff)) {
						printf("Exist\n");
					} else {
						printf("The file %s doesn't exist!\n", recvBuff);
						continue;
					}
				}

				ev.data.fd = sockfd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
			} else if (events[i].events & EPOLLOUT) { // Write
				if ((sockfd = events[i].data.fd) < 0)
					continue;

				write(sockfd, "TEST\n", 5);
				ev.data.fd = sockfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
			} else
				;
		}
	}
	return 0;
}

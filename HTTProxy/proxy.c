#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#define MAX_CLIENT_NUM 32
#define BUFFER_LEN 4096
#define STR_LEN 64

int main(int argc, char *argv[])
{
	int listenfd, connfd, i, j, len, sockfd;
	struct sockaddr_in serv_addr, client_addr;
	char recv_buff[BUFFER_LEN], parser[BUFFER_LEN], send_buff[BUFFER_LEN];
	char cmd[STR_LEN], op1[STR_LEN], op2[STR_LEN], hostname[STR_LEN];
	struct hostent *host;

	if (argc != 2) {
		printf("Usage: %s <port number>\n", argv[0]);
		return 0;
	}
	printf("Welcome to a HTTP proxy server!\n");

	//Create a listenting socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0x00, sizeof(serv_addr));

	//Setup a listening socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	//Bind and listen
	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return 0;
	}
	if (listen(listenfd, MAX_CLIENT_NUM) < 0) {
		perror("listen");
		return 0;
	}

	//Accept
	if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0) {
		perror("accept");
		return 0;
	}

	//Read the header
	memset(recv_buff, 0x00, BUFFER_LEN);
	memset(parser, 0x00, BUFFER_LEN);
	memset(send_buff, 0x00, BUFFER_LEN);
	memset(cmd, 0x00, STR_LEN);
	memset(op1, 0x00, STR_LEN);
	memset(op2, 0x00, STR_LEN);
	if (read(connfd, recv_buff, BUFFER_LEN - 1) < 0) {
		perror("read");
		return 0;
	}

	printf("%s", recv_buff);
	//Parser the header
	len = 0;
	i = 0;
	while (recv_buff[i] != '\0') {
		for (j = i; (recv_buff[i] != '\n') && (recv_buff[i] != '\0'); i++)
			parser[i - j] = recv_buff[i];
		parser[i - j] = '\0';
		if (recv_buff[i] == '\n') {
			i++;
			//printf("%s\n", parser);
		}
		if (recv_buff[i] == '\0')
			break;

		sscanf(parser, "%s ", cmd);
		//printf("The first --> %s\n", cmd);
		if (!strcmp(cmd, "GET")) {
			sscanf(parser + strlen(cmd) + 1, "%s ", op1);
			sscanf(parser + strlen(cmd) + strlen(op1) + 2, "%s ", op2);

			sprintf(send_buff + len, "%s ", cmd);
			len += strlen(cmd) + 1;
			sprintf(send_buff + len, "%s ", op1);
			len += strlen(op1) + 1;
			sprintf(send_buff + len, "HTTP/1.0\n");
			len += strlen("HTTP/1.0\n");
			//sprintf(send_buff + len, "%s\n", op2);
			//len += strlen(op2) + 1;

			//printf("The first --> %s\n", cmd);
			//printf("\tThe second --> %s\n", op1);
			//printf("\tThe third --> %s\n", op2);
		}
		else if (!strcmp(cmd, "Host:")) {
			sscanf(parser + strlen(cmd) + 1, "%s ", op1);
			sscanf(parser + strlen(cmd) + 1, "%s ", hostname);

			sprintf(send_buff + len, "%s ", cmd);
			len += strlen(cmd) + 1;
			sprintf(send_buff + len, "%s\n", op1);
			len += strlen(op1) + 1;

			//Create the connection

			//printf("The first --> %s\n", cmd);
			//printf("\tThe second --> %s\n", op1);
		}
		else if (!strcmp(cmd, "Connection:")) {
			sscanf(parser + strlen(cmd) + 1, "%s ", op1);

			sprintf(send_buff + len, "%s ", cmd);
			len += strlen(cmd) + 1;
			sprintf(send_buff + len, "close\n");
			len += strlen("close\n");
			//sprintf(send_buff + len, "%s\n", op1);
			//len += strlen(op1) + 1;

			//printf("The first --> %s\n", cmd);
			//printf("\tThe second --> %s\n", op1);
		} else {
			sprintf(send_buff + len, "%s\n", parser);
			len += strlen(parser) + 1;
		}
	}

	printf("******************************************\n");

	printf("%s", send_buff);

	printf("******************************************\n");
	host = gethostbyname(hostname);
	client_addr.sin_addr = *(struct in_addr*)host->h_addr_list[0];
	client_addr.sin_family = host->h_addrtype;
	client_addr.sin_port = htons(80);

	//Create a listenting socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&client_addr, 0x00, sizeof(client_addr));

	//Connect
	if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
		perror("connect");
		return 0;
	}
	printf("Hostname: %s/naddress list:", host->h_name);
	/*for (i = 0; host->h_addr_list[i]; i++) {
		printf("%s/t", inet_ntoa(*(struct in_addr *)(host->h_addr_list[i])));
	}
	printf("/n");*/
	//printf("Hostname: %s\n", hostname);
	//printf("IP: %s\n", host->h_addr_list[0]);
	printf("******************************************\n");
	printf("Send to the server\n");
	//Write
	if (write(sockfd, recv_buff, BUFFER_LEN - 1) < 0) {
		perror("write");
		return 0;
	}

	printf("******************************************\n");
	printf("Read from the server and send to the web browser\n");
	//Read and send to the web browser
	while (read(sockfd, recv_buff, BUFFER_LEN -1) > 0) {
		if (write(connfd, recv_buff, BUFFER_LEN -1) < 0) {
			perror("write");
			return 0;
		}
	}

	//Close the socket
	close(connfd);
	close(sockfd);

	return 0;
}

#ifndef SERVER_TOOLS_H_
#define SERVER_TOOLS_H_
#define MSG_MAXLEN 30
#define NB_MAX_CLIENT 2

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include "../general_tools.h"

typedef struct thread_arg {
  int thread_fd_connection;
  int thread_sock;
} thread_arg;

void init_serv_addr(struct sockaddr_in *serv_addr, int port);
void do_bind(int socket, struct sockaddr_in addr_in);
void do_listen(int socket, int nb_max);
int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);
//void *connection_handler(void* sock);
void *connection_handler(void* fd_connection);


#endif

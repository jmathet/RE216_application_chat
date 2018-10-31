#ifndef CLIENT_TOOLS_H_
#define CLIENT_TOOLS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../general_tools.h"

typedef struct reception_arg {
    int sock;
    int status;
    pthread_mutex_t sock_mutex;
} reception_arg;

typedef struct communication_arg {
    int sock;
    int status;
    pthread_mutex_t sock_mutex;
} communication_arg;

/* Modify specified sockaddr_in for the client side with specified port and IP
 * Usage : init_client_addr(pointer_on_already_allocated_struct, ip, port); */
void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port);

/* Connect to the server through the socket with a sockaddr_in struct
 * Usage : do_connect(socket, host) */
void do_connect(int sock, struct sockaddr_in host_addr);

/* Ask the user to enter a pseudo and send in to the server
 * Usage : auth_user(socket); */
void auth_user(int sock);

/* Check is the pseudo does not start by a space and has a non-null length
 * Usage : is_pseudo_correct(pseudo); -> return 1 if true 0 if false (pseudo-boolean) */
int is_pseudo_correct(char * pseudo);

/* Thread handler for receiving messages and display them on the stdout */
void * reception_handler(void * arg);

/* Thread handler for sending messages given by the users through the socket */
void * communication_handler(void * arg);

#endif

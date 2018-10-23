#ifndef CLIENT_TOOLS_H_
#define CLIENT_TOOLS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "../general_tools.h"

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port);
void do_connect(int sock, struct sockaddr_in host_addr);
int is_pseudo_correct(char * pseudo);

#endif

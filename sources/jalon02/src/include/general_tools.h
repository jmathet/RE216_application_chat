#ifndef GENERAL_TOOLS_H_
#define GENERAL_TOOLS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MSG_MAXLEN 30

void error(const char *msg);
int do_socket();
ssize_t readline(int file_des, void *str, size_t maxlen);
void sendline(int file_des, const void *str, size_t maxlen);

#endif

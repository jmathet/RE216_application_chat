#ifndef GENERAL_TOOLS_H_
#define GENERAL_TOOLS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MSG_MAXLEN 1000

enum SERVER_STATUS {
    SERVER_RUNNING,
    SERVER_QUITTING,
    SERVER_FULL
};

enum CLIENT_STATUS {
    CLIENT_NOT_LOGGED,
    CLIENT_LOGGED,
    CLIENT_QUITTING
};

void error(const char *msg);
int do_socket();
void send_int(int file_des, int to_send);
int read_int(int file_des);
void read_line(int file_des, void *str);
void send_line(int file_des, const void *str);
void string_strip(char *string);
#endif

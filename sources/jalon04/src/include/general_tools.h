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
    CLIENT_RUNNING,
    CLIENT_QUITTING
};

enum FUNCTIONS {
    FUNC_UNDEFINED,
    FUNC_QUIT,
    FUNC_NICK,
    FUNC_WHO,
    FUNC_WHOIS
};

/* Interrupt program because of an error
 * Usage : error("error message"); */
void error(const char *msg);

/* Create a socket and return the associated file descriptor
 * Usage : socket = do_sock(); */
int do_socket();

/* Send an integer into the socket, with conversion to int32_t for cross-compatibility
 * Usage : send_int(socket, 1); */
void send_int(int file_des, int to_send);

/* Read an integer from the socket, with conversion to int32_t for cross-compatibility
 * Usage : result = read_int(socket); */
int read_int(int file_des);

/* Read a line with length detection (pseudo-protocol)
 * Usage : read_line(socket, buffer_to_write_in); */
void read_line(int file_des, void *str);

/* Sending a line with length-detection (pseudo-protocol)
 * Usage : send_line(socket, buffer_to_send); */
void send_line(int file_des, const void *str);

/* Return function number from the given message
 * Usage : function_number = parser(message); */
int parser(char * message);

#endif

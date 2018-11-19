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
#define NB_MAX_CLIENT 5

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
    FUNC_WHOIS,
    FUNC_MSG,
    FUNC_MSGALL,
    FUNC_CHANNEL_CREATE,
    FUNC_CHANNEL_JOIN,
    FUNC_CHANNEL_LIST,
    FUNC_CHANNEL_QUIT
};

typedef struct message {
  char * source_pseudo;
  char * source;
  char * text;
} message ;

struct users{
    int id;
    int associated_fd;
    pthread_mutex_t communication_mutex;
    char *pseudo;
    char *IP_addr;
    unsigned short port;
    char * connection_date;
    int channel_id;
    struct users* next;
};

typedef struct channel {
    int id;
    char* name;
    int members[NB_MAX_CLIENT];
    int nb_users_inside;
    struct channel* next;
}channel;


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

/* Initialize a message structure with a full malloc & memset on all strings. */
message * init_message();

/* Free completly a message structure */
void free_message(message * message);

/* Perform a memset on message struct buffers */
void flush_message(message * message);

/* Send all the fields of a message structure with send_line function (with fiability and length check) */
void send_message(int file_des, char *source_pseudo, const void *text, char *source);

/* Receive content from send_message fonction, create and return a filled message structure */
message * receive_message(int file_des);

/* Return function number from the given message
 * Usage : function_number = parser(message); */
int parser(char * message);

/* Remove \n first character from the string by cuting it earlier with an '\0'
 * Usage : remove_line_breaks(string); (modifying pointer) */
void remove_line_breaks(char * string);

#endif

#ifndef SERVER_TOOLS_H_
#define SERVER_TOOLS_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "../general_tools.h"
#include "server_tools_channels.h"

typedef struct thread_arg {
  int connection_fd;
  int user_id;
  struct users * users_list;
  struct channel * channel_list;
  char * client_IP;
  int client_port;
  volatile int * pt_nb_conn;
  volatile int * pt_status;
} thread_arg;

/* Thread hanlder when accepting a new incoming connection from a client */
void *connection_handler(void* sock);

/* Duplicate struct threads_arg content between two allocated pointers */
void duplicate_threads_args(thread_arg * source_args, thread_arg * dest_args);

/* Given a pointer on first arg of the command message, the function will duplicate the argument
 * inside command_arg and the remaining text inside command_text. Those those pointers will be
 * allocated inside the function and need to be free after use in the program. */
void extract_command_args(char *message_pointer, char **pt_command_arg, char **pt_command_text);

/* Send a personnal message to an user identified by its id.
 * The dest_id MUST exist and be checked before. */
void send_message_to_user(struct users *users, int dest_id, char *text, char *source_pseudo, char *source, short mode);

#endif

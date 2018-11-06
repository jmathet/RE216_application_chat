#ifndef SERVER_TOOLS_H_
#define SERVER_TOOLS_H_

#define NB_MAX_CLIENT 2

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

struct users{
  int id;
  int associated_fd;
  pthread_mutex_t communication_mutex;
  char *pseudo;
  char *IP_addr;
  unsigned short port;
  char * connection_date;
  struct users* next;
};

typedef struct thread_arg {
  int connection_fd;
  int user_id;
  struct users * users_list;
  char * client_IP;
  int client_port;
  volatile int * pt_nb_conn;
  volatile int * pt_status;
} thread_arg;

typedef struct channel {
    int id;
    char* name;
    int members[NB_MAX_CLIENT];
    int nb_users_inside;
    struct channel* next;
}channel;

/* Modify specified sockaddr_in for the server side with specified port */
void init_serv_addr(struct sockaddr_in *serv_addr, int port);

/* Perform a bind on specified socket */
void do_bind(int socket, struct sockaddr_in addr_in);

/* Switch specified socket in the listen state */
void do_listen(int socket, int nb_max);

/* Accept a connection with the specified socket and return the file des from accepted socket*/
int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);

/* Thread hanlder when accepting a new incoming connection from a client */
void *connection_handler(void* sock);

/* Duplicate struct threads_arg content between two allocated pointers */
void duplicate_threads_args(thread_arg * source_args, thread_arg * dest_args);

/* Given a pointer on first arg of the command message, the function will duplicate the argument
 * inside command_arg and the remaining text inside command_text. Those those pointers will be
 * allocated inside the function and need to be free after use in the program. */
void extract_command_args(char *message_pointer, char **pt_command_arg, char **pt_command_text);

/* Add an user at to the list of ALREADY EXISTING users (at least system user). Users is added by the right side. */
void users_add_user(struct users * list, int user_id, int thread_fd, char* pseudo, char* IP_addr, unsigned short port, char* date);

/* Remove an user from the users list based on his ID. Not applicable for system user */
void users_delete_user(struct users * list, int user_id_to_delete);

/* Return the pseudo (char *) of the given user id. The user must exist. */
char * users_get_user_pseudo(struct users * users, int user_id);

/* Set the pseudo of the user corresponding to the id */
void user_set_pseudo(struct users * users, int user_id, char * message);

/* Return the list of connected users to the server into pseudo_list pointer */
void users_get_pseudo_display(struct users *users, char *pseudo_list);

/* Return pointer to the user identified by an ID (id-based search)
 * The user MUST exist. */
struct users *users_get_user(struct users *users_list, int id);

char *users_get_info_user(struct users * users, char *message);

/* Return the id of an user by its pseudo (pseudo-based search) */
int users_get_id_by_pseudo(struct users *users, char *pseudo);

/* Send a personnal message to an user identified by its id.
 * The dest_id MUST exist and be checked before. */
void send_message_to_user(struct users *users, int dest_id, char *message);

#endif

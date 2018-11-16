#ifndef SERVER_TOOLS_USERS_H_
#define SERVER_TOOLS_USERS_H_

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



/* Add an user at to the list of ALREADY EXISTING users (at least system user). Users is added by the right side. */
void users_add_user(struct users * list, int user_id, int thread_fd, char* pseudo, char* IP_addr, unsigned short port, char* date);

/* Remove an user from the users list based on his ID. Not applicable for system user */
void users_delete_user(struct users * list, int user_id_to_delete);

/* Return 0 if the pseudo is found. */
int users_find_name(struct users *users_list, char *pseudo);

/* Return the pseudo (char *) of the given user id. The user must exist. */
char * users_get_user_pseudo(struct users * users, int user_id);

int users_get_user_channel_id(struct users *users, int user_id);

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

/* Set the channel in the user corresponding to the id */
void user_set_channel(struct users *users_list, int user_id, int channel_id);

#endif
#ifndef SERVER_TOOLS_CHANNELS_H_
#define SERVER_TOOLS_CHANNELS_H_

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
#include "server_tools_users.h"


/* Add an channel at to the list of ALREADY EXISTING channels (at least system channel). Users is added by the right side. */
void channels_add_channel(struct channel *channel_list, char *message);

/* Add the user_id to the members listof the channel given */
void channel_add_user_to_members(struct channel *channel, int user_id);

/* Add a user to the channel given in the message */
void channel_add_user(struct channel * channels_list, struct users* users_list, int user_id, char *message);

/* Return the channel corresponding to the channel_id */
struct channel *channels_get_channel(struct channel* channels, int channel_id);

/* Remove a user from his channal based on his user_id */
void channel_delete_user(struct channel* channels_list,struct users* users_list, int user_id, char * message);

/* Remove the channel from the channels list based on his id */
void channels_delete_channel(struct channel * channels_list, int channel_id);

#endif
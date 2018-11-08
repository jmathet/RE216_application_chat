#include "server_tools_channels.h"

struct channel* channel_init(char *name, int id){
  struct channel* new_channel;
  new_channel = malloc(sizeof(struct channel));
  new_channel->name = malloc(strlen(name));
  strcpy(new_channel->name, name);
  new_channel->id = id;

  for (int i = 0; i < NB_MAX_CLIENT ; i++) { // TODO remplacer par un memset
    new_channel->members[i] = 0;
  }
  new_channel->nb_users_inside = 0;
  new_channel->next = NULL;

  return new_channel;
}

int channels_find_name(struct channel *channel_list, char *name){
  struct channel * temp = channel_list->next;
  while (temp != NULL){
    if (strcmp(temp->name,name)==0)
      return 0;
    temp = temp->next;
  }
  return 1;
}

void channels_add_channel(struct channel *channel_list, char *message){
  /* INITS */
  char channel_name[MSG_MAXLEN];
  struct channel * new_channel;

  /* GET CHANNEL NAME */
  remove_line_breaks(message);
  strcpy(channel_name, message+strlen("/create "));

  /* CHECK IF NOT ALREADY USED */
  if (channels_find_name(channel_list, message+strlen("/create "))==0) {
    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "The channel name '%s' is already used ! Error during channel creation.\n", channel_name);
  }
  else { // name available
    // find the last channel struct in the channels list
    while (channel_list->next!=NULL)
      channel_list = channel_list->next;

    // filling the structure
    new_channel = channel_init(channel_name, channel_list->id +1);

    channel_list->next = new_channel; // linking the new user

    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "Channel %s created !\n", new_channel->name);
  }
}


void channel_add_user_to_members(struct channel *channel, int user_id){
  int i = 0;
  while (channel->members[i] != 0){
    i++;
  }
  channel->members[i] = user_id;
}

void channel_add_user(struct channel * channels_list, struct users* users_list, int user_id, char *message){
  struct channel *channel_temp = channels_list->next;
  remove_line_breaks(message);
  if (users_get_user_channel_id(users_list, user_id)==0) {
    // find
    while (channel_temp != NULL && strcmp(channel_temp->name, message + strlen("/join ")) != 0)
      channel_temp = channel_temp->next;


    memset(message, 0, MSG_MAXLEN);
    if (channel_temp == NULL)
      strcpy(message, "The channel does not exist.\n");
    else {
      ++(channel_temp->nb_users_inside);
      channel_add_user_to_members(channel_temp, user_id);
      user_set_channel(users_list, user_id, channel_temp->id);
      strcpy(message, "You have joined the channel !\n");
    }
  }
  else {
    memset(message, 0, MSG_MAXLEN);
    strcpy(message, "You are already in a channel.\n");
  }
}

struct channel *channels_get_channel(struct channel* channels, int channel_id){
  while(channels!=NULL && channels->id!=channel_id){
    channels = channels->next;
  }
  return channels;
}

void channel_delete_user(struct channel* channels_list,struct users* users_list, int user_id, char * message){
  struct users *temp_user = users_get_user(users_list, user_id);
  struct channel * channel_temp = channels_get_channel(channels_list, temp_user->channel_id);
  temp_user->channel_id = 0;
  // Delete the user from the members list
  for (int i = 0; i < NB_MAX_CLIENT ; i++) {
    if (channel_temp->members[i]==user_id)
      channel_temp->members[i]=0;
  }
  // Decrease the members counter
  channel_temp->nb_users_inside--;
  // Check if this user is the last one
  if (channel_temp->nb_users_inside==0){
    channels_delete_channel(channels_list, channel_temp->id);
    // TODO message au dernier occupant (le salon a été détruit)
  }
}

void channels_delete_channel(struct channel * list, int channel_id_to_delete){
  while (list->next!=NULL) {
    // check next user
    if (channel_id_to_delete==list->next->id) {
      struct channel * temp = list->next;
      // unlink user
      list->next=list->next->next;
      free(temp->name);
      free(temp);
      // end user research
      break;
    }
    else
      list=list->next;
  }
}


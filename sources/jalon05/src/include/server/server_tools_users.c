#include "server_tools_users.h"

void users_add_user(struct users * list, int user_id, int thread_fd, char* pseudo, char* IP_addr, unsigned short port, char * date){
  struct users * new_user = malloc(sizeof(struct users));
  // filling user structure
  new_user->id = user_id;
  new_user->associated_fd = thread_fd;
  new_user->IP_addr = IP_addr;
  new_user->port = port;
  new_user->connection_date = date;
  new_user->next = NULL;
  new_user->pseudo = malloc(sizeof(char) * MSG_MAXLEN);
  new_user->channel_id = 0;

  // pseudo filling
  strcpy(new_user->pseudo, pseudo);

  // mutex init
  pthread_mutex_init(&new_user->communication_mutex, NULL);

  // finding the last user user of the list
  struct users *temp;
  temp=list;
  while (temp->next!=NULL) {
    temp=temp->next;
  }
  // linking the new user
  temp->next=new_user;
}



char * users_get_user_pseudo(struct users * users, int user_id){
  while (users->id!=user_id) {
    users = users->next;
  }
  return users->pseudo;
}

int users_get_user_channel_id(struct users *users, int user_id){
  while (users->id!=user_id) {
    users = users->next;
  }
  return users->channel_id;
}

void user_set_pseudo(struct users * users, int user_id, char * message){
  // TODO : check if the pseudo is not already used
  while (users->id!=user_id) {
    users = users->next;
  }
  remove_line_breaks(message);
  if (strcmp(users->pseudo, "Guest")==0) { //initial pseudo
    strcpy(users->pseudo, message + strlen("/nick "));
    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "Welcome in the chat %s !\n", users->pseudo);
  }
  else { //change of pseudo
    strcpy(users->pseudo, message + strlen("/nick "));
    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "Your pseudo has been changed to %s.\n", users->pseudo);
  }
}

void users_get_pseudo_display(struct users *users, char *pseudo_list) {
  strcpy(pseudo_list, "\nOnline users are : \n");
  while (users!=NULL) {
    if (users->id!=0) {
      strcat(pseudo_list, "\t -");
      strcat(pseudo_list, users->pseudo);
      strcat(pseudo_list, "\n");
    }
    users = users->next;
  }
}

struct users *users_get_user(struct users *users_list, int id) {
  while(users_list->id != id)
    users_list = users_list->next;
  return users_list;
}

char *users_get_info_user(struct users * users, char *message){
  /* Return information of the corresponding user */
  remove_line_breaks(message);
  while (users!=NULL && strcmp(users->pseudo, message + strlen("/whois ")*sizeof(char))!=0) {
    users = users->next;
  }
  memset(message, 0, MSG_MAXLEN);
  if (users==NULL) {
    sprintf(message, "No user found !");
  }
  else {
    sprintf(message, "%s connected since %s with the IP address %s and port number %d.\n", users->pseudo, users->connection_date, users->IP_addr, users->port);
  }
  return message;
}

int users_get_id_by_pseudo(struct users *users, char *pseudo) {
  while(users != NULL && strcmp(users->pseudo, pseudo) != 0) {
    users = users->next;
  }
  return users != NULL ? users->id : 0; // return the id of the user or 0 if not found
}

void user_set_channel(struct users *users_list, int user_id, int channel_id){
  while (users_list!=NULL && users_list->id!=user_id){
    users_list=users_list->next;
  }
  users_list->channel_id = channel_id;
}


void users_delete_user(struct users * list, int user_id_to_delete){
  while (list->next!=NULL) {
    // check next user
    if (user_id_to_delete==list->next->id) {
      struct users * temp = list->next;
      // unlink user
      list->next=list->next->next;
      free(temp->pseudo);
      free(temp->connection_date);
      free(temp->IP_addr);
      free(temp);
      // end user research
      break;
    }
    else
      list=list->next;
  }
}
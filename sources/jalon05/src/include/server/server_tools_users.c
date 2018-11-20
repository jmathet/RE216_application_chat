#include "server_tools_users.h"

void users_add_user(struct users * list, int user_id, int thread_fd, char* pseudo, char* IP_addr, unsigned short port, char * date){
  struct users * new_user = malloc(sizeof(struct users));
  // filling user structure
  new_user->id = user_id;
  new_user->associated_fd = thread_fd;
  new_user->IP_addr = malloc(sizeof(char)*10);
  new_user->port = port;
  new_user->connection_date = date;
  new_user->next = NULL;
  new_user->pseudo = malloc(sizeof(char) * MSG_MAXLEN);
  new_user->channel_id = 0;
  new_user->receiving_file_from = 0;

  // pseudo filling
  strcpy(new_user->pseudo, pseudo);

  // IP_addr filling
  strcpy(new_user->IP_addr, IP_addr);

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

int users_find_name(struct users *users_list, char *pseudo){
  struct users * temp = users_list->next;
  while (temp != NULL){
    if (strcmp(temp->pseudo,pseudo)==0)
      return 0;
    temp = temp->next;
  }
  return 1;
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
  remove_line_breaks(message);
  char * pseudo = malloc(sizeof(message)-strlen("/nick "));
  strcpy(pseudo, message + strlen("/nick "));
  if (users_find_name(users, pseudo)==0){
    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "The pseudo %s is already used !\n", pseudo);
  }
  else if (strcmp(pseudo, "Server")==0 || strcmp(pseudo, "System")==0 || strcmp(pseudo, "me")==0){
    memset(message, 0, MSG_MAXLEN);
    sprintf(message, "The pseudo %s can not be used !\n", pseudo);
  }
  else {
    while (users->id != user_id)
      users = users->next;
    if (strcmp(users->pseudo, "Guest") == 0) { //initial pseudo
      strcpy(users->pseudo, pseudo);
      memset(message, 0, MSG_MAXLEN);
      sprintf(message, "OK Welcome in the chat %s !\n", users->pseudo);
    } else { //change of pseudo
      strcpy(users->pseudo, pseudo);
      memset(message, 0, MSG_MAXLEN);
      sprintf(message, "OK Your pseudo has been changed to %s.\n", users->pseudo);
    }
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
      free(temp->IP_addr);
      free(temp);
      // end user research
      break;
    }
    else
      list=list->next;
  }
}
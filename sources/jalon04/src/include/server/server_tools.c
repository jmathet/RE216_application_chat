#include "server_tools.h"
// TODO séparer server_tools_users et server_tools_channels

void init_serv_addr(struct sockaddr_in *serv_addr, int port)
 {
   // clean structure
   memset(serv_addr, 0, sizeof(*serv_addr));
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = htons(port); // specified port in args
   serv_addr->sin_addr.s_addr = INADDR_ANY;
 }

 void do_bind(int socket, struct sockaddr_in addr_in)
 {
   int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
   if (-1 == bind_result)
     error("bind");
 }

 void do_listen(int socket, int nb_max)
 {
   int listen_result = listen(socket, nb_max);
   if (-1 == listen_result)
     error("listen");
 }

 int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
 {
   int file_des_new = accept(socket, addr, addrlen);
   if(-1 == file_des_new)
     error("accept");
   return file_des_new;
 }

void *connection_handler(void* thread_input)
{
  /* INITS */
  int client_status = CLIENT_RUNNING;
  int dest_id;
  struct message * message;
  thread_arg * thread_args;
  struct users * current_user;
  struct users * temp_user;

  /* MALLOC */
  thread_args = malloc(sizeof(thread_arg));
  message = init_message();

  /* THREAD ARGS GESTION (thread_args independant from thread_input of main) */
  duplicate_threads_args((thread_arg *)thread_input, thread_args);
  free(thread_input); // allocated in the main

  /* INCREASE MAIN NB_CONNECTIONS */
  ++ *(thread_args->pt_nb_conn); // increase number of nb_total_connections
  printf("![Server] : User %i connected.\n", thread_args->user_id);

  /* CREATING THE USER INTO THE USERS TABLE */
  time_t mytime;
  mytime = time(NULL);
  users_add_user(thread_args->users_list,
          thread_args->user_id,
          thread_args->connection_fd,
          "Guest",
          thread_args->client_IP,
          (unsigned short)thread_args->client_port,
          ctime(&mytime));
  current_user = users_get_user(thread_args->users_list, thread_args->user_id);

  while(*(thread_args->pt_status) != SERVER_QUITTING && client_status != CLIENT_QUITTING) {

    /* MESSAGE RECEPTION */
    flush_message(message);
    message = receive_message(thread_args->connection_fd);
    printf("<[%s] : %s", message->source_pseudo, message->text);
    fflush(stdout);

    /* FUNCTION HANDLER */
    if(message->text[0] == '/') { // if a command is sent
      switch (parser(message->text)) {
        case FUNC_NICK:; // TODO bloquer utilisation de l'user "Server", "System", "me"
          user_set_pseudo(thread_args->users_list, thread_args->user_id, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text);
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", current_user->pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_MSG:;
          char * command_arg;
          char * command_text;
          extract_command_args(message->text+strlen("/msg "), &command_arg, &command_text);
          dest_id = users_get_id_by_pseudo(thread_args->users_list, command_arg);
          if (0 != dest_id) {
            send_message_to_user(thread_args->users_list, dest_id, command_text, message->source_pseudo);
            printf(">[%s to %s] : %s", message->source_pseudo, command_arg, command_text);
          }
          else {
            if (0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
            send_message(thread_args->connection_fd, "Server", "Error, unknown user.\n");
            if (0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
            printf("![Server] : %s tried to reach %s but the user does not exist.\n", message->source_pseudo, command_arg);
          }
          fflush(stdout);
          free(command_arg);
          free(command_text);
          break;

        case FUNC_MSGALL:;
          temp_user = thread_args->users_list->next; // avoid user 0 "system"
          while (temp_user != NULL){
            dest_id = temp_user->id;
            if(temp_user->id != current_user->id) {
              send_message_to_user(temp_user, dest_id, message->text + strlen("/msgall "), message->source_pseudo);
              printf(">[%s from %s (broadcast)] : %s", temp_user->pseudo, message->source_pseudo, message->text);
            }
            temp_user = temp_user->next;
          }
          break;

        case FUNC_WHO:;
          users_get_pseudo_display(thread_args->users_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text);
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] users list sent after a '/who' request.\n", message->source_pseudo);
          fflush(stdout);
          break;

        case FUNC_WHOIS:;
          users_get_info_user(thread_args->users_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text);
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_QUIT:; // TODO fix - problème au niveau de la terminaison des threads
          -- *(thread_args->pt_nb_conn); // decrease number of nb_total_connections
          client_status = CLIENT_QUITTING;
          users_delete_user(thread_args->users_list, thread_args->user_id);
          break;

        case FUNC_CHANNEL_CREATE:;
          channels_add_channel(thread_args->channel_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text);
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_CHANNEL_JOIN:;
          channel_add_user(thread_args->channel_list,thread_args->users_list, thread_args->user_id, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text);
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_CHANNEL_QUIT:;
          channel_delete_user(thread_args->channel_list, thread_args->users_list, thread_args->user_id, message->text);
          // TODO send via send_message (avec mutexs)
          break;

        default:;
          printf("![System] : Invalid command from user %i.\n", current_user->id); // TODO informer l'user
          fflush(stdout);
          break;
        } // END switch
      } // END if command sent
    else {
      if (current_user->channel_id != 0) {
        // TODO fonctionnaliser channel broadcast
        struct channel *channel = channels_get_channel(thread_args->channel_list, current_user->channel_id);
        for (int i = 0; i < channel->nb_users_inside; i++) {
          if (channel->members[i]!=current_user->id && channel->members[i]!=0) { // In the channel, do not receive my message
            send_message_to_user(thread_args->users_list, channel->members[i], message->text, message->source_pseudo);
            printf(">[%s]<%s> : %s", current_user->pseudo, channel->name, message->text);
            fflush(stdout);
          } // END if
        } // END for
      } // END if channel_id
    } // END else
  } // END while

  printf("![System] : Connection from user %i stopped.\n", thread_args->user_id);
  fflush(stdout);

  /* CLEAN UP */
  free_message(message);
  close(thread_args->connection_fd); // closing the fd associated to the connection
  free(thread_args);
  return NULL;
}

void duplicate_threads_args(thread_arg * source_args, thread_arg * dest_args) {
  dest_args->connection_fd = source_args->connection_fd;
  dest_args->user_id = source_args->user_id;
  dest_args->users_list = source_args->users_list;
  dest_args->channel_list = source_args->channel_list;
  dest_args->client_IP = source_args->client_IP;
  dest_args->client_port = source_args->client_port;
  dest_args->pt_nb_conn = source_args->pt_nb_conn;
  dest_args->pt_status = source_args->pt_status;
}

void extract_command_args(char *message_pointer, char **pt_command_arg, char **pt_command_text) {
  int i=0;
  while(message_pointer[i] != ' ') { // detect end of the command arg
    i++;
  }
  *pt_command_arg = strndup(message_pointer, i);
  *pt_command_text = strdup(message_pointer+i+1);
}
// TODO : mettre les fonctions de gestion des users dans un fichier
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

void users_delete_user(struct users * list, int user_id_to_delete){
  while (list->next!=NULL) {
    // check next user
    if (user_id_to_delete==list->next->id) {
      struct users * temp = list->next;
      // unlink user
      list->next=list->next->next;
      free(temp->pseudo);
      free(temp);
      // end user research
      break;
    }
    else
      list=list->next;
  }
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

void send_message_to_user(struct users *users, int dest_id, char *text, char *source_pseudo) {
  struct users * dest_user = users_get_user(users, dest_id);
  if(0 != pthread_mutex_lock(&dest_user->communication_mutex)) { error("pthread_mutex_lock"); }
  send_message(dest_user->associated_fd, source_pseudo, text);
  if(0 != pthread_mutex_unlock(&dest_user->communication_mutex)) { error("pthread_mutex_unlock"); }
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
    // TODO fonctionnaliser
    new_channel = malloc(sizeof(struct channel));
    new_channel->name = malloc(strlen(channel_name));
    strcpy(new_channel->name, channel_name);
    new_channel->id = channel_list->id +1;

    for (int i = 0; i < NB_MAX_CLIENT ; i++) { // TODO remplacer par un memset
      new_channel->members[i] = 0;
    }
    new_channel->nb_users_inside = 0;
    new_channel->next = NULL;

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

void user_set_channel(struct users *users_list, int user_id, int channel_id){
  while (users_list!=NULL && users_list->id!=user_id){
    users_list=users_list->next;
  }
  users_list->channel_id = channel_id;
}


void channel_add_user(struct channel * channels_list, struct users* users_list, int user_id, char *message){
  // TODO : vérifier que le user n'est pas déjà dans un autre salon
  struct channel *temp = channels_list->next;
  remove_line_breaks(message);

  // find
  // TODO réutiliser fonction de recherche
  while (temp!=NULL && strcmp(temp->name, message + strlen("/join ")) != 0)
    temp = temp->next;


  memset(message, 0, MSG_MAXLEN);
  if (temp == NULL)
    strcpy(message, "The channel does not exist.\n");
  else {
    ++(temp->nb_users_inside);
    channel_add_user_to_members(temp, user_id);
    user_set_channel(users_list, user_id, temp->id);
    strcpy(message, "You have joined the channel !\n");
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


#include "server_tools.h"

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
  char message[MSG_MAXLEN];
  thread_arg * thread_args;

  /* MALLOC */
  thread_args = malloc(sizeof(thread_arg));

  /* THREAD ARGS GESTION (thread_args independant from thread_input of main) */
  duplicate_threads_args((thread_arg *)thread_input, thread_args);
  free(thread_input);

  /* INCREASE MAIN NB_CONNECTIONS */
  ++ *(thread_args->pt_nb_conn); // increase number of nb_total_connections
  printf("=== Connection %i opened ===\n", *(thread_args->pt_nb_conn));

  /* CREATING THE USER INTO THE USERS TABLE */
  time_t mytime;
  mytime = time(NULL);
  users_add_user(thread_args->users_list,
          thread_args->user_id,
          thread_args->connection_fd,
          "Guest",
          thread_args->client_IP,
          thread_args->client_port,
          ctime(&mytime));

  while(*(thread_args->pt_status) != SERVER_QUITTING && client_status != CLIENT_QUITTING) {
    /* MESSAGE RECEPTION */
    memset(message, 0, MSG_MAXLEN);
    read_line(thread_args->connection_fd, message);
    printf("< Received [%s] : %s\n", users_get_user_pseudo(thread_args->users_list, thread_args->user_id), message);

    /* FUNCTION HANDLER */
    if(message[0] == '/') { // if a command is sent
      switch (parser(message)) {
        case FUNC_NICK:
          remove_line_breaks(message);
          user_set_pseudo(thread_args->users_list, thread_args->user_id, message+strlen("/nick ")); // offset to remove command from temp buffer
          break;

        case FUNC_WHO:
          printf("WHO");
          char * pseudo_list;
          pseudo_list = users_get_pseudo_list(thread_args->users_list);
          memset(message, 0, MSG_MAXLEN);
          strcpy(message, pseudo_list);
          free(pseudo_list);
          break;

        case FUNC_WHOIS:
          printf("WHOIS");
          /*
                 char * info;
      char * pseudo = malloc( (strlen(message)-strlen("/whos ")) * sizeof(char));
      strncpy(pseudo, message + strlen("/whos ")*sizeof(char), strlen(message)-strlen("/whos ")-1); // -1 to remove '\n'
      info = users_get_info_user(thread_args->users_list, pseudo);
      memset(message, 0, MSG_MAXLEN);
      strcpy(message, info);
      free(info);
      free(pseudo);
           */
          break;

        case FUNC_QUIT:
          -- *(thread_args->pt_nb_conn); // decrease number of nb_total_connections
          client_status = CLIENT_QUITTING;
          users_delete_user(thread_args->users_list, thread_args->user_id);
          break;

        default:
          printf("Invalid command.");
          break;
      } // END switch
    } // END if command sent

    /* REPEATER MODE */
    send_line(thread_args->connection_fd, message);
    if(client_status != CLIENT_QUITTING)
      printf("> Sending [%s] : %s\n", users_get_user_pseudo(thread_args->users_list, thread_args->user_id),message);
    }

  printf("=== Connection from user %i stopped ===\n", thread_args->user_id);

  /* CLEAN UP */
  free(thread_args);
  close(thread_args->connection_fd); // closing the fd associated to the connection

  return NULL;
}

void duplicate_threads_args(thread_arg * source_args, thread_arg * dest_args) {
  dest_args->connection_fd = source_args->connection_fd;
  dest_args->user_id = source_args->user_id;
  dest_args->users_list = source_args->users_list;
  dest_args->client_IP = source_args->client_IP;
  dest_args->client_port = source_args->client_port;
  dest_args->pt_nb_conn = source_args->pt_nb_conn;
  dest_args->pt_status = source_args->pt_status;
}

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
  strcpy(new_user->pseudo, pseudo);

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

void user_set_pseudo(struct users * users, int user_id, char * pseudo){
  /* Set the pseudo of the user corresponding to the id */
  // TODO : check if the pseudo is not already used
  while (users->id!=user_id) {
    users = users->next;
  }
  remove_line_breaks(pseudo);
  strcpy(users->pseudo, pseudo);
}

char *users_get_pseudo_list(struct users *users) {
  /* Return the list of pseudo */
  char * pseudo_list = malloc(MSG_MAXLEN*sizeof(char));
  strcpy(pseudo_list, "\nOnline users are : \n");
  while (users!=NULL) {
    if (users->id!=0) {
      strcat(pseudo_list, "\t -");
      strcat(pseudo_list, users->pseudo);
      strcat(pseudo_list, "\n");
    }
    users = users->next;
  }
  return pseudo_list;
}

char *users_get_info_user(struct users * users, char *pseudo){
  /* Return information of the corresponding user */
  char * info = malloc(MSG_MAXLEN*sizeof(char));
  while (strcmp(users->pseudo, pseudo)!=0 && users!=NULL) {
    users = users->next;
  }
  if (users==NULL) {
    sprintf(info, "No user found !");
  }
  else {
    sprintf(info, "%s conncted since %s with the IP address %s and port number %d.\n", pseudo, users->connection_date, users->IP_addr, users->port);
  }
  return info;
}
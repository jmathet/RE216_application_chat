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
  /* Function called at the creation of a thread due to a new connection on the server */
  // Inits
  char message[MSG_MAXLEN];

  // Get thread args
  thread_arg * thread_args = (thread_arg *)thread_input;
  int thread_fd_connection = thread_args->thread_fd_connection;
  int my_id = thread_args->linked_user_id;
  struct users * users_list = thread_args->users;
  char * IP_addr = thread_args->IP_addr;
  unsigned short port_number = thread_args->port_number;

  ++ *(thread_args->pt_nb_conn); // increase number of nb_total_connections
  printf("=== Connection %i opened ===\n", *(thread_args->pt_nb_conn));
  free(thread_input); // Free thread_input

  // TODO : mettre dans une fonction get_date

  time_t mytime;
  mytime = time(NULL);

  users_list = users_add_user(users_list, my_id, thread_fd_connection, "Inconnu", IP_addr, port_number, ctime(&mytime));

  while(1) {
    //read what the client has to say
    memset(message, 0, MSG_MAXLEN);
    read_line(thread_fd_connection, message);
    printf("< Received [%s] : %s\n", users_get_user_pseudo(users_list, my_id), message);

    if (strncmp("/nick", message, strlen("/nick")) == 0) {
      int pseudo_length = strlen(message) - strlen("/nick ") - 1; // \n occupies 1 char
      char pseudo[pseudo_length];
      strcpy(pseudo, message+strlen("/nick "));
      string_strip(pseudo);
      user_set_pseudo(users_list, my_id, pseudo);
      memset(message, 0, MSG_MAXLEN);
      strcpy(message, "Hello ");
      strcat (message, users_get_user_pseudo(users_list, my_id));
    }
    else if (strncmp("/who\n", message, strlen("/who\n")) == 0) {
      char * pseudo_list;
      pseudo_list = users_get_pseudo_list(users_list);
      memset(message, 0, MSG_MAXLEN);
      strcpy(message, pseudo_list);
      free(pseudo_list);
    }
    else if (strncmp("/whos ", message, strlen("/whos ")) == 0) {
      // TODO gérer le cas où on fait "/whos" sans donné de pseudo
      char * info;
      char * pseudo = malloc( (strlen(message)-strlen("/whos ")) * sizeof(char));
      strncpy(pseudo, message + strlen("/whos ")*sizeof(char), strlen(message)-strlen("/whos ")-1); // -1 to remove '\n'
      info = users_get_info_user(users_list, pseudo);
      memset(message, 0, MSG_MAXLEN);
      strcpy(message, info);
      free(info);
      free(pseudo);
    }

    send_line(thread_fd_connection, message);

    printf("> Sending [%s] : %s\n", users_get_user_pseudo(users_list, my_id),message);

    // check if /quit
    if(strncmp("/quit", message, 5) == 0){
      -- *(thread_args->pt_nb_conn); // decrease number of nb_total_connections
      users_delete_user(users_list, my_id);
      break;
    }
    memset(message, 0, MSG_MAXLEN);
  }

  printf("=== Connection stopped ===\n");
  close(thread_fd_connection); // closing the fd associated to the connection
  return NULL; // a thread should return a pointer
}

struct users* users_add_user(struct users * list, int user_id, int thread_fd, char* pseudo, char* IP_addr, unsigned short port, char * date){
  // add a new user at the end of the list users
  struct users * new_user = malloc(sizeof( struct users));

  if (new_user == NULL) {
    error("error creation new user");
  }

  new_user->user_id = user_id;
  new_user->thread_fd = thread_fd;
  new_user->pseudo = pseudo;
  new_user->IP_addr = IP_addr;
  new_user->port = port;
  new_user->date = date;
  new_user->next = NULL;

  if (list == NULL) {
    return new_user;
  }

  struct users *temp;
  temp=list;

  while (temp->next!=NULL) {
    temp=temp->next;
  }
  temp->next=new_user;

  return list;
}

struct users* users_delete_user(struct users * list, int user_id_to_delete){
  /* Delete the user corresponding to the user_id_to_delete and return the new list */
  while (list->next!=NULL) {
    if (user_id_to_delete==list->next->user_id) {
      list->next=list->next->next;
      return list;
    } else {
      list=list->next;
    }
  }
  return list;
}

char * users_get_user_pseudo(struct users * users, int user_id){
  /* Return the pseudo of the user corresponding to the user_id */
  while (users->user_id!=user_id) {
    users = users->next;
  }
  return users->pseudo;
}

void user_set_pseudo(struct users * users, int user_id, char * pseudo){
  /* Set the pseudo of the user corresponding to the user_id */
  // TODO : check if the pseudo is not already used
  while (users->user_id!=user_id) {
    users = users->next;
  }
  users->pseudo = pseudo;
}

char *users_get_pseudo_list(struct users *users) {
  /* Return the list of pseudo */
  char * pseudo_list = malloc(MSG_MAXLEN*sizeof(char));
  strcpy(pseudo_list, "\nOnline users are : \n");
  while (users!=NULL) {
    if (users->user_id!=-1) {
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
    sprintf(info, "%s conncted since %s with the IP address %s and port number %d.\n", pseudo, users->date, users->IP_addr, users->port);
  }
  return info;
}

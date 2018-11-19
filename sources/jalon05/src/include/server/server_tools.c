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
        case FUNC_NICK:;
          user_set_pseudo(thread_args->users_list, thread_args->user_id, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
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
            send_message_to_user(thread_args->users_list, dest_id, command_text, message->source_pseudo, "", 1);
            printf(">[%s to %s] : %s", message->source_pseudo, command_arg, command_text);
          }
          else {
            if (0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
            send_message(thread_args->connection_fd, "Server", "Error, unknown user.\n", "");
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
              send_message_to_user(temp_user, dest_id, message->text + strlen("/msgall "), message->source_pseudo, "", 2);
              printf(">[%s from %s (broadcast)] : %s", temp_user->pseudo, message->source_pseudo, message->text);
            }
            temp_user = temp_user->next;
          }
          break;

        case FUNC_WHO:;
          users_get_pseudo_display(thread_args->users_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] users list sent after a '/who' request.\n", message->source_pseudo);
          fflush(stdout);
          break;

        case FUNC_WHOIS:;
          users_get_info_user(thread_args->users_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_QUIT:; // TODO fix - problème au niveau de la terminaison des threads
          -- *(thread_args->pt_nb_conn); // decrease number of nb_total_connections
          client_status = CLIENT_QUITTING;
          if (current_user->channel_id!=0)
            channel_delete_user(thread_args->channel_list, thread_args->users_list, thread_args->user_id, message->text);
          users_delete_user(thread_args->users_list, thread_args->user_id);
          break;

        case FUNC_CHANNEL_CREATE:;
          channels_add_channel(thread_args->channel_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_CHANNEL_JOIN:;
          channel_add_user(thread_args->channel_list,thread_args->users_list, thread_args->user_id, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_CHANNEL_QUIT:;
          channel_delete_user(thread_args->channel_list, thread_args->users_list, thread_args->user_id, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        case FUNC_CHANNEL_LIST:
          printf("coucou\n");
          channels_get_name_display(thread_args->channel_list, message->text);
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", message->text, "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf(">[%s] : %s", message->source_pseudo, message->text);
          fflush(stdout);
          break;

        default:;
          if(0 != pthread_mutex_lock(&current_user->communication_mutex)) { error("pthread_mutex_lock"); }
          send_message(thread_args->connection_fd, "Server", "The command does not exist !", "");
          if(0 != pthread_mutex_unlock(&current_user->communication_mutex)) { error("pthread_mutex_unlock"); }
          printf("![System] : Invalid command from user %i.\n", current_user->id); // TODO informer l'user
          fflush(stdout);
          break;
        } // END switch
      } // END if command sent
    else {
      if (current_user->channel_id != 0) {
        struct channel *channel = channels_get_channel(thread_args->channel_list, current_user->channel_id);
        for (int i = 0; i < channel->nb_users_inside; i++) {
          if (channel->members[i]!=current_user->id && channel->members[i]!=0) { // In the channel, do not receive my message
            send_message_to_user(thread_args->users_list, channel->members[i], message->text, message->source_pseudo, channel->name, 3);
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

void send_message_to_user(struct users *users, int dest_id, char *text, char *source_pseudo, char *source, short mode) {
  /* Mode : 0=ignore, 1=PM, 2=broadcast, 3=channel */
  struct users * dest_user = users_get_user(users, dest_id);
  char * source_to_send = malloc(MSG_MAXLEN);
  switch (mode) {
    case 1: // PM
      sprintf(source_to_send, "PM");
      break;
    case 2: // broadcast
      sprintf(source_to_send, "broadcast");
      break;
    case 3: // channel
      sprintf(source_to_send, "channel '%s'", source);
      break;
  }
  if(0 != pthread_mutex_lock(&dest_user->communication_mutex)) { error("pthread_mutex_lock"); }
  send_message(dest_user->associated_fd, source_pseudo, text, source_to_send);
  if(0 != pthread_mutex_unlock(&dest_user->communication_mutex)) { error("pthread_mutex_unlock"); }
  free(source_to_send);
}

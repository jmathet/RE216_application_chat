#include "server_tools.h"





void init_serv_addr(struct sockaddr_in *serv_addr, int port)
 {
   /* Modify specified sockaddr_in for the server side with specified port */
   // clean structure
   memset(serv_addr, '\0', sizeof(*serv_addr));
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = htons(port); // specified port in args
   serv_addr->sin_addr.s_addr = INADDR_ANY;
 }

 void do_bind(int socket, struct sockaddr_in addr_in)
 {
   /* Perform a bind on specified socket */
   int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
   if (bind_result == -1) {
     error("Error during socket binding");
   }
 }

 void do_listen(int socket, int nb_max)
 {
   /* Switch specified socket in the listen state */
   int listen_result = listen(socket, nb_max);
   if (listen_result == -1) {
     error("Error during socket listening");
   }
 }

 int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
 {
   /* Accept a connection with the specified socket and return the file des from accepted socket*/
   int file_des_new = accept(socket, addr, addrlen);
   if(file_des_new == -1) {
     error("Error while accepting a connection");
   }
   return file_des_new;
 }

void *connection_handler(void* thread_input)
{
  thread_arg *thread= (thread_arg *)thread_input;
  int sock_fd_connection = thread->thread_fd_connection;
  int sock = thread->thread_sock;
  char message[MSG_MAXLEN];

  free(thread_input);


  //read what the client has to say
  memset(message, '\0', MSG_MAXLEN);
  int read_length;

  while((read_length = readline(sock_fd_connection, message, MSG_MAXLEN)) > 0)
  {
    printf("< Received : %s\n", message);
    sendline(sock_fd_connection, message, MSG_MAXLEN);
    printf("> Sending : %s\n", message);
  }

  // check if /quit
  if(strncmp("/quit", message, 5) == 0) {
    printf("=== Quiting. ===\n");
    close(sock);//fermeture de la socket

  }

    return NULL; //une fonction exécutée par un thread doit retourner un pointeur
}

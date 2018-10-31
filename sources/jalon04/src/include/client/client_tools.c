#include "client_tools.h"

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port)
 {
   /* Modify specified sockaddr_in for the client side with specified port and IP */
   // clean structure
   memset(serv_addr, '\0', sizeof(*serv_addr));
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = htons(port); // specified port in args
   serv_addr->sin_addr.s_addr = inet_addr(ip); // specified server IP in args
 }

 void do_connect(int sock, struct sockaddr_in host_addr)
 {
   int connect_result;
   do {
     connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_addr));
   } while ((connect_result == -1) && (errno == EAGAIN || errno == EINTR));

   if (connect_result == -1)
     error("connect");
 }

 int is_pseudo_correct(char * pseudo) {
  if (strlen(pseudo) > 1 && *pseudo != ' ')
    return 1;
  else
    return 0;
}

void * reception_handler(void * arg) {
  /* INITS */
  reception_arg * input = (reception_arg *) arg;
  char message[MSG_MAXLEN];

  /* RECEPTION AND DISPLAY OF MESSAGES */
  while(input->status != CLIENT_QUITTING) {
    memset(message, 0, MSG_MAXLEN);
    pthread_mutex_lock(&input->sock_mutex);
    read_line(input->sock, message);
    printf("\n< Answer received : %s\n", message);
    pthread_mutex_unlock(&input->sock_mutex);

    // check if /quit
    if(strncmp("/quit", message, 5) == 0)
      input->status = CLIENT_QUITTING;
  }
  return NULL;
}

void * communication_handler(void * arg) {
  /* INITS */
  communication_arg * input = (communication_arg *) arg;
  char message[MSG_MAXLEN];

  /* EMISSION OF MESSAGES */
  while(input->status != CLIENT_QUITTING) {
    memset(message, 0, MSG_MAXLEN);
    fgets(message, MSG_MAXLEN-1, stdin);
    printf("> Sending : %s\n", message);
    pthread_mutex_lock(&input->sock_mutex);
    send_line(input->sock, message);
    pthread_mutex_unlock(&input->sock_mutex);

    // check if /quit
    if(strncmp("/quit", message, 5) == 0)
      input->status = CLIENT_QUITTING;
  }
  return NULL;
}
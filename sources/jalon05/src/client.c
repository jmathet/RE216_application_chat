#include "include/client/client_tools.h"
#include "include/general_tools.h"

int main(int argc,char** argv)
{
    /* INITS */
    volatile int status;
    int host_port;
    int sock;
    struct sockaddr_in host_addr;
    char host_ip[10];
    char * pseudo;
    pthread_t reception_thread;
    pthread_t communication_thread;
    pthread_t file_reception_thread;
    pthread_t file_emission_thread;
    reception_arg * reception_input;
    communication_arg * communication_input;
    file_reception_arg * file_reception_arg;
    file_communication_arg * file_communication_arg;
    pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;

    /* MALLOC */
    reception_input = malloc(sizeof(reception_arg));
    communication_input = malloc(sizeof(communication_arg));
    file_reception_arg = malloc(sizeof(file_reception_arg));
    file_communication_arg = malloc(sizeof(file_communication_arg));
    pseudo = malloc(MSG_MAXLEN);

    /* ARGS CHECK */
    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        exit(EXIT_FAILURE);
    }
    host_port = atoi(argv[2]);
    strncpy(host_ip, argv[1], 10);
    if(host_port <= 1024)
    {
      fprintf(stderr, "![System] : Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }

    /* SOCKET SET-UP */
    init_client_addr(&host_addr, host_ip, host_port);
    sock = do_socket();
    do_connect(sock, host_addr);

    /* CHECK SERVER CAPACITY */
    if(read_int(sock) == SERVER_FULL) {
      printf("![Server] : Too many users connected to the server. Connection closed.\n");
      status = CLIENT_QUITTING;
    }
    else {
      /* AUTHENTIFICATION */
      status=CLIENT_NOT_LOGGED;
      auth_user(sock, pseudo);
      status=CLIENT_RUNNING;

      /* RECEPTION THREAD */
      reception_input->sock = sock;
      reception_input->status = status;
      reception_input->sock_mutex = sock_mutex;
      if(0 != pthread_create(&reception_thread, NULL, reception_handler, reception_input))
        error("pthread_create");

      /* COMMUNICATION THREAD */
      communication_input->sock = sock;
      communication_input->status = status;
      communication_input->sock_mutex = sock_mutex;
      communication_input->pseudo = pseudo;
      if(0 != pthread_create(&communication_thread, NULL, communication_handler, communication_input))
        error("pthread_create");

      /* THREADS JOIN */
      if(0 != pthread_join(reception_thread, NULL))
        error("pthread_join");
      if(0 != pthread_join(communication_thread, NULL))
        error("pthread_join");
      // TODO
      printf("![System] : Quiting.\n");
    }

    /* CLEAN UP */
    free(pseudo);
    free(reception_input);
    free(communication_input);
    close(sock);

    return 0;
}

#include "include/server/server_tools.h"
#include "include/general_tools.h"

#define USERS_NB_MAX 2

int main(int argc, char** argv)
{
    /* INITS */
    int port, enable=1;
    int sock, connection_fd, status, nb_total_connections = 0;
    struct sockaddr_in serv_addr;
    pthread_t thread;
    thread_arg * thread_input; // args for thread creation
    struct users * first_user = malloc(sizeof(struct users));
    first_user->user_id = -1;

    /* ARGS CHECK */
    if (argc != 2) {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);
    if(port <= 1024) {
      fprintf(stderr, "Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }


    /* SOCKET SET-UP */
    sock = do_socket();
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
      error("setsockopt");
    init_serv_addr(&serv_addr, port);
    do_bind(sock, serv_addr);
    do_listen(sock, USERS_NB_MAX);
    status=SERVER_RUNNING;

    /* SIGNAL HANDLERS */
    // TODO : gérer la fermeture du serveur correctement + nettoyage + gestion fin des threads

    /* CHAT SERVER */
    while(status != SERVER_QUITTING)
    {
        // accept connection from client
        socklen_t addrlen = sizeof(struct sockaddr);
        connection_fd = do_accept(sock, (struct sockaddr*)&serv_addr, &addrlen);

        // check if there is a reason to refuse the client
        if(nb_total_connections >= USERS_NB_MAX) {// is the max numbers of clients reached ?
          printf("! Connection from a client closed : too many users.\n");
          send_int(connection_fd, SERVER_FULL);
          close(connection_fd);
        }
        else { // there is remaining slots for a new user
          send_int(connection_fd, SERVER_RUNNING);
          // thread args initialisation
          thread_input = (thread_arg*)malloc(sizeof *thread_input);
          thread_input->thread_fd_connection = connection_fd;
          thread_input->linked_user_id = nb_total_connections + 1;
          thread_input->pt_nb_conn = &nb_total_connections;
          thread_input->users = first_user;

          // thread creation
          if( pthread_create( &thread, NULL ,  connection_handler , (void*)thread_input) != 0)
          {
              free(thread_input);
              close(sock);
              error("pthread_create");
          }
        } // END else
      } // END while(1)

    // CLEANING
    close(sock);
    return 0;
}

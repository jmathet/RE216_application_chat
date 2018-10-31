#include "include/server/server_tools.h"
#include "include/general_tools.h"

#define USERS_NB_MAX 2

int main(int argc, char** argv)
{
    /* INITS */
    int enable=1; // used for setsockopt
    int port;
    int sock;
    int connection_fd;
    int status;
    int nb_total_connections = 0;
    struct sockaddr_in serv_addr;
    pthread_t thread;
    thread_arg * thread_input; // args for thread creation
    // TODO a ameliorer
    struct users * first_user = malloc(sizeof(struct users));
    first_user->user_id = -1;
    first_user->pseudo = "";

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
    if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
      error("setsockopt");
    init_serv_addr(&serv_addr, port);
    do_bind(sock, serv_addr);
    do_listen(sock, USERS_NB_MAX);
    status=SERVER_RUNNING;

    /* SERVER - main flow is accepting connections and creating threads */
    while(status != SERVER_QUITTING)
    {
        // receiving new connections from clients
        socklen_t addrlen = sizeof(struct sockaddr);
        connection_fd = do_accept(sock, (struct sockaddr*)&serv_addr, &addrlen);

        // check if there is a reason to refuse the client
        if(nb_total_connections >= USERS_NB_MAX) {// is the max numbers of clients reached ?
          printf("! Connection from a client closed : too many users.\n");
          send_int(connection_fd, SERVER_FULL);
          status = SERVER_QUITTING;
        }
        else { // we accept the client
          // malloc
          thread_input = (thread_arg*)malloc(sizeof *thread_input);

          // sending OK signal with the pseudo-protocol
          send_int(connection_fd, SERVER_RUNNING);

          // thread args initialisation
          thread_input->thread_fd_connection = connection_fd;
          thread_input->linked_user_id = nb_total_connections + 1;
          thread_input->pt_nb_conn = &nb_total_connections;
          thread_input->users = first_user;
          thread_input->IP_addr = inet_ntoa(serv_addr.sin_addr);
          thread_input->port_number = ntohs(serv_addr.sin_port);

          // thread creation
          if(0 != pthread_create( &thread, NULL ,  connection_handler , (void*)thread_input))
              error("pthread_create");

          // detach thread
          if(0 != pthread_detach(thread))
            error("pthread_detach");

          // TODO free thread-specific inputs
          // free(thread_input);

        } // END else
      } // END while(status != SERVER_QUITTING)

    /* CLEAN UP */
    close(sock);

    return 0;
}

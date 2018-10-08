#include "include/server/server_tools.h"
#include "include/general_tools.h"

#define USERS_NB_MAX 2

int main(int argc, char** argv)
{
    // INPUT ARGS CHECK
    if (argc != 2) {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        exit(EXIT_FAILURE);
    }
    // INPUT PORT CHECK
    int port = atoi(argv[1]);
    if(port <= 1024) {
      fprintf(stderr, "Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }

    // INITS
    int sock, connection_fd;
    int nb_total_connections = 0;
    struct sockaddr_in serv_addr;
    pthread_t thread;
    thread_arg * thread_input; // args for thread creation

    // SOCKET SET-UP
    sock = do_socket();
    init_serv_addr(&serv_addr, port);
    do_bind(sock, serv_addr);
    do_listen(sock, USERS_NB_MAX);

    while(1)
    {
        // accept connection from client
        socklen_t addrlen = sizeof(struct sockaddr);
        connection_fd = do_accept(sock, (struct sockaddr*)&serv_addr, &addrlen);

        // check if there is a reason to refuse the client
        if(nb_total_connections >= USERS_NB_MAX) {// is the max numbers of clients reached ?
          printf("! Connection from a client closed : too many users.\n");
          sendline(connection_fd, "TOO_MANY_USERS");
          close(connection_fd);
        }
        else { // there is remaining slots for a new user
          sendline(connection_fd, "OK");
          // thread args initialisation
          thread_input = (thread_arg*)malloc(sizeof *thread_input);
          thread_input->thread_fd_connection = connection_fd;
          thread_input->pt_nb_conn = &nb_total_connections;

          // thread creation
          if( pthread_create( &thread, NULL ,  connection_handler , (void*)thread_input) != 0)
          {
              free(thread_input);
              close(sock);
              error("Impossible to create a thread.");
        } // END else
      }
    } // END while(1)

    close(sock);
    return 0;
}

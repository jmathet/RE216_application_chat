#include "include/server/server_tools.h"
#include "include/general_tools.h"

#define USERS_NB_MAX 5

int main(int argc, char** argv)
{
    /* INITS */
    int enable=1; // used for setsockopt
    int port;
    int sock;
    int connection_fd;
    volatile int status;
    volatile int nb_connections = 0;
    int users_counter = 1; // start users index at 1
    struct sockaddr_in6 serv_addr;
    struct sockaddr_in6 client_addr;
    pthread_t thread;
    thread_arg * thread_input; // args for thread creation
    struct users * system_user;
    struct channel * system_channel;

    /* MALLOC */
    system_user = malloc(sizeof(struct users));
    system_channel = malloc((sizeof(struct channel)));

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

    /* SYSTEM USER CREATION (first and permanent invisible user) */
    memset(system_user, 0, sizeof(struct users));
    system_user->id = 0;
    system_user->pseudo = "";
    system_user->next = NULL;

    /* SYSTEM CHANNEL CREATION (first and permanent invisible channel) */
    memset(system_channel, 0, sizeof(struct channel));
    system_channel->id = 0;
    system_channel->name = "";
    system_channel->next = NULL;


  /* SERVER - main flow is accepting connections and creating threads */
    while(status != SERVER_QUITTING)
    {
        // receiving new connections from clients
        socklen_t addrlen = sizeof(struct sockaddr);
        connection_fd = do_accept(sock, (struct sockaddr*)&client_addr, &addrlen);

        // check if there is a reason to refuse the client
        if(nb_connections >= USERS_NB_MAX) {// is the max numbers of clients reached ?
          printf("![Server] : Connection from a client closed : too many users.\n");
          send_int(connection_fd, SERVER_FULL);
          status = SERVER_QUITTING;
        }
        else { // we accept the client
          // malloc the thread args (free will be made by the thread once copied)
          thread_input = (thread_arg*)malloc(sizeof *thread_input);

          // sending OK signal with the pseudo-protocol
          send_int(connection_fd, SERVER_RUNNING);

          // thread args initialisation
          thread_input->connection_fd = connection_fd;
          thread_input->user_id = users_counter++;
          thread_input->pt_nb_conn = &nb_connections;
          thread_input->pt_status = &status;
          thread_input->users_list = system_user;
          thread_input->channel_list = system_channel;
          inet_ntop(AF_INET6, &client_addr.sin6_addr, thread_input->client_IP, sizeof(client_addr.sin6_addr));
          thread_input->client_port = ntohs(client_addr.sin6_port);

          // thread creation
          if(0 != pthread_create( &thread, NULL ,  connection_handler , (void*)thread_input))
              error("pthread_create");

          // detach thread
          if(0 != pthread_detach(thread))
            error("pthread_detach");

        } // END else
      } // END while(status != SERVER_QUITTING)

    /* CLEAN UP */
    close(sock);
    free(system_user->pseudo);
    free(system_user->connection_date);
    free(system_user->IP_addr);
    free(system_user);

    free(system_channel->name);
    free(system_channel);

    return 0;
}

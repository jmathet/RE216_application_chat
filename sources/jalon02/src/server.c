#include "include/server/server_tools.h"
#include "include/general_tools.h"

#define MSG_MAXLEN 30

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if(port <= 1024) {
      fprintf(stderr, "Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in serv_addr;

    //create the socket, check for validity!
    sock = do_socket();

    //init the serv_add structure
    init_serv_addr(&serv_addr, port);

    //perform the binding
    //we bind on the tcp port specified
    do_bind(sock, serv_addr);

    //specify the socket to be a server socket and listen for at most 20 concurrent client
    do_listen(sock, 20);

    char message[MSG_MAXLEN];
    for (;;)
    {
        //accept connection from client
        socklen_t addrlen = sizeof(struct sockaddr);
        int connection_fd = do_accept(sock, (struct sockaddr*)&serv_addr, &addrlen);

        //read what the client has to say
        memset(message, '\0', MSG_MAXLEN);
        int read_length;
        while((read_length = readline(connection_fd, message, MSG_MAXLEN)) > 0)
        {
          printf("< Received : %s\n", message);
          //sendline(connection_fd, message, strlen(message));
          sendline(connection_fd, message, MSG_MAXLEN);
          printf("> Sending : %s\n", message);
        }

        // check if /quit
        if(strncmp("/quit", message, 5) == 0) {
          printf("=== Quiting. ===\n");
          break;
        }
    }

    //clean up server socket
    close(sock);

    return 0;
}

#include "include/client/client_tools.h"
#include "include/general_tools.h"

int main(int argc,char** argv)
{
    /* INITS */
    int status, host_port, sock;
    struct sockaddr_in host_addr;
    char host_ip[10], message[MSG_MAXLEN], reply[MSG_MAXLEN];

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
      fprintf(stderr, "Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }


    /* SOCKET SET-UP */
    init_client_addr(&host_addr, host_ip, host_port);
    sock = do_socket();
    do_connect(sock, host_addr);

    /* CHAT CLIENT */
    if(read_int(sock) == SERVER_FULL) {
      printf("Too many users connected to the server. Connection closed.\n");
      status=CLIENT_QUITTING;
    }
    else {
      // Ensure client authentification
      status=CLIENT_NOT_LOGGED;
      do {
        printf("Please identify yourself by using '/nick <Your Name>' : ");
        //get user input
        memset(message, 0, MSG_MAXLEN);
        fgets(message, MSG_MAXLEN-1, stdin);

        if(strncmp("/nick ", message, 6) == 0 && is_pseudo_correct(message+6) == 1) {
          printf("> Sending : %s\n", message);
          send_line(sock, message);
          // receive answer
          memset(reply, 0, MSG_MAXLEN);
          read_line(sock, reply);
          printf("< Answer received : %s\n", reply);
          status = CLIENT_LOGGED;
        }
      } while(status != CLIENT_LOGGED);

      while(status != CLIENT_QUITTING) {
        printf("Message: ");
        //get user input
        memset(message, 0, MSG_MAXLEN);
        fgets(message, MSG_MAXLEN-1, stdin);

        // send it to server
        printf("> Sending : %s\n", message);
          send_line(sock, message);

        // receive answer
        memset(reply, 0, MSG_MAXLEN);
          read_line(sock, reply);
        printf("< Answer received : %s\n", reply);

        // check if /quit
        if(strncmp("/quit", message, 5) == 0) {
          printf("=== Quiting. ===\n");
          status = CLIENT_QUITTING;
        }

        // clean message and reply
        memset(message, 0, MSG_MAXLEN);
        memset(reply, 0, MSG_MAXLEN);
      }
    }

    close(sock);
    return 0;
}

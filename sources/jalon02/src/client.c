#include "include/client/client_tools.h"
#include "include/general_tools.h"

int main(int argc,char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        return 1;
    }

    int host_port = atoi(argv[2]);
    char *host_ip = argv[1];
    if(host_port <= 1024)
    {
      fprintf(stderr, "Please use a non reserved port number.");
      exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in host_addr;

//get address info from the server
    init_client_addr(&host_addr, host_ip, host_port);


//get the socket
    sock = do_socket();

//connect to remote socket
    do_connect(sock, host_addr);

    char message[MSG_MAXLEN], reply[MSG_MAXLEN];
    while(1) {
      printf("Message: ");
      //get user input
      //scanf("%s", message);
      memset(message, '\0', MSG_MAXLEN);
      fgets(message, MSG_MAXLEN, stdin);

      // send it to server
      printf("> Sending : %s\n", message);
      sendline(sock, message, MSG_MAXLEN);

      // receive answer
      memset(reply, '\0', MSG_MAXLEN);
      readline(sock, reply, MSG_MAXLEN);
      printf("< Answer received : %s\n", reply);

      // check if /quit
      if(strncmp("/quit", message, 5) == 0) {
        printf("=== Quiting. ===\n");
        break;
      }

      // clean message and reply
      memset(message, '\0', MSG_MAXLEN);
      memset(reply, '\0', MSG_MAXLEN);
    }

    close(sock);
    return 0;
}

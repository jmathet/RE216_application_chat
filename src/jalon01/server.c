#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int do_socket()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
    perror("socket"); exit(EXIT_FAILURE);
  return sock;
}

struct sockaddr_in init_serv_addr(char** argv)
 {
   struct sockaddr_in serv_addr;
   memset(&serv_addr, '\0', sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(atoi(argv[1]));
   serv_addr.sin_addr.s_addr = INADDR_ANY;

   return serv_addr;
 }

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        return 1;
    }

    int sock, bind_result, listen_result;
    struct sockaddr_in serv_addr;

    //create the socket, check for validity!
    sock = do_socket();


    //init the serv_add structure
    serv_addr = init_serv_addr(argv);

    //perform the binding
    //we bind on the tcp port specified
    bind_result = bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (bind_result == -1) {
      perror("bind");
      exit(EXIT_FAILURE);
    }


    //specify the socket to be a server socket and listen for at most 20 concurrent client
    listen_result = listen(sock, 20);
    if (listen_result == -1) {
      perror("listen");
      exit(EXIT_FAILURE);
    }

    for (;;)
    {

        //accept connection from client
        //do_accept()

        //read what the client has to say
        //do_read()

        //we write back to the client
        //do_write()

        //clean up client socket
    }

    //clean up server socket
    close(sock);

    return 0;
}

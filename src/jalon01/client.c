#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct sockaddr_in get_addr_info(char** argv)
{
  struct sockaddr_in host_addr;
  memset(&client_addr, '\0', sizeof(host_addr));

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(atoi(argv[2]));
  inet_aton(argv[1], &host_addr.sin_addr);

  return client_addr;
}

int do_socket()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  return sock;
}

int do_connect(int sock, struct sockaddr_in host_addr)
{
  int connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_add));
  if(connect_result == -1)
  {
    perror("connect");
    exit(EXIT_FAILURE);
  }
}

int main(int argc,char** argv)
{
  struct sockaddr_in host_addr;
  int sock;

    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        return 1;
    }

//get address info from the server
  host_addr = get_addr_info(argv);


//get the socket
  sock = do_socket();

//connect to remote socket
  do_connect(sock, host_addr);


//get user input
//readline()

//send message to the server
//handle_client_message()

  close(sock);

    return 0;


}

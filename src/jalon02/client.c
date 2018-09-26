#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MSG_MAXLEN 30

void error(const char *msg);
int do_socket();
void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port);
void do_connect(int sock, struct sockaddr_in host_addr);
ssize_t readline(int file_des, void *str, size_t maxlen);
void sendline(int file_des, const void *str, size_t maxlen);

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
    printf("=== You are now connected ! ===\n");

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

void error(const char *msg)
{
  /* Interrupt program because of an error */
    perror(msg);
    exit(EXIT_FAILURE);
}

int do_socket()
{
  /* Create a socket and return the associated file descriptor */
  int file_des = socket(AF_INET, SOCK_STREAM, 0);
  if (file_des == -1) {
    error("Error during socket creation");
  }

  return file_des;
}

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port)
 {
   /* Modify specified sockaddr_in for the client side with specified port and IP */
   // clean structure
   memset(serv_addr, '\0', sizeof(*serv_addr));
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = htons(port); // specified port in args
   serv_addr->sin_addr.s_addr = inet_addr(ip); // specified server IP in args
 }

 void do_connect(int sock, struct sockaddr_in host_addr)
 {
   int connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_addr));
   if (connect_result == -1) {
     error("Error during connection");
   }
 }

ssize_t readline(int file_des, void *str, size_t maxlen)
{
  /* Read a line from the file descriptor with a maximum length in the specified buffer */
  return recv(file_des, str, maxlen, 0);

}

void sendline(int file_des, const void *str, size_t maxlen)
{
  /* Write a line in the file descriptor with a maximum length with the given buffer */
  int sent_length=0;
  do {
    sent_length += send(file_des, str + sent_length, maxlen - sent_length, 0);
  } while (sent_length != maxlen);
}

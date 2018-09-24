#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void error(const char *msg);
int do_socket();
void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port);
void do_connect(int sock, struct sockaddr_in host_addr);
ssize_t readline(int file_des, void *str, size_t maxlen);
ssize_t sendline(int file_des, const void *str, size_t maxlen);

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


//get user input
printf("Message à envoyer au serveur : \n");
char* message;
int maxlen = 30;
message = malloc(sizeof(char)*maxlen);
ssize_t imput_test = -1;
while (imput_test==-1) {
  imput_test = readline(STDIN_FILENO, message, maxlen);
}


//send message to the server
send(sock, message, maxlen, 0);
//handle_client_message()

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
  read(file_des, str, maxlen);
}

ssize_t sendline(int file_des, const void *str, size_t maxlen)
{
  /* Write a line in the file descriptor with a maximum length with the given buffer */
  int sent=0;
  do {
    sent += write(file_des, str + sent, maxlen - sent);
  } while (sent != maxlen);
}

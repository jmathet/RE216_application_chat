#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg);
int do_socket();
void init_serv_addr(struct sockaddr_in *serv_addr, int port);
void do_bind(int socket, struct sockaddr_in addr_in);
void do_listen(int socket, int nb_max);
 int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);
ssize_t readline(int file_des, void *str, size_t maxlen);
ssize_t sendline(int file_des, const void *str, size_t maxlen);

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

    for (;;)
    {
        socklen_t addrlen = sizeof(serv_addr);
        //accept connection from client
        int connection_fd = do_accept(sock, (struct sockaddr*)&serv_addr, &addrlen);

        //read what the client has to say
        //do_read()
        char message[30];
        memset(message, '\0', 30);
        readline(connection_fd, &message, 30);
        printf("%s\n", message);

        //we write back to the client
        //do_write()

        //clean up client socket
    }

    //clean up server socket
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

void init_serv_addr(struct sockaddr_in *serv_addr, int port)
 {
   /* Modify specified sockaddr_in for the server side with specified port */
   // clean structure
   memset(serv_addr, '\0', sizeof(*serv_addr));
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = htons(port); // specified port in args
   serv_addr->sin_addr.s_addr = INADDR_ANY;
 }

 void do_bind(int socket, struct sockaddr_in addr_in)
 {
   /* Perform a bind on specified socket */
   int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
   if (bind_result == -1) {
     error("Error during socket binding");
   }
 }

 void do_listen(int socket, int nb_max)
 {
   /* Switch specified socket in the listen state */
   int listen_result = listen(socket, nb_max);
   if (listen_result == -1) {
     error("Error during socket listening");
   }
 }

 int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
 {
   /* Accept a connection with the specified socket and return the file des from accepted socket*/
   int file_des_new = accept(socket, addr, addrlen);
   if(file_des_new == -1) {
     error("Error while accepting a connection");
   }
   return file_des_new;
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

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

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        return 1;
    }


    //create the socket, check for validity!
    //do_socket()


    //init the serv_add structure
    //init_serv_addr()

    //perform the binding
    //we bind on the tcp port specified
    //do_bind()

    //specify the socket to be a server socket and listen for at most 20 concurrent client
    //listen()

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

    return 0;
}

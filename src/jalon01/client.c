#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int main(int argc,char** argv)
{


    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        return 1;
    }

//get address info from the server
//get_addr_info()


//get the socket
//s = do_socket()

//connect to remote socket
//do_connect()


//get user input
//readline()

//send message to the server
//handle_client_message()


    return 0;


}

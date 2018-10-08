#include "general_tools.h"

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


void readline(int file_des, void *str)
{
  /* Read a line with length detection (pseudo-protocol) */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  // FIRSTLY : reading the length of the string to receive
  int32_t received; // received raw data
  char *length_received = (char *)&received; // pointer on raw data
  int left=sizeof(received);
  int read_count = 0;
  do {
    read_count = read(file_des, length_received, left);
    if(read_count == -1)
      error("Error while reading.");
    else {
      length_received += read_count;
      left -= read_count;
    }
  } while (left>0);

  // SECONDLY : receive the string
  left = ntohl(received); // final conversion
  read_count = 0;
  do {
    read_count = read(file_des, str, left); // writing directly into the buffer
    if(read_count == -1)
      error("Error while reading.");
    else {
      str += read_count;
      left -= read_count;
    }
  } while (left>0);
}

void sendline(int file_des, const void *str)
{
  /* Sending a line with length-detection (pseudo-protocol) */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  int str_length = strlen(str); // getting the legnth

  // FIRSTLY : sending the length of the message
  int32_t converted_length = htonl(str_length); // convert the length in a generic type independant from infrastucture for emission over socket
  char * data_length = (char *)&converted_length; // pointer on the remaining converted data to send
  int left = sizeof(converted_length);
  int read_count = 0;
  do { // sending the length of coming string, based on the size of int32_t (pseudo-protocol)
    read_count = write(file_des, data_length, left);
    if(read_count == -1)
      error("send");
    else {
      data_length += read_count;
      left -= read_count;
    }
  } while(left > 0);

  // SECONDLY : sending the string
  left = str_length;
  read_count = 0;
  do {
    read_count = write(file_des, str, left);
    if(read_count == -1)
      error("send");
    else {
      str += read_count;
      left -= read_count;
    }
  } while(left > 0);
}

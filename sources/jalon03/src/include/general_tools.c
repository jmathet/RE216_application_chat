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

void send_int(int file_des, int to_send) {
  /* Send an integer into the socket, with conversion to int32_t for cross-compatibility */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  int32_t converted_to_send = (int)to_send; // convert the length in a generic type independant from infrastucture for emission over socket
  char * data = (char *)&converted_to_send; // pointer on the remaining converted data to send
  int left = sizeof(converted_to_send);
  int sent = 0;

  do { // sending the length of coming string, based on the size of int32_t (pseudo-protocol)
    sent = write(file_des, data, left);
    if(sent == -1)
      error("send");
    else {
      data += sent;
      left -= sent;
    }
  } while(left > 0);
}

int read_int(int file_des) {
  /* Read an integer from the socket, with conversion to int32_t for cross-compatibility */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  int32_t received; // received raw data
  int left=sizeof(received);
  char *data_received = (char *)&received; // pointer on raw data

  int read_count = 0;
  do {
    read_count = read(file_des, data_received, left);
    if(read_count == -1)
      error("Error while reading.");
    else {
      data_received += read_count;
      left -= read_count;
    }
  } while (left>0);

  return (int)received;
}

void read_line(int file_des, void *str)
{
  /* Read a line with length detection (pseudo-protocol) */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  // Reading the length of the string to receive
  int left = read_int(file_des);

  // Receive the string
  int read_count = 0;
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

void send_line(int file_des, const void *str)
{
  /* Sending a line with length-detection (pseudo-protocol) */
  /* Thanks to garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c */

  // Sending the length of the string
  int str_length = strlen(str); // getting the legnth
  send_int(file_des, str_length);

  // Sending the string
  int left = str_length;
  int read_count = 0;
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

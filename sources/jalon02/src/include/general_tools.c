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
    sent_length += write(file_des, str + sent_length, maxlen - sent_length);
  } while (sent_length != maxlen);
}

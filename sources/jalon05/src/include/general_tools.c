#include "general_tools.h"

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int do_socket()
{
  int file_des;
  do {
    file_des = socket(AF_INET, SOCK_STREAM, 0);
  } while ((file_des == -1) && (errno == EAGAIN || errno == EINTR));

  if (file_des == -1)
    error("socket");

  return file_des;
}

void init_serv_addr(struct sockaddr_in *serv_addr, int port)
{
  // clean structure
  memset(serv_addr, 0, sizeof(*serv_addr));
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = htons(port); // specified port in args
  serv_addr->sin_addr.s_addr = INADDR_ANY;
}

void do_bind(int socket, struct sockaddr_in addr_in)
{
  int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
  if (-1 == bind_result)
    error("bind");
}

void do_listen(int socket, int nb_max)
{
  int listen_result = listen(socket, nb_max);
  if (-1 == listen_result)
    error("listen");
}

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
{
  int file_des_new = accept(socket, addr, addrlen);
  if(-1 == file_des_new)
    error("accept");
  return file_des_new;
}

void send_int(int file_des, int to_send) {
  // Inspired by garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c
  int32_t converted_to_send = (int)to_send; // convert the length in a generic type independant from infrastucture for emission over socket
  char * data = (char *)&converted_to_send; // pointer on the remaining converted data to send
  int left = sizeof(converted_to_send);
  int sent = 0;

  do { // sending the length of coming string, based on the size of int32_t (pseudo-protocol)
    sent = write(file_des, data, left);
    if((sent == -1) && (errno != EAGAIN) && (errno !=EINTR))
      error("send");
    else {
      data += sent;
      left -= sent;
    }
  } while(left > 0);
}

int read_int(int file_des) {
  // Inspired by garlix's answer from https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c
  int32_t received; // received raw data
  int left=sizeof(received);
  char *data_received = (char *)&received; // pointer on raw data

  int read_count = 0;
  do {
    read_count = read(file_des, data_received, left);
    if((read_count == -1) && (errno != EAGAIN) && (errno !=EINTR))
      error("read");
    else {
      data_received += read_count;
      left -= read_count;
    }
  } while (left>0);

  return (int)received;
}

void read_line(int file_des, void *str)
{
  // Reading the length of the string to receive
  int left = read_int(file_des);
  // Receive the string
  int read_count = 0;
  do {
    read_count = read(file_des, str, left); // writing directly into the buffer
    if((read_count == -1) && (errno != EAGAIN) && (errno !=EINTR))
      error("read");
    else {
      str += read_count;
      left -= read_count;
    }
  } while (left>0);
}

void send_line(int file_des, const void *str)
{
  // Sending the length of the string
  int str_length = strlen(str); // getting the legnth
  send_int(file_des, str_length);

  // Sending the string
  int left = str_length;
  int read_count = 0;
  do {
    read_count = write(file_des, str, left);
    if((read_count == -1) && (errno != EAGAIN) && (errno !=EINTR))
      error("send");
    else {
      str += read_count;
      left -= read_count;
    }
  } while(left > 0);
}

message * init_message() {
  message * message = NULL;
  message = malloc(sizeof(message));
  if(NULL == message)
    error("malloc");

  message->source_pseudo=malloc(MSG_MAXLEN);
  message->source=malloc(MSG_MAXLEN);
  message->text=malloc(MSG_MAXLEN);

  flush_message(message);
  return message;
}

void free_message(message * message) {
  free(message->source_pseudo);
  free(message->source);
  free(message->text);
  free(message);
}

void flush_message(message * message) {
  memset(message->source_pseudo, 0, MSG_MAXLEN);
  memset(message->source, 0, MSG_MAXLEN);
  memset(message->text, 0, MSG_MAXLEN);
}

void send_message(int file_des, char *source_pseudo, const void *text, char *source)
{
 send_line(file_des, source_pseudo);
 send_line(file_des, text);
 send_line(file_des, source);
}

message * receive_message(int file_des) {
  message * message = init_message();
  read_line(file_des, message->source_pseudo);
  read_line(file_des, message->text);
  read_line(file_des, message->source);
  return message;
}

int parser(char * message) {
  if(0 == strncmp("/quit\n", message, strlen("/quit\n")))
    return FUNC_QUIT;
  else if(0 == strncmp("/nick ", message, strlen("/nick ")))
    return FUNC_NICK;
  else if(0 == strncmp("/whois ", message, strlen("/whois ")))
    return FUNC_WHOIS;
  else if(0 == strncmp("/who", message, strlen("/who")))
    return FUNC_WHO;
  else if(0 == strncmp("/msg ", message, strlen("/msg ")))
    return FUNC_MSG;
  else if(0 == strncmp("/msgall ", message, strlen("/msgall ")))
    return FUNC_MSGALL;
  else if(0 == strncmp("/create ", message, strlen("/create ")))
    return FUNC_CHANNEL_CREATE;
  else if(0 == strncmp("/join ", message, strlen("/join ")))
    return FUNC_CHANNEL_JOIN;
  else if(0 == strncmp("/channels ", message, strlen("/channels")))
    return FUNC_CHANNEL_LIST;
  else if(0 == strncmp("/quit ", message, strlen("/quit ")))
    return FUNC_CHANNEL_QUIT;
  else if(0 == strncmp("/send ", message, strlen("/send ")))
    return FUNC_SEND;
  else if(0 == strncmp("/accept ", message, strlen("/accept ")))
    return FUNC_ACCEPT;
  else
    return FUNC_UNDEFINED;
}

void remove_line_breaks(char * string) {
  if( (string = strchr(string, '\n')) != NULL)
    *string = '\0';
}
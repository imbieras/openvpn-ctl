#ifndef HELPER_H
#define HELPER_H

#include <libubox/blobmsg_json.h>
#include <limits.h>
#include <stdio.h>

#define SOCKET_HOST "127.0.0.1"
#define SOCKET_PORT 7505
#define CLIENT_COUNT_MAX 16
#define BUFFER_SIZE 1024

struct client {
  char common_name[BUFFER_SIZE];
  char real_address[BUFFER_SIZE];
  char bytes_received[BUFFER_SIZE];
  char bytes_sent[BUFFER_SIZE];
  char connected_since[BUFFER_SIZE];
};

void signal_handler(int signal);
int connect_to_socket();
int send_text(const char *message);
int receive_text(char *buffer, int buffer_size);
void close_socket();
int client_blob_buf(struct client *clients, int count, struct blob_buf *b);
void delete_all_clients(struct client *clients, int *num_clients);

#endif // HELPER_H

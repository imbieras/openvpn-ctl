#include "helper.h"
#include "ubus_helper.h"
#include <arpa/inet.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int sockfd = -1;
extern pthread_t tid;
bool stop_loop;

void signal_handler(int signal) {
  if (signal == SIGTERM) {
    syslog(LOG_WARNING, "Signal received. Stopping.");
    stop_loop = true;

    pthread_cancel(tid);
    pthread_join(tid, NULL);

    close_socket();
  }
}

int connect_to_socket() {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    syslog(LOG_ERR, "socket: Failed to create a socket.");
    return EXIT_FAILURE;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SOCKET_PORT);
  if (inet_pton(AF_INET, SOCKET_HOST, &server_addr.sin_addr) <= 0) {
    syslog(LOG_ERR,
           "inet_pton: Invalid address or address family not supported.");
    close(sockfd);
    return EXIT_FAILURE;
  }

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    syslog(LOG_ERR, "connect: Failed to connect to the socket.");
    close(sockfd);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int send_text(const char *message) {
  int bytes_sent = send(sockfd, message, strlen(message), 0);
  if (bytes_sent < 0) {
    syslog(LOG_ERR, "send: Failed to send data to the socket.");
  }
  return bytes_sent;
}

int receive_text(char *buffer, int buffer_size) {
  int bytes_received = recv(sockfd, buffer, buffer_size - 1, 0);
  if (bytes_received < 0) {
    syslog(LOG_ERR, "recv: Failed to receive data from the socket.");
    buffer[0] = '\0';
  } else {
    buffer[bytes_received] = '\0';
  }
  return bytes_received;
}

void close_socket() {
  if (sockfd >= 0) {
    close(sockfd);
    sockfd = -1;
  }
}

int client_blob_buf(struct client *clients, int count, struct blob_buf *b) {
  void *array = blobmsg_open_array(b, "clients");

  for (int i = 0; i < count; i++) {
    void *table = blobmsg_open_table(b, clients[i].common_name);
    blobmsg_add_string(b, "common_name", clients[i].common_name);
    blobmsg_add_string(b, "real_address", clients[i].real_address);
    blobmsg_add_string(b, "bytes_received", clients[i].bytes_received);
    blobmsg_add_string(b, "bytes_sent", clients[i].bytes_sent);
    blobmsg_add_string(b, "connected_since", clients[i].connected_since);
    blobmsg_close_table(b, table);
  }
  blobmsg_close_array(b, array);

  return EXIT_SUCCESS;
}

static void delete_client(struct client *c) {
  memset(c->common_name, 0, sizeof(c->common_name));
  memset(c->real_address, 0, sizeof(c->real_address));
  memset(c->bytes_received, 0, sizeof(c->bytes_received));
  memset(c->bytes_sent, 0, sizeof(c->bytes_sent));
  memset(c->connected_since, 0, sizeof(c->connected_since));
}

void delete_all_clients(struct client *clients, int *num_clients) {
  for (int i = 0; i < *num_clients; i++) {
    delete_client(&clients[i]);
  }
  *num_clients = 0;
}

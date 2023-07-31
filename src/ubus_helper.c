#include "ubus_helper.h"
#include "helper.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

struct client clients[CLIENT_COUNT_MAX];
int num_clients = 0;
extern pthread_mutex_t mutex;
extern bool stop_loop;

void *fetch_data_thread(void *arg) {
  while (!stop_loop) {
    pthread_testcancel();

    send_text("status\n");

    char buffer[1024];
    int bytes_received = receive_text(buffer, sizeof(buffer));
    if (bytes_received <= 0) {
      syslog(LOG_ERR, "Failed to receive data from the socket.");
      return EXIT_FAILURE;
    }

    char *pos = strchr(buffer, '\n'); // Skip the next two lines
    for (int i = 0; i < 3; i++) {
      pos = strchr(pos, '\n');
      if (pos != NULL) {
        pos++; // Move past the newline character
      } else {
        syslog(LOG_ERR, "Malformed response from the socket.");
        return EXIT_FAILURE;
      }
    }

    delete_all_clients(clients, &num_clients);

    while ((pos != NULL) && (*pos != '\0')) {
      if (strncmp(pos, "ROUTING TABLE", strlen("ROUTING TABLE")) == 0) {
        break; // Stop parsing when "ROUTING TABLE" is encountered
      }

      struct client c;
      int n = sscanf(pos, "%63[^,],%63[^,],%63[^,],%63[^,],%63[^\r\n]",
                     c.common_name, c.real_address, c.bytes_received,
                     c.bytes_sent, c.connected_since);
      if ((n == 5) && (num_clients < CLIENT_COUNT_MAX)) {
        clients[num_clients] = c;
        num_clients = num_clients + 1;
      }

      pos = strchr(pos, '\n');
      if (pos != NULL) {
        pos++; // Move past the newline character
      }
    }

    if (num_clients > 0) {
      for (int i = 0; i < num_clients; i++) {
        struct client c = clients[i];
        syslog(LOG_INFO,
               "Client: {\"Common name\":\"%s\", "
               "\"Real address\":\"%s\", \"Bytes received\":\"%s\", \"Bytes "
               "sent\":\"%s\", \"Connected since\":\"%s\"}",
               c.common_name, c.real_address, c.bytes_received, c.bytes_sent,
               c.connected_since);
      }
    } else {
      syslog(LOG_INFO, "No connected clients found.");
    }

    sleep(10);
  }
  return NULL;
}

int ubus_init(struct ubus_context **ctx) {
  uloop_init();
  *ctx = ubus_connect(NULL);
  if (!(*ctx)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int ubus_deinit(struct ubus_context *ctx) {
  if (!ctx) {
    return EXIT_FAILURE;
  }
  ubus_free(ctx);
  uloop_done();
  return EXIT_SUCCESS;
}

int status(struct ubus_context *ctx, struct ubus_object *obj,
           struct ubus_request_data *req, const char *method,
           struct blob_attr *msg) {
  struct blob_buf b = {};
  blob_buf_init(&b, 0);

  pthread_mutex_lock(&mutex);

  if (num_clients < 1) {
    return UBUS_STATUS_NO_DATA;
  }

  client_blob_buf(clients, num_clients, &b);

  pthread_mutex_unlock(&mutex);

  ubus_send_reply(ctx, req, b.head);
  blob_buf_free(&b);

  return EXIT_SUCCESS;
}

int kill(struct ubus_context *ctx, struct ubus_object *obj,
         struct ubus_request_data *req, const char *method,
         struct blob_attr *msg) {
  struct blob_attr *tb[__REAL_ADDRESS_MAX];
  struct blob_buf b = {};
  blob_buf_init(&b, 0);
  blobmsg_parse(control_policy, __REAL_ADDRESS_MAX, tb, blob_data(msg),
                blob_len(msg));

  if (!tb[REAL_ADDRESS])
    return UBUS_STATUS_INVALID_ARGUMENT;

  char *real_address = blobmsg_get_string(tb[REAL_ADDRESS]);

  char command[BUFFER_SIZE];
  snprintf(command, sizeof(command), "kill %s\n", real_address);

  send_text(command);

  char buffer[BUFFER_SIZE];
  int bytes_received = receive_text(buffer, sizeof(buffer));
  if (bytes_received <= 0) {
    syslog(LOG_ERR, "Failed to receive data from the socket.");
    return EXIT_FAILURE;
  }

  blobmsg_add_string(&b, "response", buffer);
  ubus_send_reply(ctx, req, b.head);
  blob_buf_free(&b);

  return EXIT_SUCCESS;
}

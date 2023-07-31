#include "helper.h"
#include "ubus_helper.h"
#include <libubus.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

pthread_t tid;
pthread_mutex_t mutex;

static struct ubus_object control_object = {.name = "openvpn-ctl",
                                            .type = &control_object_type,
                                            .methods = control_methods,
                                            .n_methods =
                                                ARRAY_SIZE(control_methods)};

int main(int argc, char **argv) {
  struct ubus_context *ctx = NULL;

  openlog("openvpn-ctl", LOG_PID, LOG_DAEMON);

  if (connect_to_socket() < 0) {
    syslog(LOG_ERR, "Failed to connect to the socket.");
    closelog();
    return EXIT_FAILURE;
  }

  signal(SIGTERM, signal_handler);

  char buffer[1024];
  int bytes_received = receive_text(buffer, sizeof(buffer));
  if (bytes_received <= 0) {
    printf(LOG_ERR, "Failed to receive data from the socket.");
    return -1;
  }

  pthread_mutex_init(&mutex, NULL);

  if (pthread_create(&tid, NULL, fetch_data_thread, NULL) != 0) {
    syslog(LOG_ERR, "Failed to connect to fetch_data_thread");
    return EXIT_FAILURE;
  }

  if (ubus_init(&ctx) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  ubus_add_uloop(ctx);
  ubus_add_object(ctx, &control_object);
  uloop_run();

  pthread_mutex_destroy(&mutex);

  ubus_deinit(ctx);

  closelog();
  return EXIT_SUCCESS;
}

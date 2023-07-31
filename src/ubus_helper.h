#ifndef UBUS_HELPER_H
#define UBUS_HELPER_H

#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

enum {
  REAL_ADDRESS,
  __REAL_ADDRESS_MAX,
};

void *fetch_data_thread(void *arg);
int ubus_init(struct ubus_context **ctx);
int ubus_deinit(struct ubus_context *ctx);
int status(struct ubus_context *ctx, struct ubus_object *obj,
           struct ubus_request_data *req, const char *method,
           struct blob_attr *msg);
int kill(struct ubus_context *ctx, struct ubus_object *obj,
         struct ubus_request_data *req, const char *method,
         struct blob_attr *msg);

static const struct blobmsg_policy control_policy[] = {
    [REAL_ADDRESS] = {.name = "real_address", .type = BLOBMSG_TYPE_STRING}};

static const struct ubus_method control_methods[] = {
    UBUS_METHOD_NOARG("status", status),
    UBUS_METHOD("kill", kill, control_policy)};

static struct ubus_object_type control_object_type =
    UBUS_OBJECT_TYPE("openvpn-ctl", control_methods);

#endif // UBUS_HELPER_H

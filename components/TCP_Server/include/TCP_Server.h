#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include <sys/param.h>
#include "esp_netif.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "lwip/err.h"
#include "lwip/sys.h"

#include "AT_CMD_Processing.h"

#define PORT                        3333//CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              5//CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          5//CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             3//CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *SERVER_TAG = "TCP Server";

void tcp_server_task();

#endif
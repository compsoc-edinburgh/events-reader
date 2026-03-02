#include <stdio.h>

#include "pico/cyw43_arch.h"

#include "src/networking/networking.h"
#include "src/sched/sched.h"

#define MAX_WIFI_CONNECT_RETRIES 5
#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAX_PASS_LEN 64

// Note, this should only ever be used internally, so this won't be in the
// networking.h header
typedef struct {
    char username[WIFI_MAX_SSID_LEN];
    char password[WIFI_MAX_PASS_LEN];
} wifi_creds_t;

typedef enum {
    WIFI_STATE_INIT = 0,
    WIFI_STATE_CONNECT,
    WIFI_STATE_CLEANUP,
    WIFI_STATE_FATAL_NO_CREDS,
    WIFI_STATE_FATAL_MAX_RETRIES
} wifi_state_t;

typedef struct {
    wifi_state_t state;
    uint8_t retries;
    wifi_creds_t creds;
} wifi_connect_ctx_t;


void wifi_connect_task(void* arg) {
    wifi_connect_ctx_t* ctx = (wifi_connect_ctx_t*) arg;
    switch (ctx->state) {
        case WIFI_STATE_INIT:
            // Zeroes memory in ctx struct
            // Queues storage task with callback to next stage
            break;
        case WIFI_STATE_CONNECT:
            // Connect to wifi
            break;
        case WIFI_STATE_CLEANUP:
            // Free creds struct
            break;
        case WIFI_STATE_FATAL_NO_CREDS:
            // Throw fatal error, no wifi creds found.
            break;
        case WIFI_STATE_FATAL_MAX_RETRIES:
            // Throw fatal error, failed to connect to wifi after
            // MAX_WIFI_CONNECT_RETRIES retries
            break;
        default:
            break;
    }
}

NETWORK_RESPONSE_CODE init_networking() {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed!\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();  // enable station mode

    if (enqueue_task(wifi_connect_task, (void*) 0))
        return PENDING;
    return FAILURE;
}

NETWORK_RESPONSE_CODE connect_to_wifi(wifi_creds_t* creds) {
    if (cyw43_arch_wifi_connect_timeout_ms(creds->username, creds->password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connected!\n");
    } else {
        printf("Failed to connect.\n");
    }
}

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
            // Queues storage task with callback to next stage
            puts("WIFI_STATE_INIT");
            ctx->state = WIFI_STATE_CONNECT;
            enqueue_task(wifi_connect_task, arg);
            break;
        case WIFI_STATE_CONNECT:
            // Connect to wifi
            puts("WIFI_STATE_CONNECT");
            ctx->state = WIFI_STATE_CLEANUP;
            enqueue_task(wifi_connect_task, arg);
            break;
        case WIFI_STATE_FATAL_NO_CREDS:
            // Log error
            goto cleanup;
        case WIFI_STATE_FATAL_MAX_RETRIES:
            // Log error
            goto cleanup;
        case WIFI_STATE_CLEANUP:
            puts("WIFI_STATE_CLEANUP");
            cleanup:
            scfree(ctx);
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

    wifi_connect_ctx_t* ctx = szalloc(sizeof(wifi_connect_ctx_t));
    ctx->state = WIFI_STATE_INIT;
    if (enqueue_task(wifi_connect_task, (void*) ctx))
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

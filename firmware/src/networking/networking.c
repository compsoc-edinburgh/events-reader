#include "networking/networking.h"

#include <stdio.h>
#include "pico/cyw43_arch.h"

typedef struct {
    char* username;
    char* password;
} wifi_creds_t;

NETWORK_RESPONSE_CODE init_networking() {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed!\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();  // enable station mode
}

NETWORK_RESPONSE_CODE connect_to_wifi(wifi_creds_t* creds) {
    if (cyw43_arch_wifi_connect_timeout_ms(creds->username, creds->password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connected!\n");
    } else {
        printf("Failed to connect.\n");
    }
}

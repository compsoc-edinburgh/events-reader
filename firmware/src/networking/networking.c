#include <stdio.h>

#include "pico/cyw43_arch.h"

void connect_to_wifi(char* username, char* password) {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed!\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();  // enable station mode

    // Connect to Wi-Fi
    if (cyw43_arch_wifi_connect_timeout_ms(username, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connected!\n");
    } else {
        printf("Failed to connect.\n");
    }
}

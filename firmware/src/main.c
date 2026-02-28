#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed!\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();  // enable station mode

    // Connect to Wi-Fi
    if (cyw43_arch_wifi_connect_timeout_ms("ssid", "password", CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connected!\n");
    } else {
        printf("Failed to connect.\n");
    }

    while (true) {
        puts("Hello, world!");
        sleep_ms(100);
    }
}

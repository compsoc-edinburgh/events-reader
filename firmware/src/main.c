#include <stdio.h>

#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (true) {
        puts("Hello, world!");
        sleep_ms(100);
    }
}

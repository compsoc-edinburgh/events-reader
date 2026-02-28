#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();
    while (true) {
        gpio_put(25, 1);
        sleep_ms(500);
        gpio_put(25, 0);
        sleep_ms(500);
    }
}

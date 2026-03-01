#include <stdio.h>

#include "pico/stdlib.h"

#include "src/sched/sched.h"
// #include "networking/networking.h"
// #include "storage/storage.h"
// #include "nfc/nfc.h"
// #include "domain/domain.h"

int main(void) {
    stdio_init_all();

    init_sched();

    // TODOs:
    //  - Init storage
    //  - Init networking
    //  - Init nfc chip
    //  - main loop

    while (true) {
        enqueue_task(print_task, "Hello from core 1!");
        puts("Hello from core 0!");
        sleep_ms(100);
    }
}

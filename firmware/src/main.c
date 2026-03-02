#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "src/sched/sched.h"
#include "src/networking/networking.h"
// #include "storage/storage.h"
// #include "nfc/nfc.h"
// #include "domain/domain.h"

int main(void) {
    stdio_init_all();

    init_sched();
    init_networking();

    // TODOs:
    //  - Init storage
    //  - Init networking
    //  - Init nfc chip
    //  - main loop

    const char str[18] = "I'm SLAB Allocated";
    while (true) {
        char* s = szalloc(sizeof(str));
        strcpy(s, str);
        enqueue_task(print_task, s);
        sleep_ms(100);
    }
}

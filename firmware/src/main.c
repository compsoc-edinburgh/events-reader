#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#define TASK_QUEUE_SIZE 16

typedef void (*task_fn_t)(void*);

typedef struct {
    task_fn_t fn;
    void* arg;
} task_t;

typedef struct {
    task_t tasks[TASK_QUEUE_SIZE];
    volatile int head;
    volatile int tail;
} task_queue_t;

typedef struct {
    volatile uint32_t locked;
} bw_mutex_t;

static task_queue_t task_queue = {0};

static bw_mutex_t enqueue_task_mutex = {0};

void mutex_lock(bw_mutex_t* m) {
    while (__atomic_test_and_set(&m->locked, __ATOMIC_ACQUIRE)) {
        // Busy wait - yield to other core by inserting a no-op
        __asm volatile ("nop" ::: "memory");
    }
}

void mutex_unlock(bw_mutex_t* m) {
    __atomic_clear(&m->locked, __ATOMIC_RELEASE);
}

bool enqueue_task(task_fn_t fn, void* arg) {
    mutex_lock(&enqueue_task_mutex);

    int current_tail = task_queue.tail;
    int next_tail = (current_tail + 1) % TASK_QUEUE_SIZE;

    // Acquire load so we see core1's latest head update
    if (next_tail == __atomic_load_n(&task_queue.head, __ATOMIC_ACQUIRE)) {
        mutex_unlock(&enqueue_task_mutex);
        return false; // Queue full
    }

    task_queue.tasks[current_tail].fn = fn;
    task_queue.tasks[current_tail].arg = arg;

    // Release store publishes the slot to core1
    __atomic_store_n(&task_queue.tail, next_tail, __ATOMIC_RELEASE);

    mutex_unlock(&enqueue_task_mutex);

    return true;
}

void core1_worker() {
    while (1) {
        // Acquire load so we see core0's latest tail update
        int tail = __atomic_load_n(&task_queue.tail, __ATOMIC_ACQUIRE);
        int head = __atomic_load_n(&task_queue.head, __ATOMIC_RELAXED);

        if (head != tail) {
            task_t t = task_queue.tasks[head];
            int next_head = (head + 1) % TASK_QUEUE_SIZE;

            // Release store publishes the updated head to core0
            __atomic_store_n(&task_queue.head, next_head, __ATOMIC_RELEASE);

            if (t.fn) t.fn(t.arg);
        } else {
            sleep_ms(1);
        }
    }
}

// Example task
void print_task(void* arg) {
    const char* msg = (const char*)arg;
    printf("Task says: %s\n", msg);
}

int main(void) {
    stdio_init_all();

    multicore_launch_core1(core1_worker);

    while (true) {
        enqueue_task(print_task, "Hello from core 1!");
        puts("Hello from core 0!");
        sleep_ms(100);
    }
}

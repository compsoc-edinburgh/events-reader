#include <stdio.h>
#include <string.h>

#include "pico/multicore.h"

#include "src/sched/sched.h"


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

void print_task(void* arg) {
    const char* msg = (const char*)arg;
    printf("Task says: %s\n", msg);
    scfree(arg);
}


static const size_t pool_chunk_sizes[] = SLAB_POOL_SIZES;
#define NUM_POOLS  (sizeof(pool_chunk_sizes) / sizeof(pool_chunk_sizes[0]))
#define BLOCK_SIZE(chunk)  (sizeof(slab_header_t) + (chunk))
static uint8_t pool_storage[NUM_POOLS][SLAB_POOL_CAPACITY][BLOCK_SIZE(SLAB_MAX_CHUNK_SIZE)];
static slab_block_generic_t* free_list[NUM_POOLS];
static bw_mutex_t slab_mutex = {0};
#define S_ISFREE 1

void slab_init(void) {
    for (size_t i = 0; i < NUM_POOLS; i++) {
        free_list[i] = NULL;

        for (int j = 0; j < SLAB_POOL_CAPACITY; j++) {
            slab_block_generic_t* block = (slab_block_generic_t*)pool_storage[i][j];

            block->header.index = (uint8_t)i;
            block->header.flags = 0;
            block->header.padding[0] = 0;
            block->header.padding[1] = 0;

            // Thread chunk area as a next-pointer for the free list.
            slab_block_generic_t** next_slot = (slab_block_generic_t**)block->chunk;
            *next_slot = free_list[i];
            free_list[i] = block;
        }
    }
}

slab_block_generic_t* salloc(size_t size) {
    // Find the smallest pool whose chunk size fits `size`.
    size_t pool_idx = NUM_POOLS; // sentinel: not found
    for (size_t i = 0; i < NUM_POOLS; i++) {
        if (pool_chunk_sizes[i] >= size) {
            pool_idx = i;
            break;
        }
    }

    if (pool_idx == NUM_POOLS) {
        // Requested size exceeds all pools.
        return NULL;
    }

    mutex_lock(&slab_mutex);

    slab_block_generic_t* block = free_list[pool_idx];
    if (block != NULL) {
        // Pop from free list.
        slab_block_generic_t** next_slot = (slab_block_generic_t**)block->chunk;
        free_list[pool_idx] = *next_slot;

        // Unset freed flag
        block->header.flags &= ~S_ISFREE;
    }

    mutex_unlock(&slab_mutex);

    return block; // NULL if pool was exhausted
}

void sfree(slab_block_generic_t* block) {
    if (block == NULL) return;

    size_t pool_idx = block->header.index;

    if (pool_idx >= NUM_POOLS) return;

    mutex_lock(&slab_mutex);

    if (block->header.flags & S_ISFREE) {
        mutex_unlock(&slab_mutex);
        return;
    }

    // Marks as freed
    block->header.flags |= S_ISFREE;

    // Push back onto free list.
    slab_block_generic_t** next_slot = (slab_block_generic_t**)block->chunk;
    *next_slot = free_list[pool_idx];
    free_list[pool_idx] = block;

    mutex_unlock(&slab_mutex);
}

void* szalloc(size_t size) {
    slab_block_generic_t* block = salloc(size);
    if (block == NULL) return NULL;

    // Zero only the chunk area (not the header).
    // Use the actual pool's chunk size so the caller gets a fully zeroed buffer.
    size_t pool_idx = block->header.index;
    memset(block->chunk, 0, pool_chunk_sizes[pool_idx]);

    return (void*)block->chunk;
}

void scfree(void* chunk) {
    if (chunk == NULL) return;

    // Walk back from chunk to the containing slab_block_generic_t.
    slab_block_generic_t* block = (slab_block_generic_t*)(
        (uint8_t*)chunk - offsetof(slab_block_generic_t, chunk)
    );

    sfree(block);
}

void init_sched() {
    slab_init();
    multicore_launch_core1(core1_worker);
}

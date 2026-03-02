#ifndef SCHED_H
#define SCHED_H

/*
* ==== Scheduler ====
*/

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

bool enqueue_task(task_fn_t fn, void* arg);
void init_sched();

// Mainly for debugging purposes, just prints the argument given
void print_task(void* arg);

/*
* ==== Locks ====
*/

typedef struct {
    uint32_t locked;
} bw_mutex_t;

void mutex_lock(bw_mutex_t* m);
void mutex_unlock(bw_mutex_t* m);

/*
* ==== SLAB Allocator ====
*/

// Chunk sizes (bytes) for each pool, in ascending order.
#define SLAB_POOL_SIZES  { 8, 16, 32, 64, 128, 256 }
#define SLAB_MAX_CHUNK_SIZE    256

// Number of blocks per pool.
#define SLAB_POOL_CAPACITY 16

typedef struct {
    uint8_t padding[2]; // Padding for byte alignment.
    uint8_t flags;
    uint8_t index;
} slab_header_t;
typedef struct {
    slab_header_t header;
    uint8_t chunk[];
} slab_block_generic_t;

// These work with metadata pointers, you probably don't
// want to use them.
slab_block_generic_t* salloc(size_t size);
void sfree(slab_block_generic_t* block);

// You should almost certainly use these functions
void* szalloc(size_t size); // Returns a zeroed pointer to the chunk itself
void scfree(void* chunk); // Frees from the chunk itself

#endif

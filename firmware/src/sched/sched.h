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

void mutex_lock(bw_mutex_t* m);
void mutex_unlock(bw_mutex_t* m);
bool enqueue_task(task_fn_t fn, void* arg);
void init_sched();

void print_task(void* arg);

#include "ku_queue.h"

#define MAX_QUEUE 128

static job_t queue[MAX_QUEUE];
static int front = 0, rear = 0;

void ku_queue_init(void) {
    front = rear = 0;
}

int ku_queue_is_empty(void) {
    return front == rear;
}

void ku_queue_push(job_t job) {
    queue[rear++] = job;
    if (rear == MAX_QUEUE) rear = 0;
}

job_t ku_queue_pop(void) {
    job_t ret = queue[front++];
    if (front == MAX_QUEUE) front = 0;
    return ret;
}

int ku_queue_is_pid_in_queue(pid_t pid) {
    int i;
    for (i = front; i != rear; i = (i + 1) % MAX_QUEUE) {
        if (queue[i].pid == pid) return 1;
    }
    return 0;
}


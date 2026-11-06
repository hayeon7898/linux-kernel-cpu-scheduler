#ifndef KU_QUEUE_H
#define KU_QUEUE_H

#include <linux/types.h>

typedef struct {
    pid_t pid;
    int job_time;
} job_t;

void ku_queue_init(void);
int ku_queue_is_empty(void);
void ku_queue_push(job_t job);
job_t ku_queue_pop(void);
int ku_queue_is_pid_in_queue(pid_t pid);

#endif


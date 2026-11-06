#ifndef __FCFS_H__
#define __FCFS_H__

#include <linux/types.h>

typedef struct job {
    pid_t pid;
    int job_time;
} job_t;

void fcfs_init(void);
int fcfs_handle_request(pid_t pid, const char* name, int job_time); 
#endif


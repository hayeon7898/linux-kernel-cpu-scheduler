#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>     
#include <linux/slab.h>
#include <linux/string.h>

#include "fcfs.h"
#include "srtf.h"   
#include "rr.h" 
#include "pp.h"

// 사용자 정의 syscall: 현재 프로세스의 PID를 확인하는 용도 (예시용)
SYSCALL_DEFINE1(os2024_pid_print, char*, name) {
    pid_t pid = current->pid;  // 현재 실행 중인 프로세스의 PID 가져오기
    return 0;  // 실제 출력은 하지 않음
}

// 메인 스케줄링 syscall
SYSCALL_DEFINE4(os2024_ku_cpu, char __user * , name, int , job_time, int , policy, int , priority) {
    pid_t pid = current->pid;  // 현재 프로세스의 PID
    int ret;

    switch (policy) {
        case 0:
            // FCFS 스케줄러 처리 요청
            ret = fcfs_handle_request(pid, name, job_time);
            break;
        case 1:
            // SRTF 스케줄러 처리 요청
            ret = srtf_schedule(name, job_time);
            break;
        case 2:
            // 라운드 로빈(RR) 스케줄러 처리 요청
            ret = rr_schedule(name, job_time);
             if(ret==0 && job_time>0)
                printk(KERN_INFO "Working: %s\n", name);
             else if(ret==2){
               printk(KERN_INFO "Process Finished: %s\n", name);
               ret = 0;
             }
             return ret;
        case 3:
            // 우선순위 기반 선점 스케줄러 처리 요청
            ret = priority_preemptive_schedule(name, job_time, priority);
            break;
        default:
            // 잘못된 정책 번호 입력 시 에러 반환
            printk(KERN_INFO "Unknown scheduling policy: %d\n", policy);
            return -EINVAL;
    }

    // 실행 허용 여부에 따라 커널 메시지 출력
    if (ret == 0)
        printk(KERN_INFO "Working: %s\n", name);  // 요청된 프로세스 실행 허용
    else if (ret == 1)
        printk(KERN_INFO "Working Denied: %s\n", name);  // 요청된 프로세스 실행 거부

    return ret;
}


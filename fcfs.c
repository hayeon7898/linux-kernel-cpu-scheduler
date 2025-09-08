#include "ku_queue.h"
#include <linux/printk.h>
#include <linux/spinlock.h>

#define IDLE -1

static pid_t current_pid = IDLE;          // 현재 실행 중인 프로세스 PID, 없으면 IDLE
static spinlock_t fcfs_lock;              // 동기화용 spinlock 선언

// FCFS 스케줄러 초기화 함수
void fcfs_init(void) {
    ku_queue_init();                      // 내부 큐 초기화
    current_pid = IDLE;                   // 실행 중인 프로세스 없음 상태 초기화
    spin_lock_init(&fcfs_lock);          // spinlock 초기화
}

/**
 * fcfs_handle_request - FCFS 스케줄링 처리 함수
 * @pid: 요청 프로세스 PID
 * @name: 프로세스 이름 (현재 사용 안 함)
 * @job_time: 남은 작업 시간, 0이면 작업 종료 요청
 *
 * 동작:
 * 1) 현재 CPU가 비어있으면 요청한 프로세스를 바로 실행.
 * 2) 현재 실행 중인 프로세스와 요청자가 같으면 작업 시간 갱신,
 *    작업이 끝나면 큐에서 다음 프로세스를 꺼내 실행.
 * 3) 그 외의 경우는 요청을 큐에 추가하고 대기.
 *
 * 락으로 동기화 처리하여 race condition 방지.
 */
int fcfs_handle_request(pid_t pid, const char* name, int job_time) {
    job_t new_job = {pid, job_time};
    int ret = 1;

    spin_lock(&fcfs_lock);                // 임계 구역 진입

    if (current_pid == IDLE) {            // CPU가 비어있으면
        current_pid = pid;                 // 바로 실행 허용
        ret = 0;
    } else if (current_pid == pid) {      // 실행 중인 프로세스가 요청한 경우
        if (job_time == 0) {               // 작업 종료 요청이면
            if (ku_queue_is_empty()) {    // 큐가 비었으면
                current_pid = IDLE;       // CPU 비움
            } else {                      // 큐에 작업 있으면
                job_t next = ku_queue_pop();  // 다음 작업 꺼내 실행
                current_pid = next.pid;
            }
        }
        ret = 0;                         // 현재 실행 중인 프로세스는 계속 실행 허용
    } else {                            // 다른 프로세스가 실행 중이면
        ku_queue_push(new_job);         // 요청 큐에 추가
        ret = 1;                      // 실행 대기
    }

    spin_unlock(&fcfs_lock);            // 임계 구역 종료
    return ret;
}


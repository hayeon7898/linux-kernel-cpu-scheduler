#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sched.h>
#include "pp.h"

#define MAX_TASKS 3  // 등록 가능한 최대 태스크 수

/**
 * prio_task_t - 우선순위 스케줄링을 위한 태스크 구조체
 * @pid: 태스크의 프로세스 ID
 * @name: 태스크 이름 (최대 31자 + 널문자)
 * @job_time: 남은 실행 시간
 * @priority: 우선순위 값 (작을수록 높은 우선순위)
 * @state: 태스크 상태 (0: 실행 중, 1: 대기 중, 2: 완료됨)
 */
typedef struct {
    pid_t pid;
    char name[32];
    int job_time;
    int priority;
    int state;  // 0: running, 1: denied(waiting), 2: done
} prio_task_t;

// 현재 등록된 태스크 목록
static prio_task_t task_list[MAX_TASKS];

// 현재 등록된 태스크 수
static int task_count = 0;

// 현재 실행 중인 태스크
static prio_task_t* currT = NULL;

/**
 * select_next_task - 다음 실행할 태스크를 선택하는 함수
 *
 * 현재 실행 중인 태스크가 완료되었는지 확인하고,
 * 우선순위가 가장 높은 denied 태스크를 선택하여 실행 상태로 전환한다.
 */
void select_next_task(void) {
    int i;

    // 현재 실행 중인 태스크가 완료되었는지 확인
    if (currT && currT->job_time <= 0) {
        currT->state = 2;  // 완료 상태로 변경
        currT = NULL;
    }

    // 실행 중인 태스크가 없으면 새로운 태스크 선택
    if (!currT) {
        prio_task_t* highest = NULL;
        for (i = 0; i < task_count; i++) {
            if (task_list[i].state == 1) {  // 대기 중인 태스크만 고려
                if (!highest || task_list[i].priority < highest->priority) {
                    highest = &task_list[i];
                }
            }
        }
        // 우선순위가 가장 높은 태스크 실행
        if (highest) {
            currT = highest;
            currT->state = 0;  // 실행 상태로 전환
        }
    }
}

/**
 * cleanup_if_all_done - 모든 태스크가 완료되었을 경우 태스크 리스트 초기화
 *
 * 모든 태스크의 상태가 2 (done)이면 task_list를 0으로 초기화하고,
 * task_count와 currT도 리셋한다.
 */
void cleanup_if_all_done(void) {
    int i;
    int all_done = 1;

    for (i = 0; i < task_count; i++) {
        if (task_list[i].state != 2) {
            all_done = 0;
            break;
        }
    }

    if (all_done) {
        memset(task_list, 0, sizeof(task_list));
        task_count = 0;
        currT = NULL;
        //printk(KERN_INFO "[PRIO] All tasks done. Task list cleared.\n");
    }
}

/**
 * priority_preemptive_schedule - 우선순위 기반 선점 스케줄링 진입 함수
 *
 * @name: 등록할 태스크 이름
 * @job_time: 실행 시간
 * @priority: 우선순위 (작을수록 높음)
 * 
 * 새로운 태스크를 등록하고, 선점 조건에 따라 현재 실행 중인 태스크를 대기로 돌리고
 * 새 태스크를 실행 상태로 등록하거나, 대기 상태로 등록한다.
 *
 * Return:
 *   0 - 실행 중
 *   1 - 대기 중
 *   2 - 이미 완료된 태스크
 *  -ENOMEM - 태스크 리스트가 가득 찬 경우
 */
int priority_preemptive_schedule(char* name, int job_time, int priority) {
    pid_t pid = current->pid;
    int i;

    // 우선 기존 태스크 완료 여부 확인 및 다음 태스크 선택
    select_next_task();
    cleanup_if_all_done();

    // 이미 같은 이름의 태스크가 있으면 그 상태만 반환
    for (i = 0; i < task_count; i++) {
        if (strcmp(task_list[i].name, name) == 0) {
            if (task_list[i].state == 2) {
                return 2;  // 이미 완료된 태스크
            }
            task_list[i].job_time = job_time;  // job_time 업데이트
            return task_list[i].state;  // 기존 태스크 상태 반환
        }
    }

    // 태스크 리스트가 가득 찬 경우
    if (task_count >= MAX_TASKS) {
        return -ENOMEM;
    }

    // 새로운 태스크 생성
    prio_task_t new_task;
    new_task.pid = pid;
    strncpy(new_task.name, name, sizeof(new_task.name) - 1);
    new_task.name[sizeof(new_task.name) - 1] = '\0';  // 널 종료 보장
    new_task.job_time = job_time;
    new_task.priority = priority;

    // 현재 태스크보다 우선순위가 높으면 선점
    if (!currT || new_task.priority < currT->priority) {
        if (currT && currT->state == 0) {
            currT->state = 1;  // 실행 중이던 태스크를 대기로 전환
        }
        new_task.state = 0;  // 실행 상태로 등록
        currT = &task_list[task_count];  // 현재 실행 태스크 설정
    } else {
        new_task.state = 1;  // 대기 상태로 등록
    }

    // 태스크 리스트에 추가
    task_list[task_count++] = new_task;
    return new_task.state;
}


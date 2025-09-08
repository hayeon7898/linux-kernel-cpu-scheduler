#include <linux/kernel.h>
#include <linux/string.h>
#include "rr.h"

#define MAX_PROC 3         // 등록 가능한 최대 프로세스 수
#define TIME_QUANTUM 10    // 각 프로세스가 한 번에 실행할 수 있는 최대 시간(ms)

// 프로세스 상태 열거형
enum {
    STATE_RUNNING = 0,     // 실행 중
    STATE_READY = 1,       // 대기 중
    STATE_FINISHED = 2     // 실행 완료
};

/**
 * rr_proc - Round Robin 프로세스 구조체
 * @name: 프로세스 이름
 * @remaining_time: 남은 실행 시간
 * @state: 현재 상태 (실행중, 대기중, 완료됨)
 */
typedef struct {
    char name[32];
    int remaining_time;
    int state;
} rr_proc;


static rr_proc rr_queue[MAX_PROC];  // Round Robin 큐 배열
static int queue_size = 0;          // 현재 큐에 있는 프로세스 수

/**
 * init_rr_scheduler - RR 큐 초기화 함수
 *
 * 모든 큐 엔트리를 STATE_FINISHED로 설정하고 큐 크기를 0으로 리셋한다.
 */
void init_rr_scheduler(void) {
    queue_size = 0;
    int i;
    for (i = 0; i < MAX_PROC; i++) {
        rr_queue[i].state = STATE_FINISHED;
    }
}

/**
 * shift_queue - 큐의 앞쪽 요소를 제거하고 뒤의 요소들을 앞으로 당김
 *
 * 현재 실행된 프로세스가 끝났거나 대기로 전환될 때 호출되어
 * FIFO 방식의 큐 유지.
 */
void shift_queue(void) {
    int i;
    //printk(KERN_INFO "[SHIFT] Before shift, queue_size: %d\n", queue_size);
    for (i = 1; i < queue_size; i++) {
        rr_queue[i - 1] = rr_queue[i];
    }
    queue_size--;
}


/**
 * add_process - 새로운 프로세스를 RR 큐에 추가
 *
 * @name: 프로세스 이름
 * @job_time: 프로세스 실행 시간
 *
 * 중복 이름 또는 job_time이 0 이하일 경우 추가하지 않음.
 *
 * Return:
 *   1 - 성공적으로 추가됨
 *   0 - 중복 이름, 추가 안 됨
 *  -1 - 큐가 가득 찼거나 잘못된 job_time
 */
int add_process(const char *name, int job_time) {
    int i;

    if (queue_size > MAX_PROC || job_time <= 0) {
        return -1;
    }

    // 중복 이름 확인
    for (i = 0; i < queue_size; i++) {
        if (strcmp(rr_queue[i].name, name) == 0) {
            return 0;
        }
    }

    // 프로세스 추가
    strcpy(rr_queue[queue_size].name, name);
    rr_queue[queue_size].remaining_time = job_time;
    rr_queue[queue_size].state = STATE_READY;
    queue_size++;
    //printk(KERN_INFO "[ADD] After add, queue_size: %d\n", queue_size);
    return 1;
}

/**
 * rr_schedule - Round Robin 스케줄링 수행 함수
 *
 * @name: 실행 요청한 프로세스 이름
 * @job_time: 현재 프로세스에 남은 실행 시간
 *
 * 1. 새로운 프로세스가 있으면 추가
 * 2. 현재 큐의 맨 앞에 있는 프로세스만 실행 가능
 * 3. TIME_QUANTUM만큼 실행 후 상태 및 큐 업데이트
 *
 * Return:
 *   0 - 프로세스 실행 완료됨
 *   1 - 요청한 프로세스가 대기열에 있으나 실행 위치가 아님
 *  -1 - 실행 불가 (큐 없음 또는 잘못된 요청)
 */
long rr_schedule(const char *name, int job_time) {
    int process_quantum;
    int add_process_state;

    // 1. 프로세스 추가
    add_process_state = add_process(name, job_time);
    if (queue_size == 0) return -1;

    // 2. 큐 맨 앞에 있는 프로세스만 실행 가능
    rr_proc *current = &rr_queue[0];
    if (strcmp(current->name, name) != 0) {
        return 1;
    }

    // 3. 실행 시작
    current->state = STATE_RUNNING;
    int exec_time = (current->remaining_time > TIME_QUANTUM) ? TIME_QUANTUM : current->remaining_time;
    process_quantum = current->remaining_time - exec_time;

    // 4. 실행 결과 처리
    if (process_quantum == job_time) {

        // 실행 종료 조건
        if (job_time == 0) {
            current->state = STATE_FINISHED;
            current->remaining_time = 0;
            //printk(KERN_INFO "Process Finished: %s", name);
            shift_queue();
            return 2;
        }
        // 실행했지만 남은 시간 있음 → 대기로 전환, 뒤로 이동
        current->state = STATE_READY;
        current->remaining_time = job_time;
        printk(KERN_INFO "Turn Over----> %s", current->name);
        shift_queue();
    }

    return current->state;
}






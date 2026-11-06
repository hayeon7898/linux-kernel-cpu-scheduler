#include <linux/slab.h>       // kmalloc, kfree 등 메모리 할당 관련
#include <linux/string.h>     // strcmp, strncpy 등 문자열 함수
#include "srtf.h"             // 헤더 파일 선언

// 프로세스 제어 블록 (Process Control Block)
struct ku_pcb {
    char name[16];            // 프로세스 이름
    int remaining_time;       // 남은 실행 시간 (tick 단위)
    int state;                // 프로세스 상태 (0: 준비, 1: 실행, 2: 종료)
    struct list_head list;    // 커널 리스트 연결용 구조체
};

// SRTF 스케줄링을 위한 프로세스 큐 선언 (전역)
static LIST_HEAD(ku_queue);

/**
 * find_or_create_process - 이름으로 프로세스를 찾거나 새로 생성
 * @name: 프로세스 이름
 *
 * 리스트에서 같은 이름의 프로세스가 존재하면 반환하고,
 * 없으면 새로 동적할당하여 생성 후 리스트에 추가한다.
 */
static struct ku_pcb* find_or_create_process(const char *name) {
    struct ku_pcb *p;

    list_for_each_entry(p, &ku_queue, list) {
        if (strcmp(p->name, name) == 0)
            return p;  // 기존 프로세스 반환
    }

    // 새로운 프로세스 생성
    p = kmalloc(sizeof(struct ku_pcb), GFP_KERNEL);
    if (!p) return NULL;

    strncpy(p->name, name, 16);
    p->remaining_time = 0;
    p->state = 0; // 초기 상태는 준비 (ready)
    list_add_tail(&p->list, &ku_queue);  // 큐 끝에 추가

    return p;
}

/**
 * select_shortest_remaining - 남은 시간이 가장 짧은 프로세스 선택
 *
 * 아직 종료되지 않은 프로세스 중에서 남은 시간이 가장 짧은 프로세스를 반환한다.
 */
static struct ku_pcb* select_shortest_remaining(void) {
    struct ku_pcb *p, *shortest;
    p = NULL;
    shortest = NULL;

    list_for_each_entry(p, &ku_queue, list) {
        if (p->state != 2) { // 종료되지 않은 프로세스만 고려
            if (!shortest || p->remaining_time < shortest->remaining_time)
                shortest = p;
        }
    }

    return shortest;
}

/**
 * srtf_schedule - SRTF 스케줄링 처리 함수 (핸들러에서 호출)
 * @name: 프로세스 이름
 * @remain: 남은 실행 시간. 0이면 종료 요청
 *
 * 호출된 프로세스가 현재 가장 짧은 남은 시간을 가진 경우 0 반환 (CPU 허용),
 * 그렇지 않으면 1 반환 (대기).
 */
long srtf_schedule(const char *name, int remain) {
    struct ku_pcb *proc = find_or_create_process(name);
    struct ku_pcb *shortest = NULL;

    if (!proc) return -ENOMEM;

    // 종료 요청: 상태를 finished로 바꿈
    if (remain == 0) {
        proc->state = 2;
        return 0;
    }

    // 남은 시간 갱신
    proc->remaining_time = remain;

    // 현재 가장 짧은 프로세스를 선택
    shortest = select_shortest_remaining();

    // 자신이 가장 짧은 경우 -> CPU 사용 허용
    if (shortest && strcmp(shortest->name, name) == 0) {
        proc->remaining_time--;
        if (proc->remaining_time == 0) proc->state = 2;
        return 0;  // 허용
    }

    return 1;  // 거부 (대기)
}


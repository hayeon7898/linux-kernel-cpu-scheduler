#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>

#define KU_CPU 339 // define syscall number
#define FCFS_POLICY 0
#define SRTF_POLICY 1
#define RR_POLICY 2
#define PP_POLICY 3

int main(int argc, char **argv) {
    int jobTime;
    int delayTime;
    char name[16];

    int wait = 0;
    int response = -1;
    int tick = 0;
    int priority = -1; // 기본값: 우선순위 없음
    int policy = RR_POLICY;
    
    if (argc < 4) {
        printf("\nInsufficient Arguments .. \n");
        return 1;
    }

    jobTime = atoi(argv[1]);
    delayTime = atoi(argv[2]);
    strcpy(name, argv[3]);

    if (argc >= 5) {
        priority = atoi(argv[4]);
    }

    sleep(delayTime);
    printf("\nProcess %s : I will use CPU by %ds. \n", name, jobTime);

    jobTime *= 10;  // 0.1초 단위로 변환

    while (jobTime>=0) {
        long ret;

        if (priority == -1)
            ret = syscall(KU_CPU, name, jobTime, policy,-1);
        else
            ret = syscall(KU_CPU, name, jobTime,policy, priority);

        if (ret == 0) {
            if (response == -1) response = tick;
            jobTime--;
        } else {
            wait++;
        }

        usleep(100000);  // 0.1초 대기
        tick++;
    }
s
    syscall(KU_CPU, name, 0, policy);  // 작업 완료 알림

    printf("\nProcess %s : Finish! My response time is %ds and My total wait time is %ds.",
           name, (response + 5) / 10, (wait + 5) / 10);  

    return 0;
}

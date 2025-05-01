#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    int pid = fork();
    int jobLength;

    if(pid < 0){
        printf(2, "Fork failed\n");
        exit();
    }

    // Assume sjf_job_length is a system call you've implemented
    // which is declared in user.h
    if(pid > 0) { // Parent process
        wait(); // Wait for the child to finish
    }

    jobLength = sjf_job_length(getpid());
    if(jobLength < 0){
        printf(2, "Failed to get SJF job length for pid %d\n", getpid());
    } else {
        printf(1, "SJF job length for pid %d is %d\n", getpid(), jobLength);
    }

    exit();
}

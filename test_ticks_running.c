#include "types.h"
#include "user.h"
#include "syscall.h"

int main(int argc, char *argv[]) {
    int pid = getpid(); // currrent procees if
    int ticks = ticks_running(pid); // Call your new system call
    int jobLength = sjf_job_length(pid);
    if(ticks >= 0) {
        printf(1, "Process %d has job legth %d and been running for %d ticks.\n", pid, jobLength, ticks);
    } else {
        // Handle error or invalid pid
        printf(2, "Error: Could not retrieve ticks for process %d.\n", pid);
    }

    exit();
}

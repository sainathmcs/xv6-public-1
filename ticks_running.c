#include "types.h"
#include "user.h"
#include "syscall.h"

int main(int argc, char *argv[]) {
    int pid = getpid(); // Get the current process ID
    int ticks = ticks_running(pid); // Call your new system call

    if(ticks >= 0) {
        printf(1, "Process %d has been running for %d ticks.\n", pid, ticks);
    } else {
        // Handle error or invalid pid
        printf(2, "Error: Could not retrieve ticks for process %d.\n", pid);
    }

    exit();
}

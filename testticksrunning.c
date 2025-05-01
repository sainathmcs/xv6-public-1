// ticksrunningtest.c
#include "types.h"
#include "user.h"
#include "stat.h"

int main(int argc, char *argv[]) {
    int pid;

    // Optionally, get a PID from command line arguments
    if(argc < 2) {
        printf(1, "Usage: ticksrunningtest <pid>\n");
        exit();
    }

    pid = atoi(argv[1]);
    int ticks = ticks_running(pid);
    if(ticks >= 0) {
        printf(1, "Process %d has been running for %d ticks.\n", pid, ticks);
    } else {
        printf(1, "Process %d not found.\n", pid);
    }

    exit();
}

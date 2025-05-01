#include "types.h"
#include "user.h"

#define PAGE_SIZE 4096
#define NUM_PAGES 5 // Adjust based on your test needs
#define ARRAY_SIZE (NUM_PAGES * PAGE_SIZE)

int main(void) {
    int pid = getpid(); // Get the current process ID
    // int start = uptime();
    volatile char *array = sbrk(ARRAY_SIZE);
    int i, step;

    // Access every page to ensure it's mapped
    for (i = 0; i < ARRAY_SIZE; i += PAGE_SIZE) {
        array[i] = 'a'; // Trigger page fault for lazy allocation
    }

    // Further accesses to test locality-aware behavior
    step = PAGE_SIZE / 4; // Adjust step size to trigger/not trigger additional faults
    for (i = 0; i < ARRAY_SIZE - step; i += step) {
        array[i] = 'b'; // Potential additional faults if pages not preallocated
    }

    printf(1, "Test completed\n");
    // int end = uptime();
    // printf(1, " start : %d\n", start);
    // printf(1, " end : %d\n", end);
    printf(1, " ticks running:%d \n", ticks_running(pid));
    exit();
}

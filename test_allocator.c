#include "types.h"
#include "stat.h"
#include "user.h"

#define LARGE_SIZE  (1024 * 1024) // Large enough to span several pages
#define STEP        4096          // Step size to jump over page boundaries

int main(void) {
    volatile char largeArray[LARGE_SIZE];
    char sum = 0;

    for (int i = 0; i < LARGE_SIZE; i += STEP) {
        largeArray[i] = 'a'; // Access to cause page fault
    }

    // Read from the array in a way that can't be optimized out.
    for (int i = 0; i < LARGE_SIZE; i += STEP) {
        sum += largeArray[i];
    }
    
    printf(1, "Sum: %d\n", sum); // Use the result so it must compute it
    exit();
}

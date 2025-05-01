#include "types.h"
#include "user.h"

int main(int argc, char *argv[]){
    int ticks;

    // printing an error if no ticks are given.
    if (argc < 2){
        printf(2,"error: please pass ticks");
        exit();
    }

    // converting ticks to integer
    ticks = atoi(argv[1]);

    // if ticks are not validd print invalid ticks message
    if (ticks < 0) {
        printf(2,"Error: invalid ticks\n");
    }

    // else execute sleep.
    sleep(ticks);
    
    // terminating the sleep.
    exit();
}
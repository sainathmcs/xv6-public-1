#include "types.h"
#include "stat.h"
#include "user.h"
#include "ticketlock.h"

int counter = 0;
struct ticketlock lock;


void
loop(void)
{
  int i;
  for (i = 0; i < 100000; i++) {
    acquire_ticketlock(&lock);
    counter = counter + 1;
    release_ticketlock(&lock);
  }
  printf(1, "loop finished\n");
  exit();
}


int
main(int argc, char *argv[])
{
    init_ticketlock(&lock);
  clone(&loop);
    clone(&loop);
    clone(&loop);
    sleep(100);
    printf(1, "counter = %d\n", counter);
    exit();
}

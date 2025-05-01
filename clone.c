#include "types.h"
#include "stat.h"
#include "user.h"

int counter = 0;

void
loop(void)
{
  int i;
  for (i = 0; i < 1000; i++) {
    counter = counter + 1;
  }
  printf(1, "loop finished\n");
  exit();
}


int
main(int argc, char *argv[])
{
  clone(&loop);
    clone(&loop);
    clone(&loop);
    sleep(100);
    printf(1, "counter = %d\n", counter);
    exit();
}
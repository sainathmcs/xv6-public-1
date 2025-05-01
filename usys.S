#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

.global count_virtual_pages
count_virtual_pages:
  movl $SYS_count_virtual_pages, %eax
  int $T_SYSCALL
  ret

.global count_physical_pages
count_physical_pages:
  movl $SYS_count_physical_pages, %eax
  int $T_SYSCALL
  ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(ticks_running)
SYSCALL(sjf_job_length)
//SYSCALL(waitAndGetProcTimes)
SYSCALL(set_sched_priority)
SYSCALL(get_sched_priority)
SYSCALL(lseek)
SYSCALL(symlink)
SYSCALL(clone)
SYSCALL(set_park)
SYSCALL(park)
SYSCALL(unpark)
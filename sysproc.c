#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

extern int count_virtual_pages(struct proc *p);
extern int count_physical_pages(struct proc *p);

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  myproc()->sz += n;

  // if(growproc(n) < 0)
  //   return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_ticks_running(void){
  int pid;
    if(argint(0, &pid)){
    return -1;
  }
  return get_ticks_running(pid);
}

int sys_sjf_job_length(void){
  int pid;
  if(argint(0, &pid)){
    return -1;
  }
  return get_sjf_job_length(pid);
}

// int sys_waitAndGetProcTimes(void) {
//     int processId;

//     // Use argint to fetch the first argument from the syscall
//     if(argint(0, &processId) < 0) return -1;

//     return waitAndGetProcTimes(processId);
// }

int
sys_set_sched_priority(void)
{
  int pid, priority;

  // Using argint to fetch the first and second arguments
  if(argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;

  // Implementation to set the priority of the specified process
  // This might involve finding the process by pid and then setting its priority
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->priority = priority;
      release(&ptable.lock);
      return 0; // Successfully set priority
    }
  }
  release(&ptable.lock);
  return -1; // Process not found
}


int
sys_get_sched_priority(void)
{
  int pid, found = 0;
  struct proc *p;

  if(argint(0, &pid) < 0)
    return -1;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      found = 1;
      break;
    }
  }
  release(&ptable.lock);

  if(found && (p->state == RUNNABLE || p->state == RUNNING || p->state == SLEEPING))
    return p->priority;
  else
  return -1;
}

int sys_count_virtual_pages(void) {
  return count_virtual_pages(myproc());
}

int sys_count_physical_pages(void) {
  return count_physical_pages(myproc());
}

int
sys_clone(void)
{
  void *func;
  if(argptr(0, (void*)&func, sizeof(void*)) < 0)
    return -1;
  return clone(func);
}

int
sys_park(void)
{
  myproc()->state = SLEEPING;
  return 0;
}

int
sys_unpark(int pid)
{
  return unpark(pid);

}

int
sys_set_park(void)
{
  myproc()->state = PARKED;
  return 0;
}

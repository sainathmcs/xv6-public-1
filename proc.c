#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"


#define MAX_JOB_LENGTH 1000 // updated for sjf

extern uint ticks;
extern pde_t* walkpgdir(pde_t *pgdir, const void *va,int alloc);

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);
//updated for sjf

static unsigned long rand_state = 1;

// Call this with a seed value
void srand(unsigned int seed) {
    rand_state = seed;
}

// Returns a pseudo-random number
unsigned int random_no_generator(void) {
    rand_state = rand_state * 1103515245 + 12345;
    return (unsigned int)((rand_state / 65536) % 32768);
}
//unsigned int rand_state = 123456789;

// unsigned int random_no_generator(void){
//   rand_state = rand_state + 1103515245 + 12345;
//   return (unsigned int)(rand_state/65536)%32768;
// }

//end of update.
void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp; 

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->ticks_running = 0;// updated
  p->estimated_job_length = random_no_generator() % MAX_JOB_LENGTH;
  p->start_time = ticks;
  p->priority = 0; // assuming default priority =0

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  srand(ticks);
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
    #ifdef SJF_SCHED
    cprintf("Running with SJF Scheduler\n");
    #elif defined(PRIORITY_SCHED)
    cprintf("Running with Priority Scheduler\n");
    #else
    cprintf("Running with Default (Round-Robin) Scheduler\n");
    #endif

    #ifdef ALLOCATOR_LOCALITY
    cprintf("Running with Locality Aware Allocator\n");
    #else
    cprintf("Ruuning with Lazy Allocator\n");
    #endif
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }
  curproc->saved_ticks = curproc->ticks_running;
  curproc->end_time = ticks;

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  //cprintf("exit pid: %d \n",curproc->pid);
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}
// int waitAndGetProcTimes(int processId) {
//   struct proc *p;
//   int havekids, pid;
//   struct proc *curproc = myproc();

//   acquire(&ptable.lock);
//   for(;;) {
//     // Scan through the table looking for exited children.
//     havekids = 0;
//     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
//       if(p->parent != curproc)
//         continue;
//       havekids = 1;
//       if(p->state == ZOMBIE && p->pid == processId) {
//         // Print process times
//         cprintf("----------- Process Times -----------\n");
//         cprintf("Name: %s\n", p->name);
//         cprintf("PID: %d\n", p->pid);
//         cprintf("arrival time: 0");
//         cprintf("Creation Time: %d ticks\n", p->start_time);
//         cprintf("End Time: %d ticks\n", p->end_time);
//         cprintf("executiom Time: %d ticks\n", get_ticks_running(p->pid));
//         cprintf("turnaround time : %d ticks\n", p->end_time - 0);
//         cprintf("response time: %d ticks\n", p->start_time);
//         cprintf("------------------------------------\n");

//         // Clean up and return
//         pid = p->pid;
//         kfree(p->kstack);
//         p->kstack = 0;
//         freevm(p->pgdir);
//         p->pid = 0;
//         p->parent = 0;
//         p->name[0] = 0;
//         p->killed = 0;
//         p->state = UNUSED;
//         release(&ptable.lock);
//         return pid;
//       }
//     }

//     if(!havekids || curproc->killed) {
//       release(&ptable.lock);
//       return -1; // No children or error
//     }

//     // Wait for a child to exit
//     sleep(curproc, &ptable.lock); // DOC: wait-sleep
//   }
// }


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

void scheduler(void) {
    struct proc *p;
    static int seeded = 0; // updated
  if (!seeded){
    rand_state += cpuid();
    seeded = 1;
    }
    struct cpu *c = mycpu();
    c->proc = 0;
    #ifdef PRIORITY_SCHED
    struct proc *start = 0; // Pointer to remember where we start looking for a process
    #endif

    for(;;) {
        sti();

        acquire(&ptable.lock);
 
  #ifdef SJF_SCHED 
  //cprintf("SJF"); 
  struct proc *shortest = 0; // updated
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      if (!shortest || p->estimated_job_length < shortest->estimated_job_length)
        shortest = p;
    }
    if (shortest){
      p = shortest;
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
            swtch(&(c->scheduler), p->context);
      p->ticks_running++;
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    //cprintf("name: %s\n", shortest->name);
#else
#ifdef PRIORITY_SCHED
        int foundProcess = 0; // Flag to indicate if we have found a process to run
        struct proc *highestPrioProc = 0;
        int highestPrio = 0x0000000F; // Start with the lowest priority

        for(int pri = HIGH_PRIORITY; pri <= LOW_PRIORITY && !foundProcess; pri++) {
            if(!start) start = ptable.proc; // If start is NULL, start from the beginning
            for(p = start; p < &ptable.proc[NPROC]; p++) {
                if(p->state == RUNNABLE && p->priority == pri) {
                    if (!highestPrioProc || p->priority < highestPrio) {
                        highestPrioProc = p;
                        highestPrio = p->priority;
                        // Debug statement
                       // cprintf("[Scheduler] Considering PID %d with priority %d\n", p->pid, p->priority);
                        foundProcess = 1;
                        break; // Found a process to run at this priority level
                    }
                }
            }
            if(foundProcess) {
                // Prepare for the next round
                start = highestPrioProc + 1;
                if(start >= &ptable.proc[NPROC]) start = ptable.proc; // Wrap around
                break;
            }
            // If no process is found at this priority, reset start for the next priority level
            start = ptable.proc;
        }

        if(highestPrioProc) {
            p = highestPrioProc;
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            p->ticks_running++;
            switchkvm();

            // Debug statement
           //cprintf("[Scheduler] Running PID %d with priority %d (Round-Robin within Priority)\n", p->pid, p->priority);


            c->proc = 0;
        }
#else
//cprintf("[Scheduler] RR\n");  
       for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if(p->state != RUNNABLE)
                continue;
            
            // This process is runnable, switch to it
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            p->ticks_running++;
            switchkvm();
            

            //cprintf("[Scheduler] Running PID %d (Round-Robin)\n", p->pid);

            c->proc = 0;
            break; // Yield the CPU to the next process
        }
#endif
#endif
        release(&ptable.lock);
}
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// updated
int
get_ticks_running(int)
{
  int pid;
  struct proc *p;

  // Argument fetch
  if(argint(0, &pid) < 0){
    //printf(2,"error");
    return -3;
  }
  //cprintf("%d\n",pid);

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    // cprintf("Checking process with PID %d, state %d\n", p->pid, p->state);
    if(p->pid == pid){
      //found = 1;
       //cprintf("Checking unused");
      if(p->state == UNUSED) {
        release(&ptable.lock);
        return -3; // No such process
      }
      if (p->state == ZOMBIE) {
        cprintf("Checking zombie");
        // Process has exited, return the saved ticks_running value
        int savedTicks = p->saved_ticks;
        release(&ptable.lock);
        return savedTicks;
      }
      // Process is still active, return the current ticks_running value
      int ticks = p->ticks_running;
      release(&ptable.lock);
      return ticks;
    }
  }
  release(&ptable.lock);
  return -2; // Process not found
}
//       release(&ptable.lock);
//       return p->ticks_running; // Found and return ticks
//     }
//   }
//   release(&ptable.lock);
//   if(!found) return -3; // No such process
//   return 0; // Unreachable, just to satisfy the compiler
// }

// updated
int
get_sjf_job_length(int pid)
{
    struct proc *p;
    int length = -1; // Default to -1 if not found
    
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->pid == pid) {
            length = p->estimated_job_length; // Assuming est_job_length is your field name
            break;
        }
    }
    release(&ptable.lock);
    
    return length;
}

int count_virtual_pages(struct proc *p) {
  int count = 0;
  // Assuming PGSIZE is the page size and KERNBASE is the start of kernel space
  for (void *va = 0; va < (void *)KERNBASE; va += PGSIZE) {
    if (walkpgdir(p->pgdir, va, 0) != 0) {
      count++;
    }
  }
  return count;
}

int count_physical_pages(struct proc *p) {
  int count = 0;
  pte_t *pte;
  // Assuming PGSIZE is the page size and KERNBASE is the start of kernel space
  for (void *va = 0; va < (void *)KERNBASE; va += PGSIZE) {
    pte = walkpgdir(p->pgdir, va, 0);
    if (pte && (*pte & PTE_P)) {
      count++;
    }
  }
 return count;
}

int 
clone(void(*fun))
{
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  np->pgdir = curproc->pgdir;
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;
  acquire(&ptable.lock);
  void* stack =(void *) curproc->current_tstack;
  curproc->current_tstack -= 2*PGSIZE;
  release(&ptable.lock);

  np->tf->esp = (uint) stack;

  np->tf->esp += PGSIZE - 1 * sizeof(void *);
  np->tf->ebp = np->tf->esp;

  np->tf->eip = (uint) fun;

  np->tf->eax = 0;

  int i;
  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));
 
  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return np->pid;
}

int
unpark(int pid)
{
  struct proc *p;
  int found = 0;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->state = RUNNABLE;
      found = 1;
      break;
    }
  }
  release(&ptable.lock);

  if(found) return 0;
  return -1;
}

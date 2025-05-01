#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}


//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(myproc() && myproc()->state == RUNNING)
      myproc()->ticks_running++;
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    #ifdef ALLOCATOR_LOCALITY
  {
    cprintf("locality allocator and the location is:%d\n", rcr2());
    char *allocatedPage;
    uint a1 = PGROUNDDOWN(rcr2()); // Address adjustment to page boundary

    for(int pageIndex = 0; pageIndex < 3; pageIndex++) { // Extend over three pages
      allocatedPage = kalloc(); // create a new page
      if(allocatedPage == 0){
        cprintf("memory allocation failed\n");
        myproc()->killed = 1; // Terminate the process in case of allocation failure
        break; // Exit the loop early on failure
    }
    memset(allocatedPage, 0, PGSIZE); // Initialize the page to zero
    uint mappingAddress = a1 + pageIndex * PGSIZE; // Calculate the target address for mapping
    
    // Attempt to map the allocated page into the process's virtual address space
    if(mappages(myproc()->pgdir, (void*)mappingAddress, PGSIZE, V2P(allocatedPage), PTE_W|PTE_U) < 0){
        cprintf("memory mapping failed\n");
        kfree(allocatedPage); // Free the page on mapping failure
        myproc()->killed = 1; // Process termination on failure
        break; // Exit loop on failure
    }
    }
  }
    #else
    {
      cprintf(" lazy allocator and the location is %d: \n", rcr2());
        char *mem;
            uint a;
            a = PGROUNDDOWN(rcr2()); // Align the faulting address to page boundary

            // Allocate a page of physical memory
            mem = kalloc();
            if(mem == 0){
                cprintf("out of memory\n");
                myproc()->killed = 1; // Kill the current process if memory allocation fails
                break;
            }
            memset(mem, 0, PGSIZE); // Clear the allocated memory

            // Map the newly allocated page to the process's address space
            if(mappages(myproc()->pgdir, (void*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
                cprintf("out of memory (2)\n");
                kfree(mem);
                myproc()->killed = 1;
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
            }
  }
  #endif
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
  

#include "ticketlock.h"

typedef struct node{
    int pid;
    struct node *next;
}node;

typedef struct queuelock{
    struct node proc_node;
    int locked;
    struct ticketlock lock;
}queuelock;


void init_queuelock(queuelock *lk);
void acquire_queuelock(queuelock *lk);
void release_queuelock(queuelock *lk);

#include "types.h"

typedef struct ticketlock
{
    uint ticket;
    uint turn;
}ticketlock;

void init_ticketlock(ticketlock *lk);
void acquire_ticketlock(ticketlock *lk);
void release_ticketlock(ticketlock *lk);
#include "types.h"
#include "user.h"
#include "ticketlock.h"
#include "x86.h"

void
init_ticketlock(ticketlock *lk)
{
    lk->ticket = 0;
    lk->turn = 0;
}

void
acquire_ticketlock(ticketlock *lk)
{
    uint myturn = fetch_and_add(&lk->ticket, 1);
    while (lk->turn != myturn)
        sleep(1);
}


void
release_ticketlock(ticketlock *lk)
{
    fetch_and_add(&lk->turn, 1);
}


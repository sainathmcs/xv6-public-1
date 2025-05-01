#include "types.h"
#include "user.h"
#include "queuelock.h"
#include "ticketlock.h"

void init_queuelock(queuelock *lk) {
    lk->node.pid = 0;
    lk->node.next = 0;
    lk->locked = 0;
    init_ticketlock(&lk->lock);
}

void acquire_queuelock(queuelock *lk) {
    if (lk->locked) {
        // Add the process to the queue
        node *new_node = malloc(sizeof(node));
        new_node->pid = getpid();
        new_node->next = 0;
        acquire_ticketlock(&lk->lock);
        // Traverse the queue to find the tail
        node *tail = &lk->node;

        while (tail->next) {
            tail = tail->next;
        }
        
        tail->next = new_node;
        setpark();
        release_ticketlock(&lk->lock);
        park(getpid());
    } else {
        acquire_ticketlock(&lk->lock);
        lk->locked = 1;
        lk->node->pid 
        lk->node.next = 0;
        release_ticketlock(&lk->lock);
    }
}

void release_queuelock(queuelock *lk) {
    acquire_ticketlock(&lk->lock);
    if (lk->node.next) {
        // Remove the head of the queue
        node *temp = lk->node.next;
        lk->node.next = temp->next;
        free(temp);
        unpark(lk->node.pid);
    } else {
        lk->locked = 0;
    }
    release_ticketlock(&lk->lock);
}
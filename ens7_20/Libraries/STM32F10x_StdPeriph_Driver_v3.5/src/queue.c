/*
 * queue.c
 *
 *  Created on: 2018. 11. 1.
 *      Author: Team07
 */

#include "queue.h"

int queue_pop(Queue * q, Task *msg) {

    uint16_t tail;

    // ring buffer -> <= won't work
    if(q->tail != q->head) {
        tail = q->tail + 1;
        if(tail >= MAXIMUM_QUEUE_LENGTH) tail = 0;
        q->tail = tail;
        *msg = q->msg[tail];
        return 1;
    }
    return 0;
}

void queue_push(Queue * q, Task msg) {

    uint16_t head;

    head = (q->head + 1);
    if(head >= MAXIMUM_QUEUE_LENGTH) head = 0;

    // caught main's tail?
    q->msg[head] = msg;
    q->head = head;

    if ( head == q->tail) {
        // skip message
        q->overflow = 1;
    }

}

bool isEmpty(Queue *q) {
	if (q->head == q->tail) return true;
	else return false;
}
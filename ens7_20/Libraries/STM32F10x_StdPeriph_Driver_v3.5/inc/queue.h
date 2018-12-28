/*
 * queue.h
 *
 *  Created on: 2018. 11. 1.
 *      Author: Team07
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "stm32f10x.h"

typedef int bool;

#define true 1
#define false 0

#define MAXIMUM_QUEUE_LENGTH 100

typedef struct _Task {
	char data;
	int usartType;
} Task;

typedef struct _Queue {
	Task msg[15];
    volatile uint16_t tail;
    volatile uint16_t head;
    volatile uint16_t overflow;
} Queue;

int queue_pop(Queue * q, Task *msg);
void queue_push(Queue * q, Task msg);
bool isEmpty(Queue *q);


#endif /* QUEUE_H_ */
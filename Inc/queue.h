#include "main.h"

#ifndef queue_H
#define queue_H

typedef struct QUEUE {
    uint8_t val[1000];
    int lpos, rpos;
} queue;

void clear(queue *);
void push(queue *, uint8_t);
uint8_t front(queue *);
void pop(queue *);
uint8_t empty(queue *);

#endif
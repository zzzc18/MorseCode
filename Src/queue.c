#include "queue.h"
#include <stdio.h>
#include <string.h>
#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "usart.h"

void clear(queue *que) { que->lpos = que->rpos = 0; }

void push(queue *que, uint8_t ch) { que->val[que->rpos++] = ch; }

uint8_t front(queue *que) { return que->val[que->lpos]; }

void pop(queue *que) { que->lpos++; }

uint8_t empty(queue *que) { return que->lpos == que->rpos; }
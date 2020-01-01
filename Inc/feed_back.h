#include "gpio.h"
#include "main.h"

#ifndef feed_back_H
#define feed_back_H

#define Beep_On 1u
#define Beep_Off 0u

void Beep(uint8_t);
// type 0:stop 1:beep

void Light(uint8_t, uint8_t);
// GPIO_Pinstate,type 0:staight 1:smooth

void Receive_Light(uint8_t, uint8_t);
// GPIO_Pinstate,type 0:staight 1:smooth
// UART_Mode

void Error_Light();
// Error_LED

#endif
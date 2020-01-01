#include "main.h"

#ifndef morse_H
#define morse_H

#define map_max 5700
#define Input_Error ((uint8_t)0xFF)
#define Input_Empty (Input_Error - 1)
void Morse_init();
void Clear_BUF();
uint8_t Caps_Trans(uint8_t);
uint8_t convert_m2l();
void convert_l2m(uint8_t *, uint8_t);
void flush();
void inc(uint8_t *);
void Dot();
void Line();
void Min_Blank();
void Letter_Blank();
void Word_Blank();
#endif
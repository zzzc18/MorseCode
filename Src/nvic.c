#include "nvic.h"
#include <stdio.h>
#include "feed_back.h"
#include "gpio.h"
#include "main.h"
#include "morse.h"
#include "queue.h"
#include "tim.h"
#include "usart.h"

extern uint16_t invmap[200];

uint8_t Error_Flag[] = "Error: flag value is not working as expectation!";
uint8_t TMP_BUF[20];
uint8_t rx;
static queue que;

static uint8_t flag;  // flag==1 : Button pressed
static uint8_t cnt;   // time=cnt*50ms

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == SW_Pin) {
        My_Delay(20);  // debouncing
        if (HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin) == GPIO_PIN_RESET) {
            flag = 1;
            // Below means blank length calc
            if (cnt >= 39) {
                Word_Blank();
            } else if (cnt >= 19) {
                Letter_Blank();
            } else {
                Min_Blank();
            }
            cnt = 0;
            Light(GPIO_PIN_SET, 0);
            Beep(Beep_On);
        }
    }
    // MyDelay(15);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (!empty(&que)) Rev_flush();
    GPIO_PinState state = HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin);
    if (state == GPIO_PIN_RESET) {
        // Button Pressed. EXIT should have received that, so flag should be
        // positive.
        if (!flag) {
            HAL_UART_Transmit(&huart1, Error_Flag, sizeof(Error_Flag), 0xfff);
            return;
        }
        inc(&cnt);
        Light(GPIO_PIN_SET, 0);
#ifndef Master_Mode
        sprintf(TMP_BUF, "+++++++%3d+++++++", cnt);
        HAL_UART_Transmit(&huart1, TMP_BUF, sizeof(TMP_BUF) - 1, 0xfff);
#endif
    } else {
        if (flag) {
            if (cnt >= 19) {
                Line();
            } else if (cnt >= 5) {
                Dot();
            }
            cnt = 0;
            flag = 0;
            Beep(Beep_Off);
            Light(GPIO_PIN_RESET, 1);
        } else {
            inc(&cnt);
#ifndef Master_Mode
            sprintf(TMP_BUF, "-------%3d--------", cnt);
            HAL_UART_Transmit(&huart1, TMP_BUF, sizeof(TMP_BUF) - 1, 0xfff);
#endif
        }
    }
}

void My_UART_IRQHandler() {
    uint8_t ch = (uint8_t)(huart1.Instance->DR & (uint8_t)0xFF);
    push(&que, ch);
}

void Rev_flush() {
    uint8_t tmp[10];
    if (empty(&que)) {
        return;
    }
    while (!empty(&que)) {
        uint8_t ch = front(&que);
        pop(&que);
#ifndef Master_Mode
        HAL_UART_Transmit(&huart1, &ch, 1, 0xfff);
#endif
        ch = Caps_Trans(ch);
        if (ch == ' ') {
            My_Delay(2000);
        } else if (invmap[ch]) {
            convert_l2m(tmp, ch);
            int tmp_len = strlen(tmp);
            for (int k = 0; k < tmp_len; k++) {
                if (tmp[k] == '.') {
                    Receive_Light(GPIO_PIN_SET, 0);
                    Beep(Beep_On);
                    My_Delay(300);
                    Beep(Beep_Off);
                    Receive_Light(GPIO_PIN_RESET, 1);
                } else {
                    Receive_Light(GPIO_PIN_SET, 0);
                    Beep(Beep_On);
                    My_Delay(1000);
                    Beep(Beep_Off);
                    Receive_Light(GPIO_PIN_RESET, 1);
                }
            }
            My_Delay(1000);
        } else {
            static uint8_t error_info[20];
            sprintf(error_info, "Can't find %c in dictionary!", ch);
            HAL_UART_Transmit(&huart1, error_info, sizeof(error_info) - 1,
                              0xfff);
            Error_Light();
            My_Delay(1000);
        }
    }
    clear(&que);
    HAL_UART_Transmit(&huart1, "Translation End.",
                      sizeof("Translation End.") - 1, 0xfff);
}
#include "feed_back.h"
#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "usart.h"

const uint8_t PWM_max = 100 - 1;

void Beep(uint8_t type) {
    const uint32_t Hz = 5000;
    if (type == Beep_Off) {
        TIM2->CCR1 = 0;
        return;
    }
    TIM2->ARR = Hz;
    TIM2->CCR1 = TIM2->ARR >> 1;
}

void Light(uint8_t state, uint8_t type) {
    if (!type) {
        if (state == GPIO_PIN_SET) {
            TIM1->CCR2 = PWM_max;
        } else
            TIM1->CCR2 = 0;
    } else {  // only decrease need smooth
        for (int i = PWM_max, step = 1; i - step >= 0; i -= step) {
            TIM1->CCR2 = i;
            My_Delay(5);
        }
        TIM1->CCR2 = 0;
    }
}

void Receive_Light(uint8_t state, uint8_t type) {
    if (!type) {
        if (state == GPIO_PIN_SET) {
            TIM1->CCR3 = PWM_max;
        } else
            TIM1->CCR3 = 0;
    } else {  // only decrease need smooth
        for (int i = PWM_max, step = 1; i - step >= 0; i -= step) {
            TIM1->CCR3 = i;
            My_Delay(5);
        }
        TIM1->CCR3 = 0;
    }
}

void Error_Light() {
    TIM1->CCR4 = PWM_max;
    My_Delay(500);
    for (int i = PWM_max, step = 1; i - step >= 0; i -= step) {
        TIM1->CCR4 = i;
        My_Delay(10);
    }
    TIM1->CCR4 = 0;
}
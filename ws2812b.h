/**
  ******************************************************************************
  * @file           : ws2812b.h
  * @brief          : Ws2812b library header
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 Lars Boegild Thomsen <lbthomsen@gmail.com>.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by lbthomsen under MIT license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/MIT
  *
  ******************************************************************************
  */

#ifndef __WS2812B_H
#define __WS2812B_H

// Buffer allocated will be twice this
#define BUFFER_SIZE 24

// LED on/off counts.  PWM timer is running 104 counts.
#define LED_OFF 27
#define LED_ON 61
#define LED_RESET_CYCLES 10 // Full 24-bit cycles

// Define LED driver state machine states
#define LED_RES 0
#define LED_DAT 1

#define G 0
#define R 1
#define B 2

#define WS2812B_INIT_OK 0
#define WS2812B_INIT_MEM 1

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim);

typedef struct {
	TIM_HandleTypeDef *timer0;
	TIM_HandleTypeDef *timer1;
	uint32_t channel0;
	uint32_t channel1;
} ws2823b_init_TypeDef;

uint8_t ws2812b_init(TIM_HandleTypeDef *init_timer, uint32_t init_channel, uint16_t init_rows, uint16_t init_cols);

// Set a single led value
void setLedValue(uint8_t col, uint8_t row, uint8_t led, uint8_t value);

// Set values of all 3 leds
void setLedValues(uint8_t col, uint8_t row, uint8_t r, uint8_t g, uint8_t b);

#endif // _WS2812B_H

/**
 ******************************************************************************
 * @file           : ws2812.h
 * @brief          : Ws2812 library header
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 - 2024 Lars Boegild Thomsen <lbthomsen@gmail.com>.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __WS2812_H
#define __WS2812_H

// Buffer allocated will be twice this
#define BUFFER_SIZE 24

// LED on/off counts.  PWM timer is running 125 counts.  LED_CNT need to be set to the total counts in the PWM.
#define LED_OFF 1 * LED_CNT / 3 - 1
#define LED_ON 2 * LED_CNT / 3 + 2
#define LED_RESET_CYCLES 10 // Full 24-bit cycles

// Define LED driver state machine states
#define LED_RES 0 // Reset
#define LED_IDL 1 // Idle
#define LED_DAT 2 // Transfer data

#define GL 0 // Green LED
#define RL 1 // Red LED
#define BL 2 // Blue LED

#define WS2812B_INIT_OK 0
#define WS2812B_INIT_MEM 1

//void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
//void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim);

typedef struct {
    TIM_HandleTypeDef *timer;
    uint32_t channel;
    uint16_t dma_buffer[BUFFER_SIZE * 2];
    uint16_t leds;
    uint8_t *led;
    uint8_t led_state;
} ws2812_TypeDef;

uint8_t ws2812_init(ws2812_TypeDef *ws2812, TIM_HandleTypeDef *timer, uint32_t channel, uint16_t leds);

void ws2812_update_buffer(ws2812_TypeDef *ws2812, uint16_t *dma_buffer_pointer);

// Set all led values to zero
void zeroLedValues();

// Set a single led value
void setLedValue(uint16_t led, uint8_t color, uint8_t value);

// Set values of all 3 leds
void setLedValues(uint16_t led, uint8_t r, uint8_t g, uint8_t b);

#endif // _WS2812_H

/* 
 * vim: ts=4 nowrap
 */

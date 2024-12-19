/**
 ******************************************************************************
 * @file           : ws2812.h
 * @brief          : Ws2812 library header
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 - 2024 Lars Boegild Thomsen <lbthomsen@gmail.com>
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

#include "main.h"

// Buffer allocated will be twice this
#define BUFFER_SIZE 24

// LED on/off counts.  PWM timer is running 125 counts.  LED_CNT need to be set to the total counts in the PWM.
#define LED_OFF 1 * LED_CNT / 3 - 1  // A bit less than 1/3
#define LED_ON 2 * LED_CNT / 3 + 2   // A bit more than 2/3
#define LED_RESET_CYCLES 10          // Full 24-bit cycles

#define GL 0 // Green LED
#define RL 1 // Red LED
#define BL 2 // Blue LED

typedef enum {
    WS2812_Ok,
    WS2812_Err,
    WS2812_Mem
} ws2812_resultTypeDef;

typedef enum {
    LED_RES = 0,
    LED_IDL = 1,
    LED_DAT = 2
} ws2812_stateTypeDef;

typedef struct {
    TIM_HandleTypeDef *timer;               // Timer running the PWM - MUST run at 800 kHz
    uint32_t channel;                       // Timer channel
    uint16_t dma_buffer[BUFFER_SIZE * 2];   // Fixed size DMA buffer
    uint16_t leds;                          // Number of LEDs on the string
    uint8_t *led;                           // Dynamically allocated array of LED RGB values
    ws2812_stateTypeDef led_state;          // LED Transfer state machine
    uint8_t led_cnt;
    uint8_t res_cnt;
    uint8_t is_dirty;
    uint8_t zero_halves;
    uint32_t dma_cbs;
    uint32_t dat_cbs;
} ws2812_handleTypeDef;

ws2812_resultTypeDef ws2812_init(ws2812_handleTypeDef *ws2812, TIM_HandleTypeDef *timer, uint32_t channel, uint16_t leds);

void ws2812_update_buffer(ws2812_handleTypeDef *ws2812, uint16_t *dma_buffer_pointer);

// Set all led values to zero
ws2812_resultTypeDef zeroLedValues(ws2812_handleTypeDef *ws2812);

// Set a single led value
ws2812_resultTypeDef setLedValue(ws2812_handleTypeDef *ws2812, uint16_t led, uint8_t color, uint8_t value);

// Set values of all 3 leds
ws2812_resultTypeDef setLedValues(ws2812_handleTypeDef *ws2812, uint16_t led, uint8_t r, uint8_t g, uint8_t b);

#endif // _WS2812_H
/* 
 * vim: ts=4 nowrap
 */

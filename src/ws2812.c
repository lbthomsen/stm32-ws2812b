/**
 ******************************************************************************
 * @file           : ws2812.c
 * @brief          : Ws2812 library
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 Lars Boegild Thomsen <lbthomsen@gmail.com>.
 * All rights reserved
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/**
 * Notice, a timer with a DMA driven PWM output will need to be configured
 * before this library is initialized.
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"

#include "ws2812.h"
#include "color_values.h"

/*
 * Update next 24 bits in the dma buffer - assume dma_buffer_pointer is pointing
 * to the buffer that is safe to update.  The dma_buffer_pointer and the call to
 * this function is handled by the dma callbacks.
 */
inline void ws2812_update_buffer(ws2812_handleTypeDef *ws2812, uint16_t *dma_buffer_pointer) {

#ifdef BUFF_GPIO_Port
	HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_SET);
#endif

    // A simple state machine - we're either resetting (two buffers worth of zeros),
    // idle (just winging out zero buffers) or
    // we are transmitting data for the "current" led.

	++ws2812->dma_cbs;

    if (ws2812->led_state == LED_RES) { // Latch state - 10 or more full buffers of zeros

        // This one is simple - we got a bunch of zeros of the right size - just throw
        // that into the buffer.  Twice will do (two half buffers).
        if (ws2812->zero_halves < 2) {
            memset(dma_buffer_pointer, 0, 2 * BUFFER_SIZE); // Fill the buffer with zeros
            ws2812->zero_halves++; // We only need to update two half buffers
        }

        ws2812->res_cnt++;

        if (ws2812->res_cnt >= LED_RESET_CYCLES) { // done enough reset cycles - move to next state
            ws2812->led_cnt = 0;	// prepare to send data
            if (ws2812->is_dirty) {
                ws2812->is_dirty = false;
                ws2812->led_state = LED_DAT;
            } else {
                ws2812->led_state = LED_IDL;
            }
        }

    } else if (ws2812->led_state == LED_IDL) { // idle state

        if (ws2812->is_dirty) { // we do nothing here except waiting for a dirty flag
            ws2812->is_dirty = false;
            ws2812->led_state = LED_DAT; // when dirty - start processing data
        }

    } else { // LED_DAT

        ++ws2812->dat_cbs;

        // First let's deal with the current LED
        uint8_t *led = (uint8_t*) &ws2812->led[3 * ws2812->led_cnt];

        for (uint8_t c = 0; c < 3; c++) { // Deal with the 3 color leds in one led package

            // Copy values from the pre-filled color_value buffer
            memcpy(dma_buffer_pointer, color_value[led[c]], 16); // Lookup the actual buffer data
            dma_buffer_pointer += 8; // next 8 bytes

        }

        // Now move to next LED switching to reset state when all leds have been updated
        ws2812->led_cnt++; // Next led
        if (ws2812->led_cnt >= ws2812->leds) { // reached top
            ws2812->led_cnt = 0; // back to first
            ws2812->zero_halves = 0;
            ws2812->res_cnt = 0;
            ws2812->led_state = LED_RES;
        }

    }

#ifdef BUFF_GPIO_Port
	HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_RESET);
#endif

}

ws2812_resultTypeDef zeroLedValues(ws2812_handleTypeDef *ws2812) {
    ws2812_resultTypeDef res = WS2812_Ok;
    memset(ws2812->led, 0, ws2812->leds * 3); // Zero it all
    ws2812->is_dirty = true; // Mark buffer dirty
    return res;
}

ws2812_resultTypeDef setLedValue(ws2812_handleTypeDef *ws2812, uint16_t led, uint8_t col, uint8_t value) {
    ws2812_resultTypeDef res = WS2812_Ok;
    if (led < ws2812->leds) {
        ws2812->led[3 * led + col] = value;
        ws2812->is_dirty = true; // Mark buffer dirty
    } else {
        res = WS2812_Err;
    }
    return res;
}

// Just throw values into led_value array - the dma interrupt will
// handle updating the dma buffer when needed
ws2812_resultTypeDef setLedValues(ws2812_handleTypeDef *ws2812, uint16_t led, uint8_t r, uint8_t g, uint8_t b) {
    ws2812_resultTypeDef res = WS2812_Ok;
    if (led < ws2812->leds) {
        ws2812->led[3 * led + RL] = r;
        ws2812->led[3 * led + GL] = g;
        ws2812->led[3 * led + BL] = b;
        ws2812->is_dirty = true; // Mark buffer dirty
    } else {
        res = WS2812_Err;
    }
    return res;
}

ws2812_resultTypeDef ws2812_init(ws2812_handleTypeDef *ws2812, TIM_HandleTypeDef *timer, uint32_t channel, uint16_t leds) {

    ws2812_resultTypeDef res = WS2812_Ok;

    // Store timer handle for later
    ws2812->timer = timer;

    // Store channel
    ws2812->channel = channel;

    ws2812->leds = leds;

    ws2812->led_state = LED_RES;
    ws2812->is_dirty = 0;
    ws2812->zero_halves = 2;

    ws2812->led = malloc(leds * 3);
    if (ws2812->led != NULL) { // Memory for led values

        memset(ws2812->led, 0, leds * 3); // Zero it all

        // Start DMA to feed the PWM with values
        // At this point the buffer should be empty - all zeros
        HAL_TIM_PWM_Start_DMA(timer, channel, (uint32_t*)ws2812->dma_buffer, BUFFER_SIZE * 2);

    } else {
        res = WS2812_Mem;
    }

    return res;

}

/* 
 * vim: ts=4 nowrap
 */

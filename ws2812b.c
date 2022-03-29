/**
 ******************************************************************************
 * @file           : ws2812b.c
 * @brief          : Ws2812b library
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Lars Boegild Thomsen <lbthomsen@gmail.com>.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by lbthomsen under MIT license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/MIT
 *
 ******************************************************************************
 */

/**
 * Notice, a timer with a DMA driven PWM output will need to be configured
 * before this library is initialized.
 */

#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "color_values.h"
#include "ws2812b.h"

// Timer handle and channel number
TIM_HandleTypeDef *timer;
uint32_t channel;

// Number of leds - set by init
uint16_t leds = 0;

// The actual DMA buffer - contains two halves each containing 24 bytes - 48 bytes in total.
// Used directly by the stm32 hardware to control pwm.
uint16_t dma_buffer[BUFFER_SIZE * 2] = { 0 };

// Pointer to above buffer.  This will be set to the start or the half way point in turn
uint16_t *dma_buffer_pointer;

// LED RGB values - malloc'ed in init when size is known.  3 bytes per led.
uint8_t *led_value;

// Variables controlling the state machine.
uint8_t led_state = LED_RES;
uint8_t res_cnt = 0;
uint8_t led_cnt = 0;

// Bit of optimization stuff to avoid unnecessary work
uint8_t zero_halves = 2; // 0, 1 or 2 - since buffer is already zero no need to bother
bool is_dirty = false;     // Dirty is set true when led_value array is updated.

/*
 * Update next 24 bits in the dma buffer - assume dma_buffer_pointer is pointing
 * to the buffer that is safe to update.  The dma_buffer_pointer and the call to
 * this function is handled by the dma callbacks.
 *
 */
static inline void update_next_buffer() {

#ifdef BUFF_GPIO_Port
	HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_SET);
#endif

	// A simple state machine - we're either resetting (two buffers worth of zeros),
	// idle (just winging out zero buffers) or
	// we are transmitting data for the "current" led.

	if (led_state == LED_RES) { // Latch state - 10 or more full buffers of zeros

		// This one is simple - we got a bunch of zeros of the right size - just throw
		// that into the buffer.  Twice will do (two half buffers).
		if (zero_halves < 2) {
			memset(dma_buffer_pointer, 0, 48); // Fill the buffer with zeros
			zero_halves++; // We only need to update two half buffers
		}

		res_cnt++;

		if (res_cnt >= LED_RESET_CYCLES) { // done enough reset cycles - move to next state
			led_cnt = 0;	// prepare to send data
			if (is_dirty) {
				is_dirty = false;
				led_state = LED_DAT;
			} else {
				led_state = LED_IDL;
			}
		}

	} else if (led_state == LED_IDL) { // idle state

		if (is_dirty) { // we do nothing here except waiting for a dirty flag
			is_dirty = false;
			led_state = LED_DAT; // when dirty - start processing data
		}

	} else { // LED state

		// First let's deal with the current LED
		uint8_t *led = (uint8_t*) &led_value[3 * led_cnt];

		for (uint8_t c = 0; c < 3; c++) { // Deal with the 3 color leds in one led package

			// Copy values from the pre-filled color_value buffer
			memcpy(dma_buffer_pointer, color_value[led[c]], 16); // Lookup the actual buffer data
			dma_buffer_pointer += 8; // next 8 bytes

		}

		// Now move to next LED switching to reset state when all leds have been updated
		led_cnt++; // Next led
		if (led_cnt >= leds) { // reached top
			led_cnt = 0; // back to first
			zero_halves = 0;
			res_cnt = 0;
			led_state = LED_RES;
		}

	}

#ifdef BUFF_GPIO_Port
	HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_RESET);
#endif

}

// Done sending first half of the DMA buffer - this can now safely be updated
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == timer->Instance) {
		dma_buffer_pointer = &dma_buffer[0];
		update_next_buffer();
	}

}

// Done sending the second half of the DMA buffer - this can now be safely updated
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == timer->Instance) {
		dma_buffer_pointer = &dma_buffer[BUFFER_SIZE];
		update_next_buffer();
	}

}

void zeroLedValues() {
	memset(led_value, 0, leds * 3); // Zero it all
	is_dirty = true; // Mark buffer dirty
}

void setLedValue(uint16_t led, uint8_t col, uint8_t value) {
	if (led < leds) {
		led_value[3 * led + col] = value;
		is_dirty = true; // Mark buffer dirty
	}
}

// Just throw values into led_value array - the dma interrupt will
// handle updating the dma buffer when needed
void setLedValues(uint16_t led, uint8_t r, uint8_t g, uint8_t b) {
	if (led < leds) {
		led_value[3 * led + RL] = r;
		led_value[3 * led + GL] = g;
		led_value[3 * led + BL] = b;
		is_dirty = true; // Mark buffer dirty
	}
}

uint8_t ws2812b_init(TIM_HandleTypeDef *init_timer, uint32_t init_channel,
		uint16_t init_leds) {

	// Store timer handle for later
	timer = init_timer;

	// Store channel
	channel = init_channel;

	leds = init_leds;

	led_value = malloc(leds * 3);
	if (led_value != NULL) { // Memory for led values

		memset(led_value, 0, leds * 3); // Zero it all

		// Start DMA to feed the PWM with values
		// At this point the buffer should be empty - all zeros
		HAL_TIM_PWM_Start_DMA(timer, channel, (uint32_t*) dma_buffer,
		BUFFER_SIZE * 2);
		return WS2812B_INIT_OK;
	} else {
		return WS2812B_INIT_MEM;
	}

}

/* 
 * vim: ts=4 nowrap
 */

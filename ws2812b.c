/**
  ******************************************************************************
  * @file           : ws2812b.c
  * @brief          : Ws2812b library
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

#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "color_values.h"
#include "ws2812b.h"

// Timer handle and channel number
TIM_HandleTypeDef *timer;
uint32_t channel;

// Rows and cols - set by init
uint16_t rows = 0;
uint16_t cols = 0;

// The actual DMA buffer - contains two halves each containing 24 bytes - 48 bytes in total.
// Used directly by the stm32 hardware to control pwm.
uint16_t dma_buffer[BUFFER_SIZE * 2] = { 0 };

// Pointer to above buffer.  This will be set to the start of the half way point
uint16_t *dma_buffer_pointer;

// LED RGB values - malloc'ed in init when size is known
uint8_t *led_value;

// Variables controlling the state machine.
uint8_t led_state = LED_RES;
uint8_t res_cnt = 0;
uint8_t led_col = 0;
uint8_t led_row = 0;

// Bit of optimization stuff to avoid unnecessary work
uint8_t zero_halves = 2; // 0, 1 or 2 - since buffer is already zero no need to bother
bool is_dirty = false;
bool is_transferring = false;

/*
 * Update next 24 bits in the dma buffer - assume dma_buffer_pointer is pointing
 * to the buffer that is safe to update.
 *
 */
static inline void update_next_buffer() {

	#ifdef BUFF_GPIO_Port
		HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_SET);
	#endif

	// A simple state machine - we're either resetting (two buffers worth of zeros) or
	// we are transmitting data for the "current" led.

	if (led_state == LED_RES) { // Latch state - 10 or more full buffers of zeros

		// This one is simple - we got a bunch of zeros of the right size - just throw
		// that into the buffer.  Twice will do (two half buffers).
		if (zero_halves < 2) {
			memset(dma_buffer_pointer, 0, 48); // That's be 24 uint16_t values
			zero_halves++;
		}

		res_cnt++;

		if (res_cnt >= LED_RESET_CYCLES) { // done enough reset cycles - move to next state
			led_col = 0;	// prepare to send data
			led_row = 0;
			led_state = LED_DAT;
		}

	} else { // LED state


		// Since we're messing with the buffer, need to make sure it is zeroed at next latch
		zero_halves = 0;

		// First let's deal with the current LED
		uint8_t *led = (uint8_t *)&led_value[led_col + (cols * led_row)];

		for (uint8_t c = 0; c < 3; c++) { // Deal with the 3 color leds in one led package

			// Copy values from the pre-filled color_value buffer
			//memcpy(dma_buffer_pointer, color_value[led[c]], 16); // Lookup the actual buffer data
			memcpy(dma_buffer_pointer, color_value[led[c]], 16); // Lookup the actual buffer data
			dma_buffer_pointer += 8; // next 8 bytes

		}

		// Now move to next LED switching to reset state when all leds have been updated
		led_col++; // Next column
		if (led_col >= cols) { // reached top
			led_col = 0; // back to first
			led_row++; // and move on to next row
			if (led_row >= rows) { // reached end - change to latch state
				res_cnt = 0;
				led_state = LED_RES;
			}
		}

	}

	#ifdef BUFF_GPIO_Port
		HAL_GPIO_WritePin(BUFF_GPIO_Port, BUFF_Pin, GPIO_PIN_RESET);
	#endif

}

// Done sending first half of the DMA buffer - this can now safely be updated
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM3) {
		dma_buffer_pointer = &dma_buffer[0];
		update_next_buffer();
	}

}

// Done sending the second half of the DMA buffer - this can now be safely updated
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM3) {
		dma_buffer_pointer = &dma_buffer[BUFFER_SIZE];
		update_next_buffer();
	}

}

void setLedValue(uint8_t col, uint8_t row, uint8_t led, uint8_t value) {
	led_value[col + (cols * row) + led] = value;
	is_dirty = true;
}

// Just throw values into led_value array - the dma interrupt will
// handle updating the dma buffer when needed
void setLedValues(uint8_t col, uint8_t row, uint8_t r, uint8_t g, uint8_t b) {
	led_value[col + (cols * row) + R] = r;
	led_value[col + (cols * row) + G] = g;
	led_value[col + (cols * row) + B] = b;
	is_dirty = true;
}

void ws2812b_init(TIM_HandleTypeDef *init_timer, uint32_t init_channel, uint16_t init_rows, uint16_t init_cols) {

	// Store timer handle for later
	timer = init_timer;

	// Store channel
	channel = init_channel;

	rows = init_rows;
	cols = init_cols;

	led_value = malloc(rows * cols * 3); // Memory for led values
	memset(led_value, 0, rows * cols * 3); // Zero it all

	// Start DMA to feed the PWM with values
	// At this point the buffer should be empty - all zeros
	HAL_TIM_PWM_Start_DMA(timer, channel, (uint32_t*) dma_buffer,
	BUFFER_SIZE * 2);

}

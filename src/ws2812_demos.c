/**
 ******************************************************************************
 * @file           : ws2812_demos.h
 * @brief          : Ws2812b demos source
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 Lars Boegild Thomsen <lbthomsen@gmail.com>.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "main.h"
#include "ws2812b.h"
#include "ws2812_demos.h"

uint8_t active_demo = 0;

const uint8_t led_line_colors[][3] = {
        { 128, 0, 0 },
        { 0, 128, 0 },
        { 0, 0, 128 },
        { 128, 128, 0 },
        { 0, 128, 128 },
        { 128, 0, 128 },
        { 128, 128, 128 }
};

void ws2812_demos_set(uint8_t demo) {
    active_demo = demo;
}

void ws2812_demos_tick() {

    static uint16_t line_led = 0;
    static uint32_t line_count = 0;
    static uint8_t line_color = 0;
    static uint32_t last_led = 0;

    uint32_t now = HAL_GetTick();

    switch (active_demo) {
    case WS2812_DEMO_LINE:
        if (now - last_led >= 10) {
            zeroLedValues();
            setLedValues(line_led, led_line_colors[line_color][0], led_line_colors[line_color][1], led_line_colors[line_color][2]);
            ++line_led;
            ++line_count;
            if (line_count % 256 == 0)
                ++line_color;
            if (line_color >= sizeof(led_line_colors) / sizeof(led_line_colors[0]))
                line_color = 0;
            if (line_led >= LEDS)
                line_led = 0;
            last_led = now;
        }
        break;
    default:
        // De nothing really
    }
}

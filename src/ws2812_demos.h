/**
 ******************************************************************************
 * @file           : ws2812_demos.h
 * @brief          : Ws2812b demos header
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

#ifndef WS2812_DEMOS_H_
#define WS2812_DEMOS_H_

#define WS2812_DEMO_NONE 0
#define WS2812_DEMO_LINE 1

#include "main.h"
#include "ws2812.h"

void ws2812_demos_set(ws2812_handleTypeDef *ws2812, uint8_t demo);
void ws2812_demos_tick(ws2812_handleTypeDef *ws2812);

#endif /* WS2812_DEMOS_H_ */

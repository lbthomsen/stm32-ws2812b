
#ifndef __WS2812B_H
#define __WS2812B_H

// Buffer allocated will be twice this
#define BUFFER_SIZE 24

// LED on/off counts.  PWM timer is running 104 counts.
#define LED_OFF 33
#define LED_ON 71
#define LED_RESET_CYCLES 10

// Define LED driver state machine states
#define LED_RES 0
#define LED_DAT 1

#define G 0
#define R 1
#define B 2

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim);

void ws2812b_init(TIM_HandleTypeDef *init_timer, uint32_t init_channel, uint16_t init_rows, uint16_t init_cols);

// Set a single led value
void setLedValue(uint8_t col, uint8_t row, uint8_t led, uint8_t value);

// Set values of all 3 leds
void setLedValues(uint8_t col, uint8_t row, uint8_t r, uint8_t g, uint8_t b);

#endif // _WS2812B_H

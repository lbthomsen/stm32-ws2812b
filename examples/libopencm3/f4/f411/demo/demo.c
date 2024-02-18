/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2024 Lars Boegild Thomsen <lbthomsen@gmail.com>
 *
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

uint32_t systick = 0;

void sys_tick_handler(void) {
    ++systick;
} 

/* Set STM32 to 72 MHz. */
static void clock_setup(void) {

    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);

    /* 72MHz / 8 => 9000000 counts per second */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    /* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(8999);

    systick_interrupt_enable();

    /* Start counting. */
    systick_counter_enable();

    /* Enable GPIOB, GPIOC, and AFIO clocks. */
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_AFIO);
}

static void gpio_setup(void) {
    /* Set GPIO13 (in GPIO port C) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO13);

    AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_FULL_SWJ_NO_JNTRST;

    /* Preconfigure the LEDs. */
    gpio_clear(GPIOC, GPIO13); /* Switch on LED. */
}

int main(void) {
    uint32_t now = 0, last_blink = 0;

    clock_setup();
    gpio_setup();

    /* Blink the LEDs (PC13) on the board. */
    while (1)
    {

        now = systick;

        if (now - last_blink >= 500)
        {
            gpio_toggle(GPIOC, GPIO13); /* LED on/off */
            last_blink = now;
        }
    }

    return 0;
    
}

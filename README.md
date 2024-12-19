# stm32-ws2812

STM32 Library for WS2812 using DMA+PWM

## STM32CubeIDE Setup

The folder containing the library source will need to be included in STM32CubeIDE under Includes 

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/include_paths.png)

and under Sources:

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/source_locations.png)

The ws2812 library uses a PWM timer channel to drive the LED string.  This should be configured under timers:

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/tim3_config1.png)

A DMA channel will have to be enabled as a circular buffer:

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/tim3_dma.png)

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/tim3_gpio.png)

The "Counter value" is the most important value.  It need to divide the timer clock so that the resulting PWM frequency is exactly 800 kHz.

![include paths](https://raw.githubusercontent.com/lbthomsen/stm32-ws2812/master/images/tim3_params.png)

/**
 ******************************************************************************
 * @file           : gpio.h
 * @author         : Sam C
 * @brief          : Header file for GPIO driver for STM32L476RGTx
 ******************************************************************************
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef struct {
    volatile uint32_t MODER;   // 00: Input, 01: Output, 10: Alternate, 11: Analog
    volatile uint32_t OTYPER;  // 0: Push-Pull, 1: Open Drain 
    volatile uint32_t OSPEEDR; // 00: Low speed, 01: Medium, 10: High, 11: Very High
    volatile uint32_t PUPDR;   // 00: No-Pull, 01: Pull-Up, 10: Pull-Down
    volatile uint32_t IDR;     // 0: Reset (low), 1: Set (High)
    volatile uint32_t ODR;     // 0: Reset (low), 1: Set (High)
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;    // To connect a pin(0-7) to a peripheral like UART, SPI, PWM, etc
    volatile uint32_t AFRH;    // To connect a pin(8-15) to a peripheral like UART, SPI, PWM, etc
} GPIO_TypeDef;


#define GPIOA_BASE (0x48000000U)
#define GPIOB_BASE (0x48000400U)
#define GPIOC_BASE (0x48000800U)

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)


// Modos de Pin (para GPIOx_MODER)
#define GPIO_MODE_INPUT     0x00U // 00: Input mode (reset state)
#define GPIO_MODE_OUTPUT    0x01U // 01: General purpose output mode
#define GPIO_MODE_AF        0x02U // 10: Alternate function mode
#define GPIO_MODE_ANALOG    0x03U // 11: Analog mode

// Estados de Pin
#define GPIO_PIN_RESET      0U
#define GPIO_PIN_SET        1U


#define HEARTBEAT_LED_PIN           5     // PA5 para LD2
#define HEARTBEAT_LED_PORT       GPIOA // Ya conocido por el driver GPIO

#define EXTERNAL_LED_PWM_PIN        4     // PA6 (TIM3_CH1)
#define EXTERNAL_LED_PWM_PORT    GPIOB

#define EXTERNAL_LED_ONOFF_PIN      7     // PA7 para emulacion de estado de puerta
#define EXTERNAL_LED_ONOFF_PORT  GPIOA

#define USER_BUTTON_PIN             13    // PC13 para B1
#define USER_BUTTON_PORT         GPIOC    


void gpio_setup_pin(GPIO_TypeDef *gpio_port, uint8_t pin_number, uint8_t mode, uint8_t alternate_function);
void gpio_write_pin(GPIO_TypeDef *gpio_port, uint8_t pin_number, uint8_t pin_state);
uint8_t gpio_read_pin(GPIO_TypeDef *gpio_port, uint8_t pin_number);
void gpio_toggle_pin(GPIO_TypeDef *gpio_port, uint8_t pin_number);

#endif // GPIO_H

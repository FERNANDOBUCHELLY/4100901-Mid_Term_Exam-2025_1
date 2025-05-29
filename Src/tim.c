/**
 ******************************************************************************
 * @file           : tim.c
 * @author         : Sam C
 * @brief          : Timer driver for STM32L476RGTx
 ******************************************************************************
 */
#include "tim.h"
#include "rcc.h"  // Para rcc_tim3_clock_enable y TIM_PCLK_FREQ_HZ
#include "gpio.h" // Para configurar pin PB4

void tim3_ch1_pwm_init(uint32_t pwm_freq_hz)
{
    // Cambia PA6 por PB4, ambos usan AF2 para TIM3_CH1
    gpio_setup_pin(GPIOB, 4, GPIO_MODE_AF, 2); // <-- Cambiado a PB4

    // 2. Habilitar el reloj para TIM3
    rcc_tim3_clock_enable();

    // 3. Configurar TIM3
    TIM3->PSC = 100 - 1; // (4MHz / 100 = 40kHz)
    TIM3->ARR = (TIM_PCLK_FREQ_HZ / 100 / pwm_freq_hz) - 1; // 40kHz / pwm_freq_hz

    // Configurar el Canal 1 (CH1) en modo PWM 1
    TIM3->CCMR1 = (6U << 4);                    // PWM mode 1 on CH1
    TIM3->CCER  |= (1 << 0);                    // Enable CH1 output

    // Finalmente, habilitar el contador del timer (CEN bit en CR1)
    TIM3->CR1 |= 0x01 << 0;
}

void tim3_ch1_pwm_set_duty_cycle(uint8_t duty_cycle_percent)
{
    if (duty_cycle_percent > 100) {
        duty_cycle_percent = 100;
    }

    // Calcular el valor de CCR1 basado en el porcentaje y el valor de ARR guardado
    // CCR1 = ( (ARR + 1) * DutyCycle_Percent ) / 100
    // Cuidado con el orden de operaciones para evitar truncamiento prematuro.
    uint16_t tim3_ch1_arr_value = TIM3->ARR;
    uint32_t ccr_value = (((uint32_t)tim3_ch1_arr_value + 1U) * duty_cycle_percent) / 100U;

    TIM3->CCR1 = ccr_value;
}

uint32_t tim3_ch1_pwm_get_duty_cycle(void)
{
    uint16_t arr = TIM3->ARR;
    uint16_t ccr = TIM3->CCR1;
    if (arr == 0) return 0; // Evita división por cero
    return (ccr * 100U) / (arr + 1U);
}
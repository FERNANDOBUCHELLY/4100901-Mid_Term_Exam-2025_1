/**
 ******************************************************************************
 * @file           : room_control.c
 * @author         : Sam C
 * @brief          : Room control driver for STM32L476RGTx
 ******************************************************************************
 */
#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs y leer el botón (aunque el botón es por EXTI)
#include "systick.h" // Para obtener ticks y manejar retardos/tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM


static volatile uint32_t g_door_open_tick = 0;
static volatile uint8_t g_door_open = 0;
static volatile uint32_t g_last_button_tick = 0;

static uint32_t g_previous_pwm = 20; // Valor inicial igual al de init
static uint32_t g_pwm_override_tick = 0;
static uint8_t g_pwm_override_active = 0;

void room_control_app_init(void)
{
    gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_RESET);
    g_door_open = 0;
    g_door_open_tick = 0;

    tim3_ch1_pwm_set_duty_cycle(20); // Lámpara al 20%

    // Mensaje de bienvenida por UART
    uart2_send_string("\r\nControlador de Sala v1.0\r\n");
    uart2_send_string("Desarrollador: [LUIS FERNANDO CASTRO BUCHELLY]\r\n");
    uart2_send_string("Estado inicial:\r\n");
    uart2_send_string(" - Lampara: 20%\r\n");
    uart2_send_string(" - Puerta: Cerrada\r\n");
}

void room_control_on_button_press(void)
{
    uint32_t now = systick_get_tick();
    if (now - g_last_button_tick < 50) return;  // Anti-rebote de 50 ms
    g_last_button_tick = now;

    uart2_send_string("Evento: Botón presionado - Abriendo puerta.\r\n");

    gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_SET);
    g_door_open_tick = now;
    g_door_open = 1;

    // --- Lógica para la lámpara al 100% por 10 segundos ---
    if (!g_pwm_override_active) {
        // Guarda el brillo actual antes de sobrescribirlo
        g_previous_pwm = tim3_ch1_pwm_get_duty_cycle();
        tim3_ch1_pwm_set_duty_cycle(100);
        g_pwm_override_tick = now;
        g_pwm_override_active = 1;
    }
}

void room_control_on_uart_receive(char cmd)
{
     
    switch (cmd) {
        case '1':
            tim3_ch1_pwm_set_duty_cycle(100);
            uart2_send_string("Lampara: brillo al 100%.\r\n");
            break;

        case '2':
            tim3_ch1_pwm_set_duty_cycle(70);
            uart2_send_string("Lampara: brillo al 70%.\r\n");
            break;

        case '3':
            tim3_ch1_pwm_set_duty_cycle(50);
            uart2_send_string("Lampara: brillo al 50%.\r\n");
            break;

        case '4':
            tim3_ch1_pwm_set_duty_cycle(20);
            uart2_send_string("Lampara: brillo al 20%.\r\n");
            break;

        case '0':
            tim3_ch1_pwm_set_duty_cycle(0);
            uart2_send_string("Lampara apagada.\r\n");
            break;

        case 'o':
        case 'O':
            gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_SET);
            g_door_open_tick = systick_get_tick();
            g_door_open = 1;
            uart2_send_string("Puerta abierta remotamente.\r\n");
            break;

        case 'c':
        case 'C':
            gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_RESET);
            g_door_open = 0;
            uart2_send_string("Puerta cerrada remotamente.\r\n");
            break;

        default:
            uart2_send_string("Comando desconocido.\r\n");
            break;
            case 's':
        case 'S': {
            uart2_send_string("Estado:\r\n");

            // Lámpara: obtener duty actual
            uint32_t duty = tim3_ch1_pwm_get_duty_cycle();

            // Evita sprintf, usa un pequeño buffer y conversión manual
            char msg[32];
            // Convierte duty a texto (asumiendo duty <= 100)
            msg[0] = ' ';
            msg[1] = '-';
            msg[2] = ' ';
            msg[3] = 'L';
            msg[4] = 'a';
            msg[5] = 'm';
            msg[6] = 'p';
            msg[7] = 'a';
            msg[8] = 'r';
            msg[9] = 'a';
            msg[10] = ':';
            msg[11] = ' ';
            // Duty a texto
            if (duty >= 100) {
                msg[12] = '1';
                msg[13] = '0';
                msg[14] = '0';
                msg[15] = '%';
                msg[16] = '\r';
                msg[17] = '\n';
                msg[18] = '\0';
            } else if (duty >= 10) {
                msg[12] = '0' + (duty / 10);
                msg[13] = '0' + (duty % 10);
                msg[14] = '%';
                msg[15] = '\r';
                msg[16] = '\n';
                msg[17] = '\0';
            } else {
                msg[12] = '0' + duty;
                msg[13] = '%';
                msg[14] = '\r';
                msg[15] = '\n';
                msg[16] = '\0';
            }
            uart2_send_string(msg);

            // Puerta: estado real
            if (g_door_open)
                uart2_send_string(" - Puerta: Abierta\r\n");
            else
                uart2_send_string(" - Puerta: Cerrada\r\n");
            break;
        }
    }
}

void room_control_tick(void)
{
    // Control de puerta automática
    if (g_door_open && (systick_get_tick() - g_door_open_tick >= 3000)) {
        gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_RESET);
        uart2_send_string("Puerta cerrada automaticamente tras 3 segundos.\r\n");
        g_door_open = 0;
    }

    // --- Control de lámpara: volver al brillo anterior tras 10 segundos ---
    if (g_pwm_override_active && (systick_get_tick() - g_pwm_override_tick >= 10000)) {
        tim3_ch1_pwm_set_duty_cycle(g_previous_pwm);
        g_pwm_override_active = 0;
        uart2_send_string("Lampara: restaurado brillo anterior tras 10s.\r\n");
    }
}


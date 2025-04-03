
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <send.h>

static ADC_HandleTypeDef* hadc;
static UART_HandleTypeDef* huart;
volatile uint16_t adc_value;
volatile uint16_t adc_buffer[MOVING_AVG_SIZE];
volatile uint8_t _index = 0;
volatile uint8_t hall_state = 0;
void send_init(UART_HandleTypeDef* _huart,ADC_HandleTypeDef* _hadc){
    hadc = _hadc;
    huart = _huart;
    HAL_ADC_Start_DMA(hadc, (uint32_t*)&adc_value, 1);
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        adc_value = HAL_ADC_GetValue(hadc);
        adc_buffer[_index] = adc_value;
        _index = (_index + 1) % MOVING_AVG_SIZE;
    }
}

uint16_t moving_average() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < MOVING_AVG_SIZE; i++) {
        sum += adc_buffer[i];
    }
    return (uint16_t)(sum / MOVING_AVG_SIZE);
}

uint16_t add_random_noise(uint16_t value) {
    int8_t noise = (rand() % 21) - 10; // Random noise between -10 and +10
    return (uint16_t)(value + noise);
}

void send_data(const char *mode) {
    char msg[64];
    uint16_t value;
    if (strcmp(mode, "raw") == 0) {
        value = adc_value;
    } else if (strcmp(mode, "moving average") == 0) {
        value = moving_average();
    } else if (strcmp(mode, "random noise") == 0) {
        value = add_random_noise(adc_value);
    } else {
        return;
    }
    snprintf(msg, sizeof(msg), "Mode: %s, ADC: %u, Hall: %d\r\n", mode, value, hall_state);
    HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/* int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_value, 1);

    while (1) {
        send_data("raw");
        HAL_Delay(100);
    }
} */

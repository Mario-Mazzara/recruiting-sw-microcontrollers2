#ifndef CLI_H
#define CLI_H;

#include <stdint.h>
#include "stm32f4xx_hal.h"

extern volatile uint32_t CLI_Active, AVG_Active, Noise_Active;

void cli_init(UART_HandleTypeDef* _huart);

/**
  * @brief  Process the received command.
  * @param  cmd: Null-terminated string containing the command.
  * @retval None
  */
void processCommand(char *cmd);

/**
  * @brief  UART receive complete callback.
  *         Called each time a byte is received.
  * @param  huart: pointer to the UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
#endif // !CLI_H

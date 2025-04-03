#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
volatile uint32_t CLI_Active = 0, AVG_Active = 0, Noise_Active = 0;

#define CMD_BUFFER_SIZE 128

static UART_HandleTypeDef* huart;   // USART2 handle, configured via CubeMX
uint8_t rxByte;              // Single-byte reception buffer
char commandBuffer[CMD_BUFFER_SIZE]; // Command buffer for accumulating characters
volatile uint16_t cmdIndex = 0;      // Current index into the command buffer

void cli_init(UART_HandleTypeDef* _huart){
    // Start UART reception in interrupt mode (receive one byte at a time)
    huart = _huart;
    HAL_UART_Receive_IT(huart, &rxByte, 1);
    CLI_Active = 0;
}

void processCommand(char *cmd)
{
    /* char response[128];
    snprintf(response, sizeof(response), "Received command: %s\r\n", cmd);
    HAL_UART_Transmit(huart, (uint8_t *)response, strlen(response), HAL_MAX_DELAY); */

    // Here, add your command parsing and handling logic.
    // For example:
    if(strcmp(cmd, "raw") == 0) {
        AVG_Active = 0;
        Noise_Active = 0;
    }
    else if(strcmp(cmd, "moving average") == 0) {
        Noise_Active = 0;
        AVG_Active = 1;
    }
else if(strcmp(cmd, "random noise") == 0) {
        AVG_Active = 0;
        Noise_Active = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // Check for end-of-line characters (CR or LF)
        if (rxByte == '\r' || rxByte == '\n')
        {
            commandBuffer[cmdIndex] = '\0';

            // Process command if there's any content
            if (cmdIndex > 0 && CLI_Active != 0)
            {
                processCommand(commandBuffer);
            }

            // Reset the buffer index for the next command
            cmdIndex = 0;
        }
        else
        {
            // Store received byte in command buffer (if there is room)
            if (cmdIndex < CMD_BUFFER_SIZE - 1)
            {
                commandBuffer[cmdIndex++] = rxByte;
            }
            else
            {
                // Buffer overflow handling: reset the index (or you could clear the buffer)
                cmdIndex = 0;
            }
        }

        // Restart UART reception to get the next byte
        HAL_UART_Receive_IT(huart, &rxByte, 1);
    }
}


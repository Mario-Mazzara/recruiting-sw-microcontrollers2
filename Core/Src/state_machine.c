#include "state_machine.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"
#include "defines.h"

static TIM_HandleTypeDef* htim;
static UART_HandleTypeDef* huart;

__IO uint32_t BspButtonState = BUTTON_RELEASED;

void Handle_Error(void){
  sm_update(EVENT_ERROR_OCCURRED);
  uint8_t MSG[] = "ERROR";
  while (1)
  {
    HAL_UART_Transmit(huart,MSG,sizeof(MSG),100);
    if (BspButtonState == BUTTON_PRESSED)
    {
        NVIC_SystemReset();
    }
  }
}

void LED_PWM_Start(void) {
    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1) != HAL_OK)
    {
        Handle_Error();
    }
}

void LED_PWM_Stop(void) {
    if (HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1) != HAL_OK)
    {
        Handle_Error();
    }
}
StateTransition_t transitions[]={
    {STATE_INIT,EVENT_NONE,STATE_WAIT_REQUEST,NULL},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WAIT_REQUEST,EVENT_BUTTON_PRESS,STATE_LISTENING,NULL},
    {STATE_WAIT_REQUEST,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_LISTENING,EVENT_BUTTON_PRESS,STATE_PAUSE,LED_PWM_Start},
    {STATE_LISTENING,EVENT_SENSOR_STALL,STATE_WARNING,NULL},
    {STATE_LISTENING,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_PAUSE,EVENT_BUTTON_PRESS,STATE_LISTENING,LED_PWM_Stop},
    {STATE_PAUSE,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WARNING,EVENT_BUTTON_PRESS,STATE_WAIT_REQUEST,NULL},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},
};

State_t currentState = STATE_INIT;

void sm_init(TIM_HandleTypeDef* _htim, UART_HandleTypeDef* _huart){
    htim = _htim;
    huart = _huart;
    currentState = STATE_INIT;
}

void sm_update(Event_t event){
    for (size_t i=0;i<sizeof(transitions)/sizeof(transitions[0]);i++) {
        if(transitions[i].startingState==currentState
                && transitions[i].event==event){
            if(transitions[i].action != NULL) transitions[i].action();

            currentState= transitions[i].nextState;
        }
    }
    return;
}

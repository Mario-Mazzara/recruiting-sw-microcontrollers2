#include "state_machine.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"
#include "defines.h"
#include "cli.h"

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

void CLI_start(){
    CLI_Active = 1;
}
void CLI_stop(){
    CLI_Active = 0;
}

void listening_handler(void){}
void pause_handler(void){}
void wait_request_handler(void){}
void warning_handler(void){}

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

void Pause_in(){
    LED_PWM_Start();
    CLI_start();
}
void Pause_out(){
    LED_PWM_Stop();
    CLI_stop();
}

StateTransition_t transitions[]={
    {STATE_INIT,EVENT_NONE,STATE_WAIT_REQUEST,CLI_start},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WAIT_REQUEST,EVENT_BUTTON_PRESS,STATE_LISTENING,CLI_stop},
    {STATE_WAIT_REQUEST,EVENT_ERROR_OCCURRED,STATE_ERROR,CLI_stop},

    {STATE_LISTENING,EVENT_BUTTON_PRESS,STATE_PAUSE,Pause_in},
    {STATE_LISTENING,EVENT_SENSOR_STALL,STATE_WARNING,CLI_stop},
    {STATE_LISTENING,EVENT_ERROR_OCCURRED,STATE_ERROR,CLI_stop},

    {STATE_PAUSE,EVENT_BUTTON_PRESS,STATE_LISTENING,Pause_out},
    {STATE_PAUSE,EVENT_ERROR_OCCURRED,STATE_ERROR,Pause_out},

    {STATE_WARNING,EVENT_BUTTON_PRESS,STATE_WAIT_REQUEST,CLI_start},
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

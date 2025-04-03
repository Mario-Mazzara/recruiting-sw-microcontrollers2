#include "state_machine.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"
#include "defines.h"
#include "cli.h"
#include <string.h>
#include <send.h>
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

void listening_handler(void){

    if(AVG_Active && !Noise_Active){
        send_data("moving average");
    }else if ( !AVG_Active && Noise_Active ){
        send_data("random noise");
    }else {
        send_data("raw");
    }
    HAL_Delay(100);
}
void pause_handler(void){
    //HAL_UART_Transmit(huart,"Inside pause",20,100);
}
void wait_request_handler(void){
    //HAL_UART_Transmit(huart,"Inside Wait Request",20,100);
}
void warning_handler(void){
    HAL_UART_Transmit(huart,"WARNING",7,100);
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

void Pause_in(){
    LED_PWM_Start();
    CLI_start();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}
void Pause_out(){
    LED_PWM_Stop();
    CLI_stop();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
const char* stateToString(State_t state) {
    switch (state) {
        case STATE_INIT: return "STATE_INIT";
        case STATE_WAIT_REQUEST: return "STATE_WAIT_REQUEST";
        case STATE_LISTENING: return "STATE_LISTENING";
        case STATE_PAUSE: return "STATE_PAUSE";
        case STATE_WARNING: return "STATE_WARNING";
        case STATE_ERROR: return "STATE_ERROR";
        default: return "UNKNOWN_STATE";
    }
}

const char* eventToString(Event_t event) {
    switch (event) {
        case EVENT_NONE: return "EVENT_NONE";
        case EVENT_BUTTON_PRESS: return "EVENT_BUTTON_PRESS";
        case EVENT_SENSOR_STALL: return "EVENT_SENSOR_STALL";
        case EVENT_ERROR_OCCURRED: return "EVENT_ERROR_OCCURRED";
        default: return "UNKNOWN_EVENT";
    }
}
void printState(State_t state) {
    const char* stateStr = stateToString(state);
    HAL_UART_Transmit(huart, (uint8_t*)stateStr, strlen(stateStr), HAL_MAX_DELAY);
    HAL_UART_Transmit(huart, (uint8_t*)"\n", 1, HAL_MAX_DELAY); // New line for better readability
}

void printEvent(Event_t event) {
    const char* eventStr = eventToString(event);
    HAL_UART_Transmit(huart, (uint8_t*)eventStr, strlen(eventStr), HAL_MAX_DELAY);
    HAL_UART_Transmit(huart, (uint8_t*)"\n\r", 2, HAL_MAX_DELAY); // New line for better readability
}
void wait_to_listen(){
    CLI_stop();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

StateTransition_t transitions[]={
    {STATE_INIT,EVENT_NONE,STATE_WAIT_REQUEST,CLI_start},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WAIT_REQUEST,EVENT_BUTTON_PRESS,STATE_LISTENING,wait_to_listen},
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

            /* printEvent(event);
            printState(transitions[i].startingState);
            printState(transitions[i].nextState);
            HAL_UART_Transmit(huart,"\r",1,100); */
            break;
        }
    }
    return;
}

#ifndef SM_H
#define SM_H

#include "stm32f4xx_hal.h"

typedef enum {
    STATE_INIT,
    STATE_WAIT_REQUEST,
    STATE_LISTENING,
    STATE_PAUSE,
    STATE_WARNING,
    STATE_ERROR,
} State_t;

typedef enum {
    EVENT_NONE,
    EVENT_BUTTON_PRESS,
    EVENT_SENSOR_STALL,
    EVENT_ERROR_OCCURRED,
} Event_t;


typedef struct {
    State_t startingState;
    Event_t event;
    State_t nextState;
    void (*action)(void);
} StateTransition_t;



extern State_t currentState;
extern StateTransition_t transitions[];


/**
 * \brief State machine initialization
 */
void sm_init(TIM_HandleTypeDef* _htim, UART_HandleTypeDef* _huart);
/**
 * \brief updates the state machine given an event
 * \param event, event that triggers state the transition
 */
void sm_update(Event_t event);

void Handle_Error(void);
#endif

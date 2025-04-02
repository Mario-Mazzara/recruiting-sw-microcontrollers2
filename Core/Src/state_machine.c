#include "state_machine.h"
#include <stddef.h>

void enablePWM_timer(void){

}

void disablePWM_timer(void){

}
StateTransition_t transitions[]={
    {STATE_INIT,EVENT_NONE,STATE_WAIT_REQUEST,NULL},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WAIT_REQUEST,EVENT_BUTTON_PRESS,STATE_LISTENING,NULL},
    {STATE_WAIT_REQUEST,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_LISTENING,EVENT_BUTTON_PRESS,STATE_PAUSE,enablePWM_timer},
    {STATE_LISTENING,EVENT_SENSOR_STALL,STATE_WARNING,NULL},
    {STATE_LISTENING,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_PAUSE,EVENT_BUTTON_PRESS,STATE_LISTENING,disablePWM_timer},
    {STATE_PAUSE,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},

    {STATE_WARNING,EVENT_BUTTON_PRESS,STATE_WAIT_REQUEST,NULL},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR,NULL},
};

State_t currentState = STATE_INIT;

void sm_init(){
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

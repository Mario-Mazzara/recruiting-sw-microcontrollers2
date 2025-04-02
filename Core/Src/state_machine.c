#include "state_machine.h"
#include <stddef.h>

StateTransition_t transitions[]={
    {STATE_INIT,EVENT_NONE,STATE_WAIT_REQUEST},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR},

    {STATE_WAIT_REQUEST,EVENT_BUTTON_PRESS,STATE_LISTENING},
    {STATE_WAIT_REQUEST,EVENT_ERROR_OCCURRED,STATE_ERROR},

    {STATE_LISTENING,EVENT_BUTTON_PRESS,STATE_PAUSE},
    {STATE_LISTENING,EVENT_SENSOR_STALL,STATE_WARNING},
    {STATE_LISTENING,EVENT_ERROR_OCCURRED,STATE_ERROR},

    {STATE_PAUSE,EVENT_BUTTON_PRESS,STATE_LISTENING},
    {STATE_PAUSE,EVENT_ERROR_OCCURRED,STATE_ERROR},

    {STATE_WARNING,EVENT_BUTTON_PRESS,STATE_WAIT_REQUEST},
    {STATE_INIT,EVENT_ERROR_OCCURRED,STATE_ERROR},
};

State_t currentState = STATE_INIT;

void sm_init(){
    currentState = STATE_INIT;
}

void sm_update(Event_t event){
    for (size_t i=0;i<sizeof(transitions)/sizeof(transitions[0]);i++) {
        if(transitions[i].startingState==currentState
                && transitions[i].event==event){
            currentState= transitions[i].nextState;
        }
    }
    return;
}

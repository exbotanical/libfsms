#include <stdio.h>
#include <stdlib.h>

#include "libfsms.h"

// off : {
//   transitions : {
//     switch : {
//       target: 'on'
//       action : fn
//     }
//   }
// }
// on : {
//   transitions : {
//     switch : {
//       target: 'off'
//       action : fn
//     }
//   }
// }
const char* TRANSITION_NAME = "switch";

const char* ON_STATE = "on";
const char* OFF_STATE = "off";

void on_action_handler() {}
void off_action_handler() {}

int main() {
  StateMachine* fsm = fsm_create("test", NULL);
  fsm->state = OFF_STATE;

  // TODO: read from file config
  StateDescriptor* off_s = fsm_state_create(fsm, OFF_STATE);
  StateDescriptor* on_s = fsm_state_create(fsm, ON_STATE);

  fsm_transition_create(fsm, TRANSITION_NAME, off_s, ON_STATE, NULL,
                        on_action_handler);

  fsm_transition_create(fsm, TRANSITION_NAME, on_s, OFF_STATE, NULL,
                        off_action_handler);

  printf("starting state: %s\n", fsm->state);
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm->state);
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm->state);
}

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

const char* ON_STATE        = "on";
const char* OFF_STATE       = "off";

void
on_action_handler ()
{
}

void
off_action_handler ()
{
}

int
main ()
{
  state_machine_t* switch1  = fsm_create("switch1", NULL);

  // TODO: read from file config
  state_descriptor_t* off_s = fsm_state_create(OFF_STATE);
  state_descriptor_t* on_s  = fsm_state_create(ON_STATE);
  fsm_state_register(switch1, off_s);
  fsm_state_register(switch1, on_s);

  fsm_set_initial_state(switch1, off_s);

  transition_t* t1
    = fsm_transition_create(TRANSITION_NAME, on_s, NULL, on_action_handler);
  fsm_transition_register(switch1, off_s, t1);

  transition_t* t2
    = fsm_transition_create(TRANSITION_NAME, off_s, NULL, off_action_handler);
  fsm_transition_register(switch1, on_s, t2);

  state_machine_t* switch2 = fsm_clone("switch2", NULL, switch1);
  fsm_set_initial_state(switch2, on_s);

  printf("[switch1] starting state: %s\n", fsm_get_state_name(switch1));
  printf("[switch2] starting state: %s\n", fsm_get_state_name(switch2));

  fsm_transition(switch1, TRANSITION_NAME);
  fsm_transition(switch2, TRANSITION_NAME);

  printf("[switch1] current state: %s\n", fsm_get_state_name(switch1));
  printf("[switch2] current state: %s\n", fsm_get_state_name(switch2));

  fsm_transition(switch1, TRANSITION_NAME);
  printf("[switch1] ending state: %s\n", fsm_get_state_name(switch1));
  printf("[switch2] ending state: %s\n", fsm_get_state_name(switch2));

  fsm_state_free(off_s);
  fsm_state_free(on_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(switch1);
  fsm_free(switch2);

  printf("done\n");
}

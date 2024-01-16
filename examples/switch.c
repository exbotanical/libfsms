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
  state_machine_t* fsm      = fsm_create("test", NULL);

  // TODO: read from file config
  state_descriptor_t* off_s = fsm_state_create(OFF_STATE);
  state_descriptor_t* on_s  = fsm_state_create(ON_STATE);

  fsm_state_register(fsm, off_s);
  fsm_state_register(fsm, on_s);

  fsm_set_initial_state(fsm, off_s);

  transition_t* t1
    = fsm_transition_create(TRANSITION_NAME, on_s, NULL, on_action_handler);
  fsm_transition_register(fsm, off_s, t1);

  transition_t* t2
    = fsm_transition_create(TRANSITION_NAME, off_s, NULL, off_action_handler);
  fsm_transition_register(fsm, on_s, t2);

  printf("starting state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm_get_state_name(fsm));

  fsm_state_free(on_s);
  fsm_state_free(off_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(fsm);

  printf("done\n");
}

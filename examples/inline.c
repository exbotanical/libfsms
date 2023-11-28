#include <stdio.h>
#include <stdlib.h>

#include "libfsms.h"

const char* TRANSITION_NAME = "switch";

void*
on_transition (void* ctx)
{
}

int
main (int argc, char const* argv[])
{
  StateMachine* fsm = fsm_inline(
    "test",
    "PENDING",
    fsm_inline_states({"PENDING", "READY", "RUNNING", "DONE"}),
    &(InlineTransition){
      .name   = TRANSITION_NAME,
      .source = "PENDING",
      .target = "READY",
      .action = on_transition,
    },
    &(InlineTransition){
      .name   = TRANSITION_NAME,
      .source = "READY",
      .target = "RUNNING",
      .action = on_transition},
    &(InlineTransition){
      .name   = TRANSITION_NAME,
      .source = "RUNNING",
      .target = "DONE",
      .action = on_transition}
  );

  printf("starting state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm_get_state_name(fsm));

  printf("done\n");

  return 0;
}

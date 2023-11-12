#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libfsms.h"

static StateDescriptor* get_state(StateMachine* fsm, char* name) {
  foreach (fsm->states, i) {
    StateDescriptor* s = array_get(fsm->states, i);
    if (s_equals(s->name, name)) {
      return s;
    }
  }
  return NULL;
}

static void register_state(StateMachine* fsm, StateDescriptor* s) {
  array_push(fsm->states, s);
}

static void fsm_register_transition(StateMachine* fsm, char* state_name,
                                    Transition* t) {
  StateDescriptor* state = get_state(fsm, state_name);
  array_push(state->transitions, t);
}

StateMachine* fsm_create(char* name, void* context) {
  StateMachine* fsm = malloc(sizeof(StateMachine));
  fsm->name = name;
  fsm->context = context;
  fsm->states = array_init();

  return fsm;
}

StateDescriptor* fsm_state_create(StateMachine* fsm, char* name) {
  StateDescriptor* s = malloc(sizeof(StateDescriptor));
  s->name = name;
  s->transitions = array_init();

  register_state(fsm, s);

  return s;
}

void fsm_transition_create(StateMachine* fsm, char* name,
                           StateDescriptor* source, StateDescriptor* target,
                           bool (*cond)(void*), void (*action)(void*)) {
  Transition* t = malloc(sizeof(Transition));
  t->name = name;
  t->target = target;
  t->cond = cond;
  t->action = action;

  fsm_register_transition(fsm, source->name, t);
}

void fsm_transition(StateMachine* fsm, char* event) {
  StateDescriptor* s = get_state(fsm, fsm->state);

  foreach (s->transitions, i) {
    Transition* t = array_get(s->transitions, i);

    if (s_equals(t->name, event)) {
      if (t->cond && !(t->cond(fsm->context))) {
        return;
      }

      if (t->action) {
        t->action(fsm->context);
      }

      fsm->state = t->target->name;
    }
  }
}

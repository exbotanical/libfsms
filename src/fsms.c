#include "fsms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StateMachine* fsm_create(char* name, void* context) {
  StateMachine* fsm = malloc(sizeof(*fsm));
  fsm->name = name;
  fsm->context = context;

  return fsm;
}

static void register_state(StateMachine* fsm, StateDescriptor* s) {
  fsm->states[fsm->num_states] = malloc(sizeof(s));
  fsm->states[fsm->num_states++] = s;
}

StateDescriptor* get_state(StateMachine* fsm, char* name) {
  for (int i = 0; i < fsm->num_states; i++) {
    if (strcmp(fsm->states[i]->name, name) == 0) {
      return fsm->states[i];
    }
  }
  return NULL;
}

StateDescriptor* fsm_state_create(StateMachine* fsm, char* name) {
  StateDescriptor* s = malloc(sizeof(*s));
  s->name = name;

  register_state(fsm, s);

  return s;
}

void fsm_register_transition(StateMachine* fsm, char* state_name,
                             Transition* t) {
  StateDescriptor* state = get_state(fsm, state_name);

  state->transitions[state->num_transitions] = malloc(sizeof(t));
  state->transitions[state->num_transitions++] = t;
}

Transition* fsm_transition_create(char* name, char* target,
                                  PredicateFunction* cond,
                                  void*(action)(void)) {
  Transition* t = malloc(sizeof(*t));
  t->name = name;
  t->target = target;
  t->cond = cond;
  t->action = action;

  return t;
}

void fsm_transition(StateMachine* fsm, char* event) {
  StateDescriptor* s = get_state(fsm, fsm->state);

  for (int k = 0; k < s->num_transitions; k++) {
    Transition* t = s->transitions[k];

    if (strcmp(t->name, event) == 0 && t->cond(fsm->context)) {
      t->action();
      fsm->state = t->target;
    }
  }
}

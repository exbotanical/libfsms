#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libfsms.h"

static StateDescriptor *
get_state (StateMachine *fsm, const char *name)
{
  foreach (fsm->states, i) {
    StateDescriptor *s = array_get(fsm->states, i);
    if (s_equals(s->name, name)) {
      return s;
    }
  }
  return NULL;
}

StateMachine *
fsm_create (const char *name, void *context)
{
  StateMachine *fsm = malloc(sizeof(StateMachine));
  fsm->name         = name;
  fsm->context      = context;
  fsm->states       = array_init();
  fsm->state        = NULL;

  return fsm;
}

void
fsm_free (StateMachine *fsm)
{
  fsm->context = NULL;
  fsm->name    = NULL;
  fsm->state   = NULL;
  array_free(fsm->states);
  fsm->states = NULL;
  free(fsm);
  fsm = NULL;
}

void
fsm_set_initial_state (StateMachine *fsm, StateDescriptor *s)
{
  fsm->state = s;
}

const char *
fsm_get_state_name (StateMachine *fsm)
{
  return fsm->state->name;
}

StateDescriptor *
fsm_state_create (const char *name)
{
  StateDescriptor *s = malloc(sizeof(StateDescriptor));
  s->name            = name;
  s->transitions     = array_init();

  return s;
}

StateDescriptor *
fsm_state_register (StateMachine *fsm, StateDescriptor *s)
{
  array_push(fsm->states, s);
  return s;
}

void
fsm_state_free (StateDescriptor *s)
{
  array_free(s->transitions);
  s->name        = NULL;
  s->transitions = NULL;

  free(s);
  s = NULL;
}

Transition *
fsm_transition_create (
  const char      *name,
  StateDescriptor *target,
  bool (*cond)(void *),
  void (*action)(void *)
)
{
  Transition *t = malloc(sizeof(Transition));
  t->name       = name;
  t->target     = target;
  t->cond       = cond;
  t->action     = action;

  return t;
}

Transition *
fsm_transition_register (
  StateMachine    *fsm,
  StateDescriptor *source,
  Transition      *t
)
{
  StateDescriptor *state = get_state(fsm, source->name);
  array_push(state->transitions, t);

  return t;
}

void
fsm_transition_free (Transition *t)
{
  t->action = NULL;
  t->cond   = NULL;
  t->name   = NULL;
  t->target = NULL;
  free(t);
  t = NULL;
}

void
fsm_transition (StateMachine *fsm, const char *const event)
{
  StateDescriptor *curr_s = fsm->state;

  foreach (curr_s->transitions, i) {
    Transition *t = array_get(curr_s->transitions, i);

    if (s_equals(t->name, event)) {
      if (t->cond && !(t->cond(fsm->context))) {
        return;
      }

      if (t->action) {
        t->action(fsm->context);
      }

      fsm->state = t->target;
    }
  }
}

StateMachine *
fsm_clone (const char *name, void *context, StateMachine *source)
{
  StateMachine *clone = fsm_create(name, context);

  foreach (source->states, i) {
    StateDescriptor *source_s = array_get(source->states, i);
    fsm_state_register(clone, source_s);
  }

  return clone;
}

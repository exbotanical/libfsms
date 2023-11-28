#include <stdarg.h>
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
  bool (*guard)(void *),
  void (*action)(void *)
)
{
  Transition *t = malloc(sizeof(Transition));
  t->name       = name;
  t->target     = target;
  t->guard      = guard;
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
  t->guard  = NULL;
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
      if (t->guard && !(t->guard(fsm->context))) {
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

static bool
find_by_name (StateDescriptor *el, char *compare_to)
{
  return s_equals(el->name, compare_to);
}

StateMachine *
__fsm_inline (
  const char       *name,
  const char       *initial_state,
  char            **states,
  int               num_states,
  InlineTransition *t,
  ...
)
{
  StateMachine *fsm = fsm_create(name, NULL);

  for (int i = 0; i < num_states; i++) {
    fsm_state_register(fsm, fsm_state_create(states[i]));
  }

  int initial_sd_idx
    = array_find(fsm->states, (comparator_t *)find_by_name, initial_state);
  if (initial_sd_idx == -1) {
    return NULL;
  }

  StateDescriptor *initial_sd = array_get(fsm->states, initial_sd_idx);
  if (!initial_sd) {
    return NULL;
  }

  fsm_set_initial_state(fsm, initial_sd);

  va_list args;
  va_start(args, t);

  do {
    int source_idx
      = array_find(fsm->states, (comparator_t *)find_by_name, t->source);
    int target_idx
      = array_find(fsm->states, (comparator_t *)find_by_name, t->target);

    if (source_idx == -1 || target_idx == -1) {
      return NULL;
    }

    StateDescriptor *source = array_get(fsm->states, source_idx);
    StateDescriptor *target = array_get(fsm->states, target_idx);

    if (!source || !target) {
      return NULL;
    }

    fsm_transition_register(
      fsm,
      source,
      fsm_transition_create(t->name, target, t->guard, t->action)
    );
  } while ((t = va_arg(args, InlineTransition *)));

  va_end(args);

  return fsm;
}

void
__fsm_inline_free (StateMachine *fsm)
{
  foreach (fsm->states, i) {
    StateDescriptor *s = array_get(fsm->states, i);
    foreach (s->transitions, j) {
      Transition *t = array_get(s->transitions, j);

      fsm_transition_free(t);
    }
    fsm_state_free(s);
  }

  fsm_free(fsm);
}

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libfsms.h"

static void *
xmalloc (size_t sz)
{
  void *ptr;
  if ((ptr = malloc(sz)) == NULL) {
    fprintf(stderr, "malloc failed to allocate memory\n");
    exit(EXIT_FAILURE);
  }

  return ptr;
}

static state_descriptor_t *
get_state (state_machine_t *fsm, const char *name)
{
  foreach (fsm->states, i) {
    state_descriptor_t *s = array_get(fsm->states, i);
    if (s_equals(s->name, name)) {
      return s;
    }
  }
  return NULL;
}

state_machine_t *
fsm_create (const char *name, void *context)
{
  state_machine_t *fsm = xmalloc(sizeof(state_machine_t));
  fsm->name            = name;
  fsm->context         = context;
  fsm->subscribers     = array_init();
  fsm->states          = array_init();
  fsm->state           = NULL;

  return fsm;
}

void
fsm_subscribe (state_machine_t *fsm, void *(*subscriber)(void *))
{
  array_push(fsm->subscribers, subscriber);
}

void
fsm_free (state_machine_t *fsm)
{
  fsm->context = NULL;
  fsm->name    = NULL;
  fsm->state   = NULL;
  array_free(fsm->subscribers);
  fsm->subscribers = NULL;
  array_free(fsm->states);
  fsm->states = NULL;
  free(fsm);
  fsm = NULL;
}

void
fsm_set_initial_state (state_machine_t *fsm, state_descriptor_t *s)
{
  fsm->state = s;
}

const char *
fsm_get_state_name (state_machine_t *fsm)
{
  return fsm->state->name;
}

state_descriptor_t *
fsm_state_create (const char *name)
{
  state_descriptor_t *s = xmalloc(sizeof(state_descriptor_t));
  s->name               = name;
  s->transitions        = array_init();

  return s;
}

state_descriptor_t *
fsm_state_register (state_machine_t *fsm, state_descriptor_t *s)
{
  array_push(fsm->states, s);
  return s;
}

void
fsm_state_free (state_descriptor_t *s)
{
  array_free(s->transitions);
  s->name        = NULL;
  s->transitions = NULL;

  free(s);
  s = NULL;
}

transition_t *
fsm_transition_create (
  const char         *name,
  state_descriptor_t *target,
  bool (*guard)(void *),
  void (*action)(void *)
)
{
  transition_t *t = xmalloc(sizeof(transition_t));
  t->name         = name;
  t->target       = target;
  t->guard        = guard;
  t->action       = action;

  return t;
}

transition_t *
fsm_transition_register (
  state_machine_t    *fsm,
  state_descriptor_t *source,
  transition_t       *t
)
{
  state_descriptor_t *state = get_state(fsm, source->name);
  array_push(state->transitions, t);

  return t;
}

void
fsm_transition_free (transition_t *t)
{
  t->action = NULL;
  t->guard  = NULL;
  t->name   = NULL;
  t->target = NULL;
  free(t);
  t = NULL;
}

void
fsm_transition (state_machine_t *fsm, const char *const event)
{
  state_descriptor_t *curr_s = fsm->state;

  foreach (curr_s->transitions, i) {
    transition_t *t = array_get(curr_s->transitions, i);

    if (s_equals(t->name, event)) {
      if (t->guard && !(t->guard(fsm->context))) {
        return;
      }

      if (t->action) {
        t->action(fsm->context);
      }

      transition_subscriber_args_t *s = NULL;

      if (has_elements(fsm->subscribers)) {
        s       = xmalloc(sizeof(transition_subscriber_args_t));
        s->prev = s_copy(fsm->state->name);
        s->next = s_copy(t->target->name);
        s->ev   = s_copy(event);
      }

      fsm->state = t->target;

      foreach (fsm->subscribers, i) {
        void *(*subscriber)(void *) = array_get(fsm->subscribers, i);
        subscriber(s);
      }
    }
  }
}

state_machine_t *
fsm_clone (const char *name, void *context, state_machine_t *source)
{
  state_machine_t *clone = fsm_create(name, context);

  foreach (source->states, i) {
    state_descriptor_t *source_s = array_get(source->states, i);
    fsm_state_register(clone, source_s);
  }

  return clone;
}

static bool
find_by_name (state_descriptor_t *el, char *compare_to)
{
  return s_equals(el->name, compare_to);
}

state_machine_t *
__fsm_inline (
  const char          *name,
  const char          *initial_state,
  char               **states,
  int                  num_states,
  inline_transition_t *t,
  ...
)
{
  state_machine_t *fsm = fsm_create(name, NULL);

  for (int i = 0; i < num_states; i++) {
    fsm_state_register(fsm, fsm_state_create(states[i]));
  }

  int initial_sd_idx
    = array_find(fsm->states, (comparator_t *)find_by_name, initial_state);
  if (initial_sd_idx == -1) {
    return NULL;
  }

  state_descriptor_t *initial_sd = array_get(fsm->states, initial_sd_idx);
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

    state_descriptor_t *source = array_get(fsm->states, source_idx);
    state_descriptor_t *target = array_get(fsm->states, target_idx);

    if (!source || !target) {
      return NULL;
    }

    fsm_transition_register(
      fsm,
      source,
      fsm_transition_create(t->name, target, t->guard, t->action)
    );
  } while ((t = va_arg(args, inline_transition_t *)));

  va_end(args);

  return fsm;
}

void
__fsm_inline_free (state_machine_t *fsm)
{
  foreach (fsm->states, i) {
    state_descriptor_t *s = array_get(fsm->states, i);
    foreach (s->transitions, j) {
      transition_t *t = array_get(s->transitions, j);

      fsm_transition_free(t);
    }
    fsm_state_free(s);
  }

  fsm_free(fsm);
}

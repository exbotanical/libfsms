#ifndef LIBFSMS_H
#define LIBFSMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "libutil/libutil.h"

#define GET_CONTEXT(tt, fsm) ((tt *)fsm->context)

typedef struct {
  const char *name;
  array_t    *transitions;
} state_descriptor_t;

typedef struct {
  const char         *name;
  state_descriptor_t *target;
  void (*action)(void *context);
  bool (*guard)(void *context);
} transition_t;

typedef struct {
  const char *name;
  const char *target;
  const char *source;
  void (*action)(void *context);
  bool (*guard)(void *context);
} inline_transition_t;

typedef struct {
  const char         *name;
  void               *context;
  array_t            *subscribers;
  array_t            *states;
  state_descriptor_t *state;
} state_machine_t;

typedef struct {
  const char *prev;
  const char *next;
  const char *ev;
} transition_subscriber_args_t;

// Creates a function `<name>` that returns the fsm context as the type `<tt>`.
#define CREATE_CONTEXT_GETTER(name, tt) \
  tt *name(state_machine_t *fsm)        \
  {                                     \
    return (tt *)(fsm->context);        \
  }

#define CREATE_MUTATOR(name, tt, code) \
  tt *name(state_machine_t *fsm)       \
  {                                    \
    tt *ctx = (tt *)(fsm->context);    \
    ctx->code                          \
  }

// creates a conditional function `name` that runs a condition which if true
// mutates a context type `tt` with the member accessor `code` e.g.
// CREATE_CONDITIONAL_MUTATOR(mutate_if, MyContext, some_member++) ->
// MyContext->some_member++ `code` only runs if the result of calling
// `mutate_if` is true

// e.g. mutate_if(fsm, some_predicate) where `some_predicate` is passed as an
// argument `fsm->context`
#define CREATE_CONDITIONAL_MUTATOR(name, tt, code)         \
  void name(state_machine_t *fsm, bool (*guard)(tt * ctx)) \
  {                                                        \
    tt *ctx = (tt *)(fsm->context);                        \
    if (!guard(ctx)) return;                               \
    ctx->code                                              \
  }

/**
 * Create a new state machine
 *
 * @param name A name for identifying the state machine
 * @param context A context object holding metadata, will be passed into
 * actions and subscribers. May be NULL.
 * @return state_machine_t*
 */
state_machine_t *fsm_create(const char *name, void *context);

/**
 * Free the given state machine
 *
 * @param fsm
 */
void fsm_free(state_machine_t *fsm);

/**
 * Subscribe a callback to the given state machine's state transitions.
 * Does not fire when a transition is cancelled due to a guard condition.
 * @param fsm
 * @param subscriber
 */
void fsm_subscribe(state_machine_t *fsm, void *(*subscriber)(void *));

/**
 * Set the state machine's initial state.
 * TODO: make fsm_create param since this is required...OR set to first
 * registered state
 *
 * @param fsm
 * @param s
 */
void fsm_set_initial_state(state_machine_t *fsm, state_descriptor_t *s);

const char *fsm_get_state_name(state_machine_t *fsm);

state_descriptor_t *fsm_state_create(const char *name);

state_descriptor_t *
fsm_state_register(state_machine_t *fsm, state_descriptor_t *s);

void fsm_state_free(state_descriptor_t *s);

transition_t *fsm_transition_create(
  const char         *name,
  state_descriptor_t *target,
  bool (*guard)(void *),
  void (*action)(void *)
);

transition_t *fsm_transition_register(
  state_machine_t    *fsm,
  state_descriptor_t *source,
  transition_t       *t
);

void fsm_transition_free(transition_t *t);

void fsm_transition(state_machine_t *fsm, const char *const event);

state_machine_t *
fsm_clone(const char *name, void *context, state_machine_t *source);

state_machine_t *__fsm_inline(
  const char          *name,
  const char          *initial_state,
  char                *states[],
  int                  num_states,
  inline_transition_t *t,
  ...
);

// TODO: improve
#define fsm_inline_states(...) \
  ((const char *[])__VA_ARGS__), sizeof((char *[])__VA_ARGS__) / sizeof(char *)

#define fsm_inline(name, initial_state, states, num_states, ...) \
  __fsm_inline(name, initial_state, states, num_states, __VA_ARGS__, NULL)

void fsm_inline_free(state_machine_t *fsm);

#ifdef __cplusplus
}
#endif

#endif /* LIBFSMS_H */

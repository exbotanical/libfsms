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
} StateDescriptor;

typedef struct {
  const char      *name;
  StateDescriptor *target;
  void (*action)(void *context);
  bool (*guard)(void *context);
} Transition;

typedef struct {
  const char *name;
  const char *target;
  const char *source;
  void (*action)(void *context);
  bool (*guard)(void *context);
} InlineTransition;

typedef struct {
  const char      *name;
  void            *context;
  array_t         *states;
  StateDescriptor *state;
} StateMachine;

// Creates a function `<name>` that returns the fsm context as the type `<tt>`.
#define CREATE_CONTEXT_GETTER(name, tt) \
  tt *name(StateMachine *fsm)           \
  {                                     \
    return (tt *)(fsm->context);        \
  }

#define CREATE_MUTATOR(name, tt, code) \
  tt *name(StateMachine *fsm)          \
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
#define CREATE_CONDITIONAL_MUTATOR(name, tt, code)      \
  void name(StateMachine *fsm, bool (*guard)(tt * ctx)) \
  {                                                     \
    tt *ctx = (tt *)(fsm->context);                     \
    if (!guard(ctx)) return;                            \
    ctx->code                                           \
  }

StateMachine *fsm_create(const char *name, void *context);

void fsm_free(StateMachine *fsm);

void fsm_set_initial_state(StateMachine *fsm, StateDescriptor *s);

const char *fsm_get_state_name(StateMachine *fsm);

StateDescriptor *fsm_state_create(const char *name);

StateDescriptor *fsm_state_register(StateMachine *fsm, StateDescriptor *s);

void fsm_state_free(StateDescriptor *s);

Transition *fsm_transition_create(
  const char      *name,
  StateDescriptor *target,
  bool (*guard)(void *),
  void (*action)(void *)
);

Transition *fsm_transition_register(
  StateMachine    *fsm,
  StateDescriptor *source,
  Transition      *t
);

void fsm_transition_free(Transition *t);

void fsm_transition(StateMachine *fsm, const char *const event);

StateMachine *fsm_clone(const char *name, void *context, StateMachine *source);

StateMachine *__fsm_inline(
  const char       *name,
  const char       *initial_state,
  char             *states[],
  int               num_states,
  InlineTransition *t,
  ...
);

// TODO: improve
#define fsm_inline_states(...) \
  ((const char *[])__VA_ARGS__), sizeof((char *[])__VA_ARGS__) / sizeof(char *)

#define fsm_inline(name, initial_state, states, num_states, ...) \
  __fsm_inline(name, initial_state, states, num_states, __VA_ARGS__, NULL)

void __fsm_inline_free(StateMachine *fsm);
#define fsm_inline_free(fsm) __fsm_inline_free(fsm)

#ifdef __cplusplus
}
#endif

#endif /* LIBFSMS_H */

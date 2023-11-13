#ifndef LIBFSMS_H
#define LIBFSMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "libutil/libutil.h"

#define GET_CONTEXT(tt, fsm) ((tt *)fsm->context)

typedef struct state_t {
  const char *name;
  array_t    *transitions;
} StateDescriptor;

typedef struct transition_t {
  const char      *name;
  StateDescriptor *target;
  void (*action)(void *context);
  bool (*cond)(void *context);
} Transition;

typedef struct state_machine_t {
  const char      *name;
  void            *context;
  array_t         *states;
  StateDescriptor *state;
} StateMachine;

// Creates a function `get_context` that returns the fsm context as the type.
// The function name is `get_context` plus underscore and your context type name
// e.g. tt = MyContext* mctx = get_context_MyContext(fsm);

// TODO: just provide the name like CREATE_MUTATOR
#define CREATE_CONTEXT_GETTER(tt)         \
  tt *get_context_##tt(StateMachine *fsm) \
  {                                       \
    return (tt *)(fsm->context);          \
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
#define CREATE_CONDITIONAL_MUTATOR(name, tt, code)     \
  void name(StateMachine *fsm, bool (*cond)(tt * ctx)) \
  {                                                    \
    tt *ctx = (tt *)(fsm->context);                    \
    if (!cond(ctx)) return;                            \
    ctx->code                                          \
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
  bool (*cond)(void *),
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

#ifdef __cplusplus
}
#endif

#endif /* LIBFSMS_H */

#ifndef LIBFSMS_H
#define LIBFSMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "libutil/libutil.h"

#define GET_CONTEXT(tt, fsm) ((tt*)fsm->context)

typedef struct state_t {
  char* name;
  array_t* transitions;
} StateDescriptor;

typedef struct transition_t {
  char* name;
  StateDescriptor* target;
  void (*action)(void*);
  bool (*cond)(void*);
} Transition;

typedef struct state_machine_t {
  char* name;
  char* state;
  void* context;
  array_t* states;
} StateMachine;

// Creates a function `get_context` that returns the fsm context as the type.
// The function name is `get_context` plus underscore and your context type name
// e.g. tt = MyContext* mctx = get_context_MyContext(fsm);

// TODO: just provide the name like CREATE_MUTATOR
#define CREATE_CONTEXT_GETTER(tt) \
  tt* get_context_##tt(StateMachine* fsm) { return (tt*)(fsm->context); }

#define CREATE_MUTATOR(name, tt, code) \
  tt* name(StateMachine* fsm) {        \
    tt* ctx = (tt*)(fsm->context);     \
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
  tt* name(StateMachine* fsm, bool (*cond)(tt * ctx)) { \
    tt* ctx = (tt*)(fsm->context);                      \
    if (!cond(ctx)) return;                             \
    ctx->code                                           \
  }

StateMachine* fsm_create(char* name, void* context);

void fsm_transition_create(StateMachine* fsm, char* name,
                           StateDescriptor* source, StateDescriptor* target,
                           bool (*cond)(void*), void (*action)(void*));

StateDescriptor* fsm_state_create(StateMachine* fsm, char* name);

void fsm_transition(StateMachine* fsm, char* event);

#ifdef __cplusplus
}
#endif

#endif /* LIBFSMS_H */

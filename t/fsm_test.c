#include <stdlib.h>

#include "fsms.h";
#include "tap.c/tap.h"

typedef struct context_t {
  char* name;
  int count;
} Context;

CREATE_CONTEXT_GETTER(Context)
CREATE_MUTATOR(increment_count, Context, count++;)

const char* TRANSITION_NAME = "switch";

const char* ON_STATE = "on";
const char* OFF_STATE = "off";

static int on_counter = 0;
static int off_counter = 0;

void on_action_handler() { on_counter++; }
void off_action_handler() { off_counter++; }
void noop_handler() { return; }
bool cond_true() { return true; }
bool cond_false() { return false; }
bool cond_context(void* context) {
  Context* ctx = (Context*)context;
  return ctx->count == 1;
}

void fsm_create_test() {
  char* name = "test";
  char* initial_state = "off";

  StateMachine* fsm = fsm_create(name, NULL);

  is(fsm->name, name, "has the assigned name");
  cmp_ok(fsm->num_states, "==", 0, "initializes number of states to zero");
  is(fsm->state, NULL, "initializes state to NULL");
  is(fsm->context, NULL, "initializes context to provided value");

  fsm->state = initial_state;
  is(fsm->state, initial_state, "has the assigned initial state");
}

void fsm_state_create_test() {
  char* fsm_name = "test";

  StateMachine* fsm = fsm_create(fsm_name, NULL);
  StateDescriptor* s = fsm_state_create(fsm, OFF_STATE);

  is(s->name, OFF_STATE, "has the assigned state name");
  cmp_ok(s->num_transitions, "==", 0,
         "initializes number of transitions to zero");

  // also registers
  cmp_ok(fsm->num_states, "==", 1, "increments the states count");
  isnt(fsm->states, NULL, "states array is non-NULL");
  is(fsm->states[fsm->num_states - 1]->name, OFF_STATE,
     "maintains the registered state's attributes");
}

void fsm_transition_create_test() {
  char* name = "switch";
  Transition* t =
      fsm_transition_create("switch", ON_STATE, cond_true, noop_handler);

  is(t->name, name, "has the assigned transition name");
  is(t->target, ON_STATE, "has the assigned transition target");
  cmp_ok(t->action, "==", noop_handler, "assigns the given action callback");
  cmp_ok(t->cond, "==", cond_true, "assigns the given cond callback");
}

void fsm_register_transition_test() {
  char* fsm_name = "test";

  StateMachine* fsm = fsm_create(fsm_name, NULL);
  StateDescriptor* s = fsm_state_create(fsm, OFF_STATE);
  Transition* t =
      fsm_transition_create(TRANSITION_NAME, ON_STATE, cond_true, noop_handler);

  fsm_register_transition(fsm, s->name, t);

  StateDescriptor* registered_state = fsm->states[fsm->num_states - 1];
  Transition* registered_transition =
      registered_state->transitions[registered_state->num_transitions - 1];

  cmp_ok(registered_state->num_transitions, "==", 1, "");
  is(registered_transition->name, TRANSITION_NAME,
     "maintains the registered transition's name");
  cmp_ok(registered_transition->action, "==", noop_handler,
         "maintains the registered transition's action");
  cmp_ok(registered_transition->cond, "==", cond_true,
         "maintains the registered transition's cond");
}

void fsm_transition_test() {
  StateMachine* fsm = fsm_create("test", NULL);
  fsm->state = OFF_STATE;

  Transition* t1 = fsm_transition_create(TRANSITION_NAME, ON_STATE, cond_true,
                                         on_action_handler);
  Transition* t2 = fsm_transition_create(TRANSITION_NAME, OFF_STATE, cond_true,
                                         off_action_handler);

  StateDescriptor* off_s = fsm_state_create(fsm, OFF_STATE);
  StateDescriptor* on_s = fsm_state_create(fsm, ON_STATE);

  fsm_register_transition(fsm, off_s->name, t1);
  fsm_register_transition(fsm, on_s->name, t2);

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, OFF_STATE, "initial state is off");    // sanity check
  cmp_ok(fsm->num_states, "==", 2, "has two states");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, ON_STATE, "new state is on");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 1, "off counter is 1");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, ON_STATE, "new state is on");
  cmp_ok(on_counter, "==", 2, "on counter is 2");
  cmp_ok(off_counter, "==", 1, "off counter is 1");
}

void conditional_transition_test() {
  off_counter = 0;
  on_counter = 0;

  StateMachine* fsm = fsm_create("test", NULL);
  fsm->state = OFF_STATE;

  Transition* t1 = fsm_transition_create(TRANSITION_NAME, ON_STATE, cond_false,
                                         on_action_handler);
  Transition* t2 = fsm_transition_create(TRANSITION_NAME, OFF_STATE, cond_false,
                                         off_action_handler);

  StateDescriptor* off_s = fsm_state_create(fsm, OFF_STATE);
  StateDescriptor* on_s = fsm_state_create(fsm, ON_STATE);

  fsm_register_transition(fsm, off_s->name, t1);
  fsm_register_transition(fsm, on_s->name, t2);

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, OFF_STATE, "initial state is off");    // sanity check

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");
}

void with_context_test() {
  off_counter = 0;
  on_counter = 0;

  Context* ctx = malloc(sizeof(*ctx));
  ctx->count = 0;
  ctx->name = "test_ctx";
  StateMachine* fsm = fsm_create("test", (void*)ctx);
  fsm->state = OFF_STATE;

  Transition* t1 = fsm_transition_create(TRANSITION_NAME, ON_STATE,
                                         cond_context, on_action_handler);
  Transition* t2 = fsm_transition_create(TRANSITION_NAME, OFF_STATE,
                                         cond_context, off_action_handler);

  StateDescriptor* off_s = fsm_state_create(fsm, OFF_STATE);
  StateDescriptor* on_s = fsm_state_create(fsm, ON_STATE);

  fsm_register_transition(fsm, off_s->name, t1);
  fsm_register_transition(fsm, on_s->name, t2);

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, OFF_STATE, "initial state is off");    // sanity check

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  increment_count(fsm);

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, ON_STATE, "new state is on");
  cmp_ok(on_counter, "==", 1, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");
}

int main() {
  plan(52);

  fsm_create_test();
  fsm_state_create_test();
  fsm_transition_create_test();
  fsm_register_transition_test();
  fsm_transition_test();
  conditional_transition_test();
  with_context_test();

  done_testing();
}

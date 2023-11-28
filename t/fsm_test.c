#include <stdlib.h>

#include "tests.h"

typedef struct context_t {
  char* name;
  int   count;
} Context;

CREATE_MUTATOR(increment_count, Context, count++;)

static const char* TRANSITION_NAME = "switch";

static const char* ON_STATE        = "on";
static const char* OFF_STATE       = "off";

static int on_counter              = 0;
static int off_counter             = 0;

static void
on_action_handler ()
{
  on_counter++;
}

static void
off_action_handler ()
{
  off_counter++;
}

static void
noop_handler ()
{
  return;
}

static bool
cond_true ()
{
  return true;
}

static bool
cond_false ()
{
  return false;
}

static bool
cond_context (void* context)
{
  Context* ctx = (Context*)context;
  return ctx->count == 1;
}

void
fsm_create_test ()
{
  char* name        = "test";

  StateMachine* fsm = fsm_create(name, NULL);

  is(fsm->name, name, "has the assigned name");
  cmp_ok(
    array_size(fsm->states),
    "==",
    0,
    "initializes number of states to zero"
  );
  is(fsm->state, NULL, "initializes state to NULL");
  is(fsm->context, NULL, "initializes context to provided value");

  char* initial_state = fsm_state_create(OFF_STATE);
  fsm_state_register(fsm, initial_state);
  fsm_set_initial_state(fsm, initial_state);

  is(fsm->state, initial_state, "has the assigned initial state");

  fsm_state_free(initial_state);
  fsm_free(fsm);
}

void
fsm_state_create_test ()
{
  char* fsm_name       = "test";

  StateMachine*    fsm = fsm_create(fsm_name, NULL);
  StateDescriptor* s   = fsm_state_create(OFF_STATE);
  fsm_state_register(fsm, s);

  is(s->name, OFF_STATE, "has the assigned state name");
  cmp_ok(
    array_size(s->transitions),
    "==",
    0,
    "initializes number of transitions to zero"
  );

  // also registers
  cmp_ok(array_size(fsm->states), "==", 1, "increments the states count");
  isnt(fsm->states, NULL, "states array is non-NULL");

  is(
    ((StateDescriptor*)array_get(fsm->states, -1))->name,
    OFF_STATE,
    "maintains the registered state's attributes"
  );

  fsm_state_free(s);
  fsm_free(fsm);
}

void
fsm_transition_create_test ()
{
  char* name         = "switch";

  StateDescriptor* s = fsm_state_create(ON_STATE);

  Transition* t = fsm_transition_create("switch", s, cond_true, noop_handler);

  is(t->name, name, "has the assigned transition name");
  is(t->target, s, "has the assigned transition target");
  cmp_ok(t->action, "==", noop_handler, "assigns the given action callback");
  cmp_ok(t->guard, "==", cond_true, "assigns the given guard callback");

  fsm_state_free(s);
  fsm_transition_free(t);
}

void
fsm_transition_register_test ()
{
  char* fsm_name         = "test";

  StateMachine* fsm      = fsm_create(fsm_name, NULL);

  StateDescriptor* off_s = fsm_state_create(OFF_STATE);
  fsm_state_register(fsm, off_s);
  StateDescriptor* on_s = fsm_state_create(ON_STATE);
  fsm_state_register(fsm, on_s);

  Transition* t1
    = fsm_transition_create(TRANSITION_NAME, on_s, cond_true, noop_handler);
  fsm_transition_register(fsm, off_s, t1);

  cmp_ok(array_size(off_s->transitions), "==", 1, "off state has 1 transition");

  Transition* t2
    = fsm_transition_create(TRANSITION_NAME, off_s, cond_true, noop_handler);
  fsm_transition_register(fsm, on_s, t2);

  cmp_ok(array_size(on_s->transitions), "==", 1, "on state has 1 transition");

  Transition* off_transition = array_get(on_s->transitions, 0);

  is(
    off_transition->name,
    TRANSITION_NAME,
    "maintains the registered transition's name"
  );
  cmp_ok(
    off_transition->action,
    "==",
    noop_handler,
    "maintains the registered transition's action"
  );
  cmp_ok(
    off_transition->guard,
    "==",
    cond_true,
    "maintains the registered transition's guard"
  );

  Transition* on_transition = array_get(off_s->transitions, 0);

  is(
    on_transition->name,
    TRANSITION_NAME,
    "maintains the registered transition's name"
  );
  cmp_ok(
    on_transition->action,
    "==",
    noop_handler,
    "maintains the registered transition's action"
  );
  cmp_ok(
    on_transition->guard,
    "==",
    cond_true,
    "maintains the registered transition's guard"
  );

  fsm_state_free(on_s);
  fsm_state_free(off_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(fsm);
}

void
fsm_transition_test ()
{
  StateMachine* fsm      = fsm_create("test", NULL);

  StateDescriptor* off_s = fsm_state_register(fsm, fsm_state_create(OFF_STATE));
  StateDescriptor* on_s  = fsm_state_register(fsm, fsm_state_create(ON_STATE));

  fsm_set_initial_state(fsm, off_s);

  Transition* t1 = fsm_transition_register(
    fsm,
    off_s,
    fsm_transition_create(TRANSITION_NAME, on_s, cond_true, on_action_handler)
  );

  Transition* t2 = fsm_transition_register(
    fsm,
    on_s,
    fsm_transition_create(TRANSITION_NAME, off_s, cond_true, off_action_handler)
  );

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, off_s, "initial state is off");        // sanity check
  cmp_ok(array_size(fsm->states), "==", 2, "has two states");

  fsm_transition(fsm, TRANSITION_NAME);

  is(fsm->state, on_s, "new state is on");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, off_s, "new state is off");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 1, "off counter is 1");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, on_s, "new state is on");
  cmp_ok(on_counter, "==", 2, "on counter is 2");
  cmp_ok(off_counter, "==", 1, "off counter is 1");

  fsm_state_free(on_s);
  fsm_state_free(off_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(fsm);
}

void
conditional_transition_test ()
{
  off_counter            = 0;
  on_counter             = 0;

  StateMachine* fsm      = fsm_create("test", NULL);

  StateDescriptor* off_s = fsm_state_register(fsm, fsm_state_create(OFF_STATE));
  StateDescriptor* on_s  = fsm_state_register(fsm, fsm_state_create(ON_STATE));

  fsm_set_initial_state(fsm, off_s);

  Transition* t1 = fsm_transition_register(
    fsm,
    off_s,
    fsm_transition_create(TRANSITION_NAME, on_s, cond_false, on_action_handler)
  );

  Transition* t2 = fsm_transition_register(
    fsm,
    on_s,
    fsm_transition_create(
      TRANSITION_NAME,
      off_s,
      cond_false,
      off_action_handler
    )
  );

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, off_s, "initial state is off");        // sanity check

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, off_s, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, off_s, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_state_free(on_s);
  fsm_state_free(off_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(fsm);
}

void
with_context_test ()
{
  off_counter            = 0;
  on_counter             = 0;

  Context* ctx           = malloc(sizeof(*ctx));
  ctx->count             = 0;
  ctx->name              = "test_ctx";
  StateMachine* fsm      = fsm_create("test", (void*)ctx);

  StateDescriptor* off_s = fsm_state_register(fsm, fsm_state_create(OFF_STATE));
  StateDescriptor* on_s  = fsm_state_register(fsm, fsm_state_create(ON_STATE));

  fsm_set_initial_state(fsm, off_s);

  Transition* t1 = fsm_transition_register(
    fsm,
    off_s,
    fsm_transition_create(
      TRANSITION_NAME,
      on_s,
      cond_context,
      on_action_handler
    )
  );

  Transition* t2 = fsm_transition_register(
    fsm,
    on_s,
    fsm_transition_create(
      TRANSITION_NAME,
      off_s,
      cond_context,
      off_action_handler
    )
  );

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(fsm->state, off_s, "initial state is off");        // sanity check

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, off_s, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, off_s, "new state is off");
  cmp_ok(on_counter, "==", 0, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  increment_count(fsm);

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm->state, on_s, "new state is on");
  cmp_ok(on_counter, "==", 1, "on counter is zero");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_state_free(on_s);
  fsm_state_free(off_s);
  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_free(fsm);
}

void
run_fsm_tests (void)
{
  fsm_create_test();
  fsm_state_create_test();
  fsm_transition_create_test();
  fsm_transition_register_test();
  fsm_transition_test();
  conditional_transition_test();
  with_context_test();
}

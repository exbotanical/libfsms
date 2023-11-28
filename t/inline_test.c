
#include "tests.h"

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

void
print (char* a[])
{
  printf(">>> %d\n", sizeof(a));
}

void
fsm_inline_test ()
{
  StateMachine* fsm = fsm_inline(
    "test",
    OFF_STATE,
    fsm_inline_states({ON_STATE, OFF_STATE}),
    &(InlineTransition){
      .name   = TRANSITION_NAME,
      .action = on_action_handler,
      .guard  = cond_true,
      .source = OFF_STATE,
      .target = ON_STATE},
    &(InlineTransition){
      .name   = TRANSITION_NAME,
      .action = off_action_handler,
      .guard  = cond_true,
      .source = ON_STATE,
      .target = OFF_STATE}
  );

  cmp_ok(on_counter, "==", 0, "on counter is zero");    // baseline
  cmp_ok(off_counter, "==", 0, "off counter is zero");  // baseline
  is(
    fsm_get_state_name(fsm),
    OFF_STATE,
    "initial state is off"
  );  // sanity check
  cmp_ok(array_size(fsm->states), "==", 2, "has two states");

  fsm_transition(fsm, TRANSITION_NAME);

  is(fsm_get_state_name(fsm), ON_STATE, "new state is on");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 0, "off counter is zero");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm_get_state_name(fsm), OFF_STATE, "new state is off");
  cmp_ok(on_counter, "==", 1, "on counter is 1");
  cmp_ok(off_counter, "==", 1, "off counter is 1");

  fsm_transition(fsm, TRANSITION_NAME);
  is(fsm_get_state_name(fsm), ON_STATE, "new state is on");
  cmp_ok(on_counter, "==", 2, "on counter is 2");
  cmp_ok(off_counter, "==", 1, "off counter is 1");

  fsm_inline_free(fsm);
}

void
run_inline_tests (void)
{
  fsm_inline_test();
}

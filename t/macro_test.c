#include <stdlib.h>

#include "tests.h"

typedef struct context_t {
  int count;
  int port;
} Context;

CREATE_CONTEXT_GETTER(Context)

void
create_context_getter_test ()
{
  int      count    = 11;
  int      port     = 3000;
  Context* ctx      = malloc(sizeof(*ctx));
  ctx->count        = count;
  ctx->port         = port;
  StateMachine* fsm = fsm_create("test", (void*)ctx);

  lives_ok({ get_context_Context(fsm)->count; }, "retrieves the context");
  lives_ok({ get_context_Context(fsm)->port; }, "retrieves the context");
  cmp_ok(
    get_context_Context(fsm)->count,
    "==",
    count,
    "retrieves the correct context value"
  );
  cmp_ok(
    get_context_Context(fsm)->port,
    "==",
    port,
    "retrieves the correct context value"
  );

  fsm_free(fsm);
}

CREATE_MUTATOR(increment_count2, Context, count++;)
CREATE_MUTATOR(decrement_count, Context, count--;)
CREATE_MUTATOR(change_port, Context, port *= 2;)

void
create_mutator_test ()
{
  int      count    = 11;
  int      port     = 3000;
  Context* ctx      = malloc(sizeof(*ctx));
  ctx->count        = count;
  ctx->port         = port;
  StateMachine* fsm = fsm_create("test", (void*)ctx);

  cmp_ok(
    ctx->count,
    "==",
    count,
    "count is initialized value"
  );  // sanity check

  increment_count2(fsm);
  cmp_ok(ctx->count, "==", count + 1, "increments the count");

  decrement_count(fsm);
  decrement_count(fsm);
  cmp_ok(ctx->count, "==", count - 1, "decrements the count");

  cmp_ok(ctx->port, "==", port,
         "port is initialized value");  // sanity check

  change_port(fsm);
  cmp_ok(ctx->port, "==", port * 2, "changes the port");

  fsm_free(fsm);
}

bool
is_even (int n)
{
  return n % 2 == 0;
}

CREATE_CONDITIONAL_MUTATOR(incr_if, Context, count++;)

void
create_conditional_mutator_test ()
{
  int      count    = 11;
  Context* ctx      = malloc(sizeof(*ctx));
  ctx->count        = count;
  StateMachine* fsm = fsm_create("test", (void*)ctx);

  cmp_ok(
    ctx->count,
    "==",
    count,
    "count is initialized value"
  );  // sanity check

  // TODO: use macro e.g. count == 2;
  incr_if(fsm, is_even);
  cmp_ok(ctx->count, "==", count + 1, "increments the count");

  fsm_free(fsm);
}

void
run_macro_tests (void)
{
  create_context_getter_test();
  create_mutator_test();
}

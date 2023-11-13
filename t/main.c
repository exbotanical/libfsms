#include "tap.c/tap.h"
#include "tests.h"

int
main ()
{
  plan(61);

  run_fsm_tests();
  run_macro_tests();

  done_testing();
}

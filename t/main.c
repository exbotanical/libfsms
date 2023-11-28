#include "tap.c/tap.h"
#include "tests.h"

int
main ()
{
  plan(78);

  run_fsm_tests();
  run_macro_tests();
  run_inline_tests();

  done_testing();
}

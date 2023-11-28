#include <stdio.h>
#include <stdlib.h>

#include "libfsms.h"

typedef struct {
  unsigned int id;
  int          pid;
  int          status;
} job_t;

// PENDING : {
//   transitions : {
//     promote : {
//       target: 'RUNNING'
//       action : run_job
//     }
//   }
// }
// RUNNING : {
//   transitions : {
//     promote : {
//       target: 'EXITED'
//       action : await_job
//       guard: ???
//     }
//   }
// }
// EXITED : {
//   transitions : {
//     promote : {
//       target: 'RESOLVED'
//       action : report_job
//     }
//   }
// }
const char *TRANSITION_NAME  = "promote";

const char *PENDING_STATE    = "PENDING";
const char *RUNNING_STATE    = "RUNNING";
const char *EXITED_STATE     = "EXITED";
const char *RESOLVED_STATE   = "RESOLVED";

static unsigned int id       = 0;
static unsigned int fake_pid = 0;

static job_t *
new_job ()
{
  job_t *job  = malloc(sizeof(job_t));
  job->id     = ++id;
  job->pid    = -1;
  job->status = -1;

  return job;
}

void
on_running (void *arg)
{
  job_t *job = (job_t *)arg;
  printf("running!\n");

  job->pid = ++fake_pid;
}

bool
should_await (void *arg)
{
  printf("should i ?\n");

  job_t *job = (job_t *)arg;
  return job->pid != -1;
}

void
on_await (void *arg)
{
  job_t *job = (job_t *)arg;
  printf("await!\n");
  job->status = 0;
}

bool
should_resolve (void *arg)
{
  job_t *job = (job_t *)arg;
  return job->status == 0;
}

void
on_resolve (void *arg)
{
  job_t *job = (job_t *)arg;
  printf("resolved!\n");
}

int
main ()
{
  job_t *job                 = new_job();

  StateMachine *fsm          = fsm_create("scheduler", job);

  StateDescriptor *pending_s = fsm_state_create(PENDING_STATE);
  fsm_state_register(fsm, pending_s);

  StateDescriptor *running_s = fsm_state_create(RUNNING_STATE);
  fsm_state_register(fsm, running_s);

  StateDescriptor *exited_s = fsm_state_create(EXITED_STATE);
  fsm_state_register(fsm, exited_s);

  StateDescriptor *resolved_s = fsm_state_create(RESOLVED_STATE);
  fsm_state_register(fsm, resolved_s);

  fsm_set_initial_state(fsm, pending_s);

  Transition *t1
    = fsm_transition_create(TRANSITION_NAME, running_s, NULL, on_running);
  fsm_transition_register(fsm, pending_s, t1);

  Transition *t2
    = fsm_transition_create(TRANSITION_NAME, exited_s, should_await, on_await);
  fsm_transition_register(fsm, running_s, t2);

  Transition *t3 = fsm_transition_create(
    TRANSITION_NAME,
    resolved_s,
    should_resolve,
    on_resolve
  );
  fsm_transition_register(fsm, exited_s, t3);

  printf("starting state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("current state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  printf("current state: %s\n", fsm_get_state_name(fsm));
  fsm_transition(fsm, TRANSITION_NAME);
  fsm_transition(fsm, TRANSITION_NAME);  // no-op

  printf("ending state: %s\n", fsm_get_state_name(fsm));

  fsm_state_free(pending_s);
  fsm_state_free(running_s);
  fsm_state_free(exited_s);
  fsm_state_free(resolved_s);

  fsm_transition_free(t1);
  fsm_transition_free(t2);
  fsm_transition_free(t3);

  fsm_free(fsm);

  printf("done\n");
}

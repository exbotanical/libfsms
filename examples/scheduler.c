#include <stdio.h>
#include <stdlib.h>

#include "libfsms.h"

typedef struct {
  unsigned int id;
  int pid;
  int status;
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
//       cond: ???
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
const char* TRANSITION_NAME = "promote";

const char* PENDING_STATE = "PENDING";
const char* RUNNING_STATE = "RUNNING";
const char* EXITED_STATE = "EXITED";
const char* RESOLVED_STATE = "RESOLVED";

static unsigned int id = 0;
static unsigned int fake_pid = 0;

static job_t* new_job() {
  job_t* job = malloc(sizeof(job_t));
  job->id = ++id;
  job->pid = -1;
  job->status = -1;

  return job;
}

void on_running(void* arg) {
  job_t* job = (job_t*)arg;
  printf("running!\n");

  job->pid = ++fake_pid;
}

bool should_await(void* arg) {
  printf("should i ?\n");

  job_t* job = (job_t*)arg;
  return job->pid != -1;
}

void on_await(void* arg) {
  job_t* job = (job_t*)arg;
  printf("await!\n");
  job->status = 0;
}

bool should_resolve(void* arg) {
  job_t* job = (job_t*)arg;
  return job->status == 0;
}

void on_resolve(void* arg) {
  job_t* job = (job_t*)arg;
  printf("resolved!\n");
}

int main() {
  job_t* job = new_job();

  StateMachine* fsm = fsm_create("scheduler", job);
  fsm->state = PENDING_STATE;

  Transition* run_job =
      fsm_transition_create(TRANSITION_NAME, RUNNING_STATE, NULL, on_running);

  Transition* await_job = fsm_transition_create(TRANSITION_NAME, EXITED_STATE,
                                                should_await, on_await);

  Transition* report_job = fsm_transition_create(
      TRANSITION_NAME, RESOLVED_STATE, should_resolve, on_resolve);

  StateDescriptor* pending_s = fsm_state_create(fsm, PENDING_STATE);
  StateDescriptor* running_s = fsm_state_create(fsm, RUNNING_STATE);
  StateDescriptor* exited_s = fsm_state_create(fsm, EXITED_STATE);

  fsm_register_transition(fsm, pending_s->name, run_job);
  fsm_register_transition(fsm, running_s->name, await_job);
  fsm_register_transition(fsm, exited_s->name, report_job);

  printf("starting state: %s\n", fsm->state);
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm->state);
  fsm_transition(fsm, TRANSITION_NAME);
  printf("ending state: %s\n", fsm->state);
}

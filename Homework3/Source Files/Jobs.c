#include <sys/wait.h>
#include <stdio.h>
#include "Jobs.h"
#include "deq.h"
#include "error.h"

extern Jobs newJobs() {
  return deq_new();
}

extern void addJobs(Jobs jobs, Pipeline pipeline) {
  deq_tail_put(jobs, pipeline);
}

extern int sizeJobs(Jobs jobs) {
  return deq_len(jobs);
}

/**
 * Non-blockingly reaps any terminated background child processes using waitpid(), 
 * locating and removing their corresponding completed pipelines from the active jobs queue.
 */
extern void reapJobs(Jobs jobs) {
  pid_t pid;
  int status;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    int len = deq_len(jobs);
    for (int i = 0; i < len; i++) {
      Pipeline p = deq_head_ith(jobs, i);
      if (pipelineContainsPid(p, pid)) {
        deq_head_rem(jobs, p);
        freePipeline(p);
        break;
      }
    }
  }
}

/**
 * Waits for all active background pipelines to complete execution and 
 * frees all associated job structures and memory resources upon shell exit.
 */
extern void waitAllJobs(Jobs jobs) {
  int len = deq_len(jobs);
  for (int i = 0; i < len; i++) {
    Pipeline p = deq_head_ith(jobs, i);
    waitPipeline(p);
  }
  while (deq_len(jobs) > 0) {
    Pipeline p = deq_head_get(jobs);
    freePipeline(p);
  }
}

extern void freeJobs(Jobs jobs) {
  deq_del(jobs, freePipeline);
}

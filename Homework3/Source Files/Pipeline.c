#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Pipeline.h"
#include "deq.h"
#include "error.h"

typedef struct {
  Deq commands;
  int fg;
  pid_t *pids;
  int npids;
} *PipelineRep;

extern Pipeline newPipeline(int fg) {
  PipelineRep r=(PipelineRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->commands=deq_new();
  r->fg=fg;
  r->pids=0;
  r->npids=0;
  return r;
}

extern void addPipeline(Pipeline pipeline, Command command) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_tail_put(r->commands,command);
}

extern int sizePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  return deq_len(r->commands);
}

/**
 * Checks whether a specified process ID matches any process 
 * currently tracked within a given pipeline's process array.
 */
extern int pipelineContainsPid(Pipeline pipeline, pid_t pid) {
  PipelineRep r=(PipelineRep)pipeline;
  if (r->pids) {
    for (int i = 0; i < r->npids; i++) {
      if (r->pids[i] == pid) return 1;
    }
  }
  return 0;
}

/**
 * Blocks until all child processes associated with a pipeline have terminated,
 * then safely deallocates the pipeline's tracked process ID array.
 */
extern void waitPipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  if (r->pids) {
    for (int i = 0; i < r->npids; i++) {
      int status;
      waitpid(r->pids[i], &status, 0);
    }
    free(r->pids);
    r->pids = 0;
    r->npids = 0;
  }
}

/**
 * Sets up command execution across a pipeline by setting up pipe descriptors, 
 * managing input/output redirection, forking child processes, and handling 
 * either synchronous foreground waiting or asynchronous background job tracking.
 */
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof) {
  PipelineRep r=(PipelineRep)pipeline;
  int n = sizePipeline(pipeline);
  if (n == 0) return;

  if (n == 1 && r->fg) {
    Command cmd = deq_head_ith(r->commands, 0);
    if (execBuiltinIfPossible(cmd, eof, jobs)) {
      freePipeline(pipeline);
      return;
    }
  }

  r->pids = (pid_t *)malloc(sizeof(pid_t) * n);
  if (!r->pids) ERROR("malloc() failed");
  r->npids = n;

  int in_fd = 0;
  int fds[2];

  for (int i = 0; i < n; i++) {
    Command cmd = deq_head_ith(r->commands, i);
    
    if (i < n - 1) {
      if (pipe(fds) == -1)
        ERROR("pipe() failed");
    } else {
      fds[0] = 0;
      fds[1] = 0;
    }

    pid_t pid = fork();
    if (pid == -1) {
      ERROR("fork() failed");
    } else if (pid == 0) {
      if (in_fd != 0) {
        dup2(in_fd, 0);
        close(in_fd);
      }
      if (i < n - 1) {
        close(fds[0]);
        dup2(fds[1], 1);
        close(fds[1]);
      }

      execCommandInChild(cmd, eof, jobs);
      exit(0);
    } else {
      r->pids[i] = pid;
      if (in_fd != 0)
        close(in_fd);
      if (i < n - 1) {
        close(fds[1]);
        in_fd = fds[0];
      }
    }
  }

  if (r->fg) {
    waitPipeline(pipeline);
    freePipeline(pipeline);
  } else {
    addJobs(jobs, pipeline);
  }
}

extern void freePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  if (r->pids)
    free(r->pids);
  deq_del(r->commands,freeCommand);
  free(r);
}

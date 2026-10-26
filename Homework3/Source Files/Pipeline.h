#ifndef PIPELINE_H
#define PIPELINE_H

#include <sys/types.h>

typedef void *Pipeline;

#include "Command.h"
#include "Jobs.h"

extern Pipeline newPipeline(int fg);
extern void addPipeline(Pipeline pipeline, Command command);
extern int sizePipeline(Pipeline pipeline);
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof);
extern void freePipeline(Pipeline pipeline);
extern int pipelineContainsPid(Pipeline pipeline, pid_t pid);
extern void waitPipeline(Pipeline pipeline);

#endif

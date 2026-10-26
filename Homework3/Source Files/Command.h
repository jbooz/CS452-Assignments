#ifndef COMMAND_H
#define COMMAND_H

typedef void *Command;

#include "Tree.h"
#include "Jobs.h"
#include "Sequence.h"

extern Command newCommand(T_command t);
extern int isBuiltin(char *name);
extern int execBuiltinIfPossible(Command command, int *eof, Jobs jobs);
extern void execCommandInChild(Command command, int *eof, Jobs jobs);

extern void freeCommand(Command command);
extern void freestateCommand();

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/history.h>

#include "Command.h"
#include "error.h"

typedef struct {
  char *file;
  char **argv;
  char *infile;
  char *outfile;
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd=0;
static char *cwd=0;

static void builtin_args(CommandRep r, int n) {
  char **argv=r->argv;
  int argc = 0;
  while (argv && argv[argc]) argc++;
  if (argc - 1 != n)
    ERROR("wrong number of arguments to builtin command");
}

BIDEFN(exit) {
  builtin_args(r,0);
  *eof=1;
}

BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd)
    cwd=getcwd(0,0);
  printf("%s\n",cwd ? cwd : "");
}

BIDEFN(cd) {
  char **argv = r->argv;
  int argc = 0;
  while (argv && argv[argc]) argc++;
  if (argc != 2)
    ERROR("wrong number of arguments to builtin command");

  if (strcmp(r->argv[1],"-")==0) {
    char *twd=cwd;
    cwd=owd;
    owd=twd;
  } else {
    if (owd) free(owd);
    owd=cwd;
    cwd=strdup(r->argv[1]);
  }
  if (cwd && chdir(cwd))
    ERROR("chdir() failed");
}

BIDEFN(history) {
  builtin_args(r,0);
  HIST_ENTRY **hist = history_list();
  if (hist) {
    for (int i = 0; hist[i]; i++) {
      printf("%d: %s\n", i + history_base, hist[i]->line);
    }
  }
}

static char **getargs(T_words words) {
  int n=0;
  T_words p=words;
  while (p) {
    p=p->words;
    n++;
  }
  char **argv=(char **)malloc(sizeof(char *)*(n+1));
  if (!argv)
    ERROR("malloc() failed");
  p=words;
  int i=0;
  while (p) {
    argv[i++]=strdup(p->word->s);
    p=p->words;
  }
  argv[i]=0;
  return argv;
}

/**
 * Allocates and initializes a new Command structure from parsed tokens,
 * sanitizing arguments by stripping trailing carriage returns, newlines, 
 * and whitespace to ensure cross-platform compatibility and robust parsing.
 */
extern Command newCommand(T_command t) {
  CommandRep r=(CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  
  r->argv=getargs(t->words);
  r->infile=t->infile ? strdup(t->infile) : 0;
  r->outfile=t->outfile ? strdup(t->outfile) : 0;

  for (int i = 0; r->argv[i]; i++) {
    size_t len = strlen(r->argv[i]);
    while (len > 0 && (r->argv[i][len - 1] == '\r' || 
                       r->argv[i][len - 1] == '\n' || 
                       r->argv[i][len - 1] == ' '  || 
                       r->argv[i][len - 1] == '\t')) {
      r->argv[i][len - 1] = '\0';
      len--;
    }
  }

  r->file = r->argv[0] ? r->argv[0] : "";

  return r;
}

extern int isBuiltin(char *name) {
  if (!name) return 0;
  return (!strcmp(name, "exit") ||
          !strcmp(name, "pwd") ||
          !strcmp(name, "cd") ||
          !strcmp(name, "history"));
}

static void executeBuiltin(CommandRep r, int *eof, Jobs jobs) {
  if (!strcmp(r->file, "exit")) bi_exit(r, eof, jobs);
  else if (!strcmp(r->file, "pwd")) bi_pwd(r, eof, jobs);
  else if (!strcmp(r->file, "cd")) bi_cd(r, eof, jobs);
  else if (!strcmp(r->file, "history")) bi_history(r, eof, jobs);
}

/**
 * Determines if a command is a shell builtin. Executes it directly in the 
 * parent process (handling optional I/O redirection when present) and 
 * returns 1 if handled; otherwise, returns 0.
 */
extern int execBuiltinIfPossible(Command command, int *eof, Jobs jobs) {
  CommandRep r = command;
  if (!r->argv[0]) return 0;
  if (isBuiltin(r->file)) {
    if (r->infile || r->outfile) {
      pid_t pid = fork();
      if (pid == -1) ERROR("fork() failed");
      if (pid == 0) {
        if (r->infile) {
          int fd = open(r->infile, O_RDONLY);
          if (fd == -1) ERROR("open() failed");
          dup2(fd, 0);
          close(fd);
        }
        if (r->outfile) {
          int fd = open(r->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (fd == -1) ERROR("open() failed");
          dup2(fd, 1);
          close(fd);
        }
        executeBuiltin(r, eof, jobs);
        exit(0);
      } else {
        int status;
        waitpid(pid, &status, 0);
        return 1;
      }
    } else {
      executeBuiltin(r, eof, jobs);
      return 1;
    }
  }
  return 0;
}

/**
 * Configures input/output file redirection and executes external commands 
 * via execvp() within a spawned child process.
 */
extern void execCommandInChild(Command command, int *eof, Jobs jobs) {
  CommandRep r = command;
  if (!r->argv[0]) return;

  if (r->infile) {
    int fd = open(r->infile, O_RDONLY);
    if (fd == -1) ERROR("open() failed");
    dup2(fd, 0);
    close(fd);
  }
  if (r->outfile) {
    int fd = open(r->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) ERROR("open() failed");
    dup2(fd, 1);
    close(fd);
  }

  if (isBuiltin(r->file)) {
    executeBuiltin(r, eof, jobs);
  } else {
    execvp(r->argv[0], r->argv);
    ERROR("execvp() failed");
  }
}

extern void freeCommand(Command command) {
  CommandRep r=command;
  char **argv=r->argv;
  while (*argv)
    free(*argv++);
  free(r->argv);
  if (r->infile) free(r->infile);
  if (r->outfile) free(r->outfile);
  free(r);
}

extern void freestateCommand() {
  if (cwd) free(cwd);
  if (owd) free(owd);
}

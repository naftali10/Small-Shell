#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_COMMAND_SZ 80
#define MAX_PATH_SZ 100
#define MAX_COMMAND_ARGS 20
#define HISTORY_SZ 50

struct Work_dir{
  char curr [MAX_PATH_SZ];
  char prev [MAX_PATH_SZ];
};

struct Command{
  char command[MAX_COMMAND_SZ]; //full prompt string
  char* command_args[MAX_COMMAND_ARGS]; //pointers to parts of the string
};

struct Job{
  int serial_num;
  char name[MAX_COMMAND_SZ];
  int pid;
  time_t start_time;
  bool fg;
  bool stopped;
  struct Job* next;
  struct Job* prev;
};
struct Job jobs;

#endif
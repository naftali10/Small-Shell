#ifndef SMASH_FUNCTIONS_H_INCLUDED
#define SMASH_FUNCTIONS_H_INCLUDED

#include "settings.h"
void load_pwd(char*);
void cd(char*, char*, char*);
void load_history(char*, struct Command*);
void print_jobs(void);
void send_kill(char**);
void fg(char**);
void bg(char**);
void cp(char**);
void diff(char**);
void quit(char**);
void run_prog(char*, char**);

#endif
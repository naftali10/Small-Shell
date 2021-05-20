#ifndef AUX_FUNCTIONS_H_INCLUDED
#define AUX_FUNCTIONS_H_INCLUDED

#include "settings.h"
void init_history(struct Command*);
void breakup_command(char*, char* [MAX_COMMAND_ARGS]);
void get_command(char*, char*);
void insert_to_history(char*, struct Command*, int*);
void add_to_jobs(char*, int, bool);
void clear_job(int);
void update_jobs(void);
void mark_job(int, char*);
void ctrl_z(int, siginfo_t *, void *);
void ctrl_c(int, siginfo_t *, void *);

#endif
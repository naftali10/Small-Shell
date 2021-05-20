#include "settings.h"

/*
Inputs: Commands history
Outputs:None
Usage:  Initiating all strings to be empty
*/
void init_history(struct Command* history){
  for(int i=0; i<HISTORY_SZ; i++){
    history[i].command[0]='\0';
    for(int j=0; j<MAX_COMMAND_ARGS; j++){
      history[i].command_args[j]=0;
    }
  }
}

/*
Inputs: Two strings
Outputs:None
Usage:  Receiving command from the user, and putting a 0 at its end,
        Then saving the same result twice.
*/
void get_command(char* str, char* str_cpy){
  do{
    errno=0;
    fgets(str, MAX_COMMAND_SZ, stdin);
  }
  while(EINTR == errno);
  if ((strlen(str) > 0) && (str[strlen(str) - 1] == '\n'))
      str[strlen(str) - 1] = '\0';
  strcpy(str_cpy, str);
}

/*
Inputs: Raw command from prompt
        Empty array of strings
Outputs:None
Usage:  Splitting raw command string into command and arguments.
        Input command will point to the command string, and
        input command_args' cells will point to the argument strings.
*/
void breakup_command(char* command, char* command_args[MAX_COMMAND_ARGS]) {
  int i = 0;
  int j = 0;
  for(j=0; j<MAX_COMMAND_ARGS; j++)
    command_args[j]=NULL;
  j = 0;
  while(command[i]!='\0'){
    if(command[i]==' ' && command[i+1]!=' ' && command[i+1]!='\0'){
      command_args[j] = &command[i+1];
      j++;
    }
    i++;
  }
  i = 0;
  while(command[i]!=0){
    if(command[i]==' ')
      command[i] = 0;
    i++;
  }
}

/*
Inputs: Command from user
        Command arguments from user
        Pointer to a history entry
Outputs:None
Usage:  Filling the history enrty with data from user.
        Used after a successful execution.
*/
void insert_to_history(char* user_input, struct Command* history_entry, int* hist_count){
  strcpy(history_entry->command, user_input);
  breakup_command(history_entry->command, history_entry->command_args);
  *hist_count = (*hist_count+1) % HISTORY_SZ;
}

/*
Inputs: Pointer to job's linked list
        Job name
        Job pid
Outputs:None
Usage:  Adding job to job list
*/
void add_to_jobs(char* job_name, int job_pid, bool fg){
  struct Job* job = &jobs;
  //If there are no valid jobs in the list
  if(job->serial_num==0){
    //put values into jobs
    job->serial_num = 1;
    strcpy(job->name,job_name);
    job->pid = job_pid;
    job->start_time = time(0);
    job->fg = fg;
    job->stopped = false;
    job->next = NULL;
    job->prev = NULL;
  }
  else{
    //point last job to a newly allocated one
    struct Job* new_job = malloc(sizeof(struct Job));
    while(job->next!=NULL)
      job = job->next;
    new_job->serial_num = job->serial_num+1;
    strcpy(new_job->name,job_name);
    new_job->pid = job_pid;
    new_job->start_time = time(0);
    new_job->fg = fg;
    new_job->stopped = false;
    new_job->next = NULL;
    new_job->prev = job;
    job->next = new_job;
  }
}

/*
Inputs: Mandatory inputs for a sigaction function
Outputs:None
Usage:  Removing job from job list
*/
void clear_job(int pid){
  struct Job* job = &jobs;
  //if the head should be removed
  if(jobs.pid==pid){
    //if it's the only job
    if(jobs.next==NULL)
      jobs.serial_num = 0;
    else{
      jobs = *jobs.next;
      free(job);
    }
    return;
  }
  while(job->next!=NULL){
    job = job->next;
    if(job->pid==pid){
      if(job->prev!=NULL)
        job->prev->next = job->next;
      if(job->next!=NULL)
        job->next->prev = job->prev;
      free(job);
      return;
    }
  }
}

/*
Inputs: Process ID
        "stop" ot "cont"
Outputs:None
Usage:  Marking a job as stopped, or unmarking it
*/
void mark_job(int pid, char* str){
  struct Job* job = &jobs;
  while(1){
    if(job->pid==pid){
      if(!strcmp(str,"stop"))
        job->stopped = true;
      else if(!strcmp(str,"cont"))
        job->stopped = false;
      else if(!strcmp(str,"fg"))
        job->fg = true;
      else if(!strcmp(str,"bg"))
        job->fg = false;
    }
    if(job->next!=NULL)
      job=job->next;
    else
      return;
  }
}

/*
Inputs: None
Outputs:None
Usage:  Editing job list according to latest changes, right befor prompt is shown
*/
void update_jobs(void){
  int child_pid, status;
  do{
    child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    //if a child process died
    if(child_pid>0){
      if(WIFEXITED(status) || WIFSIGNALED(status)){
        clear_job(child_pid);
      }
      //if a child process stopped
      else if (WIFSTOPPED(status)){
        mark_job(child_pid,"stop");
      }
      //if a child process continued
      else if(WIFCONTINUED(status)){
        mark_job(child_pid,"cont");
      }
    }
  }while(child_pid>0);
  return;
}

/*
Inputs: Arguments sent by sigaction function
Outputs:None
Usage:  Handling ctrl+z key stroke
*/
void ctrl_z(int signum, siginfo_t* info, void* context){
  if(kill((int)getpid(), SIGCONT))
    perror("Can't continue");
  struct Job* job = &jobs;
  //find the single fg job, if exists, and mark it as bg job
  while(1){
    printf("%d",job->fg);
    if(job->serial_num!=0 && job->fg==true){
      if(!kill(job->pid, SIGSTOP)){
        printf("Signal SIGSTOP was sent to pid %d\n",job->pid);
        mark_job(job->pid, "stop");
        mark_job(job->pid, "bg");
      }
      else
        perror("SIGTSTP error");
      return;
    }
    if(job->next!=NULL)
      job = job->next;
    else
      return;
  }
  return;
}

/*
Inputs: Arguments sent by sigaction function
Outputs:None
Usage:  Handling ctrl+c key stroke
*/
void ctrl_c(int signum, siginfo_t* info, void* context){
  if(kill((int)getpid(), SIGCONT))
    perror("Can't continue");
  struct Job* job = &jobs;
  //find the single fg job, if exists, and kill it
  int loop = 1;
  while(loop){
    if(job->serial_num!=0 && job->fg==true){
      if(!kill(job->pid, SIGINT)){
        printf("Signal SIGINT was sent to pid %d\n", job->pid);
        // clear_job?
        loop = 0;
      }
      else
        perror("SIGINT error");
      loop = 0;
    }
    if(job->next!=NULL)
      job = job->next;
    else
      loop = 0;
  }
  return;
}
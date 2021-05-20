#include "settings.h"
#include "aux_functions.h"

/*
Inputs: String
Outputs:None
Usage:  Writes the current working directory to the string.
*/
void load_pwd(char* str){
  getcwd(str, MAX_PATH_SZ);
}

/*
Inputs: New, current and previous paths of working directory
Outputs:None
Usage:  Changing current working directory according to the new one.
*/
void cd(char* path, char* curr_dir, char* prev_dir){
  if (!strcmp(path, "-"))
    path = prev_dir;
  if (chdir(path)==0){
    strcpy(prev_dir, curr_dir);
    load_pwd(curr_dir);
    printf("%s\n", curr_dir);
  }
  else
    printf("%s – No such file or directory\n", path);
}

/*
Inputs: Pointer to array of commands
Outputs:String - a list of existing commands
*/
void load_history(char* result, struct Command* history){
  result[0] = '\0';
  for (int i=0; i<HISTORY_SZ; i++){
    if(history[i].command[0]!='\0'){
      strcat(result, history[i].command);
      strcat(result, " ");
      for(int j=0; j<MAX_COMMAND_ARGS; j++){
        if(history[i].command_args[j]!=0){
          strcat(result, history[i].command_args[j]);
          strcat(result, " ");
        }
      }
      strcat(result, "\n");
    }
  }
}

/*
Inputs: Pointer to the head of the jobs' linked list
Outputs:None
Usage:  Printing all information of the job list
*/
void print_jobs(void){
  struct Job job = jobs;
  if (job.serial_num==0)
    return;
  while(1){
    printf("[%d]\t",job.serial_num);
    printf("%s:\t",job.name);
    printf("%d\t",job.pid);
    printf("%d secs\t",(int)difftime(time(0), job.start_time));
    if(job.stopped)
      printf("(Stopped)");
    printf("\n");
    if(job.next!=NULL)
      job = *job.next;
    else
      break;
  }
}

/*
Inputs: Pointer to the head of the jobs' linked list
        Arguments from the user: signal number, and job's s/n
Outputs:None
Usage:  Sending a signal to an active process (job)
*/
void send_kill(char** arguments){
  struct Job* job = &jobs;
  if(arguments[0]==NULL || arguments[1]==NULL)
    return;
  int signum = (int)strtol(&arguments[0][1], NULL, 10);
  int serial_num = (int)strtol(arguments[1], NULL, 10);
  while(job->next!=NULL && job->serial_num!=serial_num)
    job = job->next;
  if(job->serial_num!=serial_num)
    return;
  if(kill(job->pid, signum))
    perror("Kill failed");
  else
    printf("Signal #%d was sent to pid %d\n", signum, job->pid);
}

/*
Inputs: Optional argument from the user: job's s/n
Outputs:None
Usage:  Running the job in foreground
*/
void fg(char** arguments){
  struct Job* job = &jobs;
  // If job s/n wasn;t specified, go to last job
  if(arguments[0]==NULL)
    while(job->next!=NULL)
      job=job->next;
  else{
    int serial_num = (int)strtol(arguments[0], NULL, 10);
    while(job->next!=NULL && job->serial_num!=serial_num)
      job=job->next;
    if(job->serial_num!=serial_num)
      return;
  }
  if(job->serial_num!=0){
    if(job->stopped){
      if(kill(job->pid, SIGCONT))
        perror("Kill failed");
      else{
        printf("Signal SIGCONT was sent to pid %d\n", job->pid);
        mark_job(job->pid, "cont");
      }
    }
    mark_job(job->pid, "fg");
    printf("%s\n", job->name);
    waitpid(job->pid, NULL, 0);
  }
}

/*
Inputs: Job number
Outputs:None
Usage:  Continuing a stopped job in background
*/
void bg(char** arguments){
  struct Job* job = &jobs;
  //If no job number given, continue latest stopped job
  if(arguments[0]==NULL){
    //Go to last job
    while(job->next!=NULL)
      job=job->next;
    //Searche backwords for stopped job
    while(job->prev!=NULL && job->stopped!=true)
      job=job->prev;
    if(job->stopped!=true){
      printf("Error: there is no stopped job to continue\n");
      return;
    }
  }
  else{
    int serial_num = (int)strtol(arguments[0], NULL, 10);
    while(job->next!=NULL && job->serial_num!=serial_num)
      job=job->next;
    if(job->serial_num!=serial_num){
      printf("Error: job [%d] doesn't exist\n", serial_num);
      return;
    }
  }
  if(kill(job->pid, SIGCONT))
    perror("Continuing process failed");
  return;
}

/*
Inputs: Source and target file names
Outputs:None
Usage:  Copying source file to target file
*/
void cp(char** arguments){
  char ch;
  FILE *source, *target;
  source = fopen(arguments[0], "r");
  if(source==NULL){
    perror("Can't read source file");
    return;
  }
  target = fopen(arguments[1], "w");
  if(target==NULL){
    fclose(source);
    perror("Can't create target file");
    return;
  }
  while ((ch = fgetc(source)) != EOF)
    fputc(ch, target);
  printf("%s has been copied to %s\n", arguments[0], arguments[1]);
  fclose(source);
  fclose(target);
  return;
}

/*
Inputs: Two file paths
Outputs:None
Usage:  Checking if two files are identical
*/
void diff(char** arguments){
  char ch;
  FILE *file1, *file2;
  file1 = fopen(arguments[0],"r");
  if(file1==NULL){
    perror("Can't read first file");
    return;
  }
  file2 = fopen(arguments[1],"r");
  if(file2==NULL){
    fclose(file1);
    perror("Can't read second file");
    return;
  }
  while ((ch = fgetc(file1)) != EOF)
    if(ch!=fgetc(file2)){
      printf("0\n");
      return;
    }
  if(fgetc(file2)==EOF)
    printf("1\n");
  else
    printf("0\n");
  fclose(file1);
  fclose(file2);
  return;
}

/*
Inputs: None
Outputs:None
Usage:  Quitting smash program
*/
void quit(char** arguments){
  if(arguments[0]==NULL)
    exit(0);
  //If user requests killing all jobs before quitting
  if(!strcmp(arguments[0],"kill")){
    if(jobs.serial_num==0)
      exit(0);
    struct Job* job = &jobs;
    int status;
    while(1){
      printf("[%d] %s - Sending SIGTERM... ", job->serial_num, job->name);
      kill(job->pid, SIGTERM);
      // If job doesn't terminate, wait 5 seconds and kill it
      if(waitpid(job->pid, &status, WNOHANG))
        printf("Done.\n");
      else{
        sleep(5);
        printf("(5 sec passed)");
        if(!waitpid(job->pid, &status, WNOHANG)){
          printf(" Sending SIGKILL... ");
          kill(job->pid, SIGKILL);
          printf("Done.\n");
        }
      }
      if(job->next!=NULL)
        job = job->next;
      else
        exit(0);
    }
  }
}

/*
Inputs: Script's file name
        Arguments
Outputs:None
Usage:  Running the script
*/
void run_prog(char* prog_name, char** arguments){
  char* extended_arguments[MAX_COMMAND_ARGS+1];
  //initiate extended_arguments
  for(int i=0; i<MAX_COMMAND_ARGS+1; i++)
    extended_arguments[i] = NULL;
  //copy program name to extended_arguments
  extended_arguments[0] = prog_name;
  //copy all arguments to extended_arguments
  int i=0;
  while(i<MAX_COMMAND_ARGS && arguments[i]!=NULL){
    extended_arguments[i+1] = arguments[i];
    i++;
  }
  int child_id = fork();
  //child process
  if (child_id == 0){
    setpgrp();
    execve(prog_name, extended_arguments, NULL);
    printf("%s - %s\n", prog_name, strerror(errno));
    exit(errno);
  }
  //parent process: if last arg is '&', run child in background
  else
    if(!strcmp(extended_arguments[i],"&"))
      add_to_jobs(prog_name, child_id, false);
    else{
      add_to_jobs(prog_name, child_id, true);
      if(child_id==waitpid(child_id, NULL, 0))
        clear_job(child_id);
    }
}
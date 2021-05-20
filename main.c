#include "settings.h"
#include "smash_functions.h"
#include "aux_functions.h"

int main(void) {
  //Declare and initiate command history
  struct Command history[HISTORY_SZ];
  init_history(history);
  char history_str[MAX_COMMAND_SZ * HISTORY_SZ];
  int hist_count = 0;
  //Declare and initiate working directory
  struct Work_dir work_dir;
  load_pwd (work_dir.curr);
  load_pwd (work_dir.prev);
  //Initiate jobs linked list
  jobs.next = NULL;
  jobs.serial_num = 0;
  //Declare and initiate signal handling
  struct sigaction ctrlz_action;
  memset(&ctrlz_action, '\0', sizeof(ctrlz_action));
  ctrlz_action.sa_flags = SA_SIGINFO;
  sigemptyset(&ctrlz_action.sa_mask);
  ctrlz_action.sa_sigaction = ctrl_z;
  sigaction(SIGTSTP, &ctrlz_action, NULL);
  struct sigaction ctrlc_action;
  memset(&ctrlc_action, '\0', sizeof(ctrlc_action));
  ctrlc_action.sa_flags = SA_SIGINFO;
  sigemptyset(&ctrlc_action.sa_mask);
  ctrlc_action.sa_sigaction = ctrl_c;
  sigaction(SIGINT, &ctrlc_action, NULL);
  //Declare auxiliary variables
  char user_input[MAX_COMMAND_SZ];
  char user_input_raw[MAX_COMMAND_SZ];
  char* user_input_args[MAX_COMMAND_ARGS];

  while (1) {
    printf("smash> ");
    get_command(user_input, user_input_raw);
    breakup_command(user_input, user_input_args);
    //Using user input delay to wait for the jobs to finalize. Update only afterwards.
    update_jobs();

    if (!strcmp(user_input,"pwd")){
      printf("%s\n", work_dir.curr);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"cd")){
      cd(*user_input_args, work_dir.curr, work_dir.prev);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"history")){
      load_history(history_str, history);
      printf("%s", history_str);
    } 
    else if (!strcmp(user_input,"jobs")){
      print_jobs();
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"kill")){
      send_kill(user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"showpid")){
      printf("smash pid is %d\n", (int)getpid());
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"fg")){
      fg(user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"bg")){
      bg(user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"cp")){
      cp(user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"diff")){
      diff(user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
    else if (!strcmp(user_input,"quit"))
      quit(user_input_args);
    else {
      run_prog(user_input, user_input_args);
      insert_to_history(user_input_raw, &history[hist_count], &hist_count);
    }
  }
}
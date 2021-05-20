# Small-Shell
A small Linux shell, nicknamed Smash. It was created by me as part of an OS course assignment.  
## Features 
Smash supports the following commands:  
###### note: The follwoing command syntax has <> for mandatory arguments and [] for optional ones.
### Built-in commnads  
* `pwd` - show working directory.
* `cd <path>` - change working directory.
* `history` - show latest commands.
* `jobs` - show all running/stopped jobs.
* `kill -<signum> <job>` - send signum to job # from jobs.
* `showpid` - show Smash's own process ID.
* `fg [job number]` - run latest/specified job in foreground.
* `bg [job number]` - run latest/specified job in background.
* `quit [kill]` - exit smash and optionally kill all running jobs.
* `cp <old file name> <new file name>` - create a copy of an existing file.
* `diff <file1> <file2>` - compare the content of two files.
### Executables
* `<command> [arguments]` - run command in foreground.
* `<command> [arguments] &` - run command in background.
### Key strokes
* `ctrl+c` - kill foreground process.
* `ctrl+z` - stop foregroud process.
## Try it!
Smash is available for running at https://replit.com/@naftali10/OS-HW1  
The work directory has 2 executables to play with using Smash: count & print_hello.

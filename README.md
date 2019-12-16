# MavShell
A customized Shell command line interface to execute all major scripting commands utilizing the functionality of fork and exec and handling multiple processes in C 

# How to run:
 To run the program, type the following commands :
 
    make
    ./msh
 
# Commands Supported:
 The shell supports all commands in /bin, /usr/bin/ and /usr/local/bin/. Other than that the shell supports the following commands
  ## listpids
    This command will display the pids of the last 15 children.
  ## history
    This command will display the last 50 commands typed by the user
     Typing !n, where n is a number between 1 and 50 will
     result in the shell re-running the nth command. 

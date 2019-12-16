/*

  Name: Nihar Gupte
  ID: 1001556441

 */

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" /*  We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.    \
                           // In this case  white space                         \
                           // will separate the tokens on our command line */

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports ten arguments

//this function is to handle all signals encountered while execution of program.
static void handle_signal(int sig) //from shell assignment pdf reference https://stackoverflow.com/questions/12683466/after-suspending-child-process-with-sigtstp-shell-not-responding
{
  int status;
  switch (sig)
  {
  case SIGCHLD:
    // note that the last argument is important for the wait to work
    waitpid(-1, &status, WNOHANG);
    break;
  }
  //printf("Caught signal %d\n", sig);
  if (sig == 2) //ctrl c
  {
    kill(SIGKILL, getpid()); //killing the current process on ctrl c
  }
  if (sig == 20) //ctrl z
  {
    kill(SIGTSTP, getpid()); // temporarily stopping the process on ctrl z
    //kill(getpid,SIGSTOP);
  }
  printf("\nmsh> ");
}

//this function populates -1 to every index to initialize the array.
//-1 implies that a pid has not been stored in that index
//returns void, accepts int array of pid_numbers
void default_array(int pid_numbers[15])
{
  int i = 0;
  for (i = 0; i < 15; i++)
  {
    pid_numbers[i] = -1;
  }
}

//this function checks if the given string *str contains a semi colon or not
//returns -1 if it does not contain the semi colon, else returns the index containing the semi colon
//returns int, accepts char array
int contains_semi_colon(char *str)
{

  int i = 0;
  for (i = 0; i < strlen(str); i++)
  {
    if (str[i] == ';')
    {
      return i;
    }
  }
  return -1;
}

//this function arranges the array when number of commands is more than 15.
// it deletes the first command entered and shifts the entire array up by one place
// returns void, accepts the array of strings
void arrange_array(char history[15][255])
{
  int i;
  for (i = 0; i < 14; i++)
  {
    strcpy(history[i], history[i + 1]); //shifting up by one place
  }
}

//this function arranges the array when number of pids is more than 15.
// it deleletes the first pid record and shifts the entire array up by one place
// returns void, accepts int array of pids
void arrange_arraypids(int pids[])
{
  int i;
  for (i = 0; i < 14; i++)
  {
    pids[i] = pids[i + 1]; //shifting up
  }
}

//this function returns the numerical value from the given character using ASCII
// returns an int, and accepts a character
int get_number(char ch)
{
  int x = ch;
  return x - 48;
}

//the driver function
int main()
{

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
  char history[15][255];      //array to maintain history of commands
  int index_count = 0;        // count variable for history array
  int pid_numbers[15];        //array to maintain history of pids
  default_array(pid_numbers); //prepopulating and initializing the array
  //char history[15][];
  int count = 0; //count variable for pids array

  //signal handling from code_samples on cse 3320 gitbhub
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_handler = &handle_signal;
  if (sigaction(SIGINT, &act, NULL) < 0) // ctrl + c
  {
    perror("sigaction: sigint ");
    return 1;
  }
  if (sigaction(SIGTSTP, &act, NULL) < 0) // ctrl +z
  {
    perror("sigaction: sigtstp");
    return 1;
  }

  while (1)
  {

    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input

    //printf("1\n");

    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;
    //printf("1\n");

    //for blank input
    if (strlen(cmd_str) == 1) //if the user enters nothing, the shell shall skip the current iteration and move to next.
    {
      continue;
    }

    //handling inputs of the type !n where n is a int between 1 to 15
    int x = 0;
    if (cmd_str[0] == '!')
    {
      if (cmd_str[1] >= 49 && cmd_str[1] <= 57)
      {
        if (cmd_str[2] >= 48 && cmd_str[2] <= 53)
        {
          x = get_number(cmd_str[2]);
          x = 10 + x;
        }
        else
        {
          x = get_number(cmd_str[1]);
        }
        if (x <= index_count)
        {
          strcpy(cmd_str, history[x - 1]);
        }
        else
        {
          printf("Command not in history.\n");
          continue;
        }
      }
    }

    /* Parse input */
    //char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep

    char *working_root;

    //printf("%s %d\n",cmd_str,(int)strlen(cmd_str));
    int bool_flag = 0;
    //semi colon handling
    while (1)
    {

      char new_str2[255];

      char *token[MAX_NUM_ARGUMENTS];

      int contains_colon = contains_semi_colon(cmd_str); //gets to know whether we have semi colon in string or not
      //printf("%d\n", contains_colon);
      if (contains_colon != -1) //condition: semi colon present
      {
        if (bool_flag == 0)
        {
          if (index_count == 15) //if history reaches 15 commands, then move the entire array up by one place
          {
            arrange_array(history);
            index_count = 14;
          }

          strcpy(history[index_count], cmd_str); //considering the combined command of semi colon as one command in history
          index_count++;
          bool_flag = 2;
        }

        if (cmd_str[strlen(cmd_str)] != ';') //making sure a string ends with semi colon for string manipulation
        {
          cmd_str[strlen(cmd_str)] = ';';
        }

        char delim[] = ";";
        char *ptr = strtok(cmd_str, delim);

        if (ptr != NULL)
        {
          strcpy(cmd_str, ptr);
          ptr = strtok(NULL, delim);

          strcpy(new_str2, ptr);

          //new_str2[strlen(new_str2)] = ';';

          //printf("%s || %s \n", cmd_str, new_str2);
        }

        /*
        int i = 0;
        for (i = 0; i < contains_colon; i++)
        {
          new_str[i] = cmd_str[i];
        }
        new_str[contains_colon] = '\0';
        strcpy(cmd_str, new_str);
        int k = 0;
        for (i = contains_colon + 1; i < strlen(stored_str); i++)
        {
          new_str2[k] = stored_str[i];
          k++;
        }
        new_str2[k] = '\0'; */
      }
      else
      {
        if (bool_flag != 2)
        {
          bool_flag = 1;
        }
      }

      //printf("%s\n",new_str2);
      //printf("%s\n",cmd_str);
      char *working_str = strdup(cmd_str);
      char *arg_ptr;
      int flag2 = 0;

      if (bool_flag == 1)
      {
        if (index_count == 15) //if history reaches 15 commands, then move the entire array up by one place
        {
          arrange_array(history);
          index_count = 14;
        }
        strcpy(history[index_count], cmd_str);
        index_count++;
      }

      // we are going to move the working_str pointer so
      // keep track of its original value so we can deallocate
      // the correct amount at the end
      working_root = working_str;

      // Tokenize the input stringswith whitespace used as the delimiter
      //token_count = 0;
      while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
             (token_count < MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
        if (strlen(token[token_count]) == 0)
        {
          token[token_count] = NULL;
        }
        token_count++;
      }

      if (strcmp(token[0], "bg") == 0)
      {
        flag2 = 1;               //setting to 1 for semi colon processing
        kill(SIGCONT, getpid()); //continuing the background process on bg
        break;
      }

      if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0) //exit condition handled first
      {
        exit(0);
      }
      else if (strcmp(token[0], "cd") == 0) //cd had to be handled separately
      {
        flag2 = 1; //setting to 1 for semi colon processing
        if (token_count == 1)
        {
          chdir("/home/");
        }
        else
        {
          chdir(token[1]);
          strcpy(cmd_str, new_str2);
          if (flag2 == 1 || cmd_str == NULL)
          {
            break;
          }
        }
      }
      else if (strcmp(token[0], "listpids") == 0) //listing all pid numbers of processes
      {
        flag2 = 1; //setting to 1 for semi colon processing
        int i = 0;
        for (i = 0; i < 15; i++)
        {
          if (pid_numbers[i] == -1)
          {
            break;
          }
          printf("%d: %d\n", i, pid_numbers[i]);
        }
        strcpy(cmd_str, new_str2);
        if (flag2 == 1 || cmd_str == NULL)
        {
          break;
        }
      }
      else if (strcmp(token[0], "history") == 0) //listing all commands from history
      {
        flag2 = 1; //setting to 1 for semi colon processing
        int i = 0;
        for (i = 0; i <= index_count - 1; i++)
        {
          printf("%d: %s", i + 1, history[i]);
        }
        strcpy(cmd_str, new_str2);
        if (flag2 == 1 || cmd_str == NULL)
        {
          break;
        }
      }
      else //handles all typical linux commands
      {
        flag2 = 1;                //setting to 1 for semi colon processing
        char temp_str[246];       //255 is max limit and 10 are the max number of arguments allowed so 255-10+1=246
        char cwd[255];            //this stores the current working directory
        getcwd(cwd, sizeof(cwd)); //gets the cwd

        int pid = fork(); //create a child process to run all the commands in directories
        //printf("%d\n",pid);

        int status;

        if (pid == 0) //child process
        {

          strcpy(temp_str, cwd);      //copy cwd to temp_str
          strcat(temp_str, "/");      //format temp_str
          strcat(temp_str, token[0]); //specify path for execvp
          execvp(temp_str, token);    //exec in the specified path. if success, nothing below gets executed.
          //if fails, the following lines get executed

          strcpy(temp_str, "/usr/local/bin/"); //formatting path
          strcat(temp_str, token[0]);          //specify path for execvp
          execvp(temp_str, token);             //exec in the specified path. if success, nothing below gets executed.
          //if fails, the following lines get executed

          strcpy(temp_str, "/usr/bin/"); //formatting path
          strcat(temp_str, token[0]);    //specify path for execvp
          execvp(temp_str, token);       //exec in the specified path. if success, nothing below gets executed.
          //if fails, the following lines get executed

          strcpy(temp_str, "/bin/");  //formatting path
          strcat(temp_str, token[0]); //specify path for execvp
          execvp(temp_str, token);    //exec in the specified path. if success, nothing below gets executed.
          //if fails, the following lines get executed

          printf("%s: Command not found.\n", token[0]); //if control reaches this block
                                                        //implies that command not found in any of the specified directories.
                                                        // and execvp could not run.

          exit(0); //exiting the child process successfully implying command not found.
        }
        else
        {
          //pause();
          wait(&status); //wait for child process to complete so that msh> prompt is displayed at proper stage

          if (count == 15) //if array of pids reaches max value, shift up the array by one place
          {
            arrange_arraypids(pid_numbers);
            count = 14;
          }
          pid_numbers[count] = pid;
          count++;

          strcpy(cmd_str, new_str2);
          //printf("%s \n",cmd_str);
          if (flag2 == 1 || cmd_str == NULL)
          {
            break;
          }
        }
      }
    }
    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality
    /*
    int token_index = 0;
    for (token_index = 0; token_index < token_count; token_index++)
    {
      printf("token[%d] = %s\n", token_index, token[token_index]);
    }
*/
    free(working_root);
  }

  return 0;
}

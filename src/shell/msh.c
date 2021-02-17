/*
Name: Nicholas Untrecht
ID:   1001745062
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
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
// 7f704d5f-9811-4b91-a918-57c1bb646b70
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

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define MAX_PROCESSES_SHOWN 15 // Restraint for listpids()
#define MAX_COMMANDS_SHOWN 16 // Restraint for 'history'

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  // Number for counting processes forked
  // Delcared outside of while() to prevent reseting of value.
  int pid_num = 0;

  // Counts commands inputted for 'history'
  int com_num = 0;

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }


    // Handles empty input so program doesn't seg fault.
    // Restarts the loop when empty.
    if( token[0] == NULL )
      continue;


    /*
    /  array for 'history' command.
    /  copies from token and puts into history array
    /  If the max number of commands has been reached,
    /    shifts array left one and continues to override last spot.
    /  This function keeps the com_num below its max.
   */
    char * com_arr[MAX_COMMANDS_SHOWN];
    if( com_num >= MAX_COMMANDS_SHOWN )
    {
      for(int i = 0; i < com_num - 1; i++)
      {
        free( com_arr[i] );
        com_arr[i] =  strdup( com_arr[i+1] );
      }

      free( com_arr[ com_num - 1 ] );
      com_arr[com_num] = strdup( token[0] );
    }
    else
      com_arr[com_num++] = strdup( token[0] );

    // When 'history' command used, prints out the counted number of commands.
    if( !strcmp( token[0], "history") )
    {
      for(int i = 0; i < com_num; i++)
        printf("[%d]: %s\n", i, com_arr[i]);
      continue;
    }


    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }


    // Exits the shell when either the exit or quit commands are called
    if( !strcmp(token[0], "exit") || !strcmp(token[0], "quit") || !strcmp(token[0], "q" ) )
      return (EXIT_SUCCESS);


    /*
    /  Uses chdir() to change directory.
    /  Doesn't fork as only works in parent.
    /  Restarts loop upon completion
   */
    if( !strcmp(token[0], "cd") )
    {
      chdir(token[1]);
      continue;
    }


    pid_t pid_arr[MAX_PROCESSES_SHOWN];

    if( !strcmp(token[0], "listpids") )
    {
      for(int i = 0; i < pid_num; i++)
        printf("%d: %d\n", i, pid_arr[i]);
      continue;
    }

    /*
    /  If command doesn't need special handling, forks the process and CHILD execs.
    /  If the command isn't found, error message prints 
    /  Parent process waits for child to finish execution.
    /  Continues shell once child is complete.
   */
    pid_t pid = fork();

    /*
    /  If the max listpid limit hasn't been reached...
    /  ...put the recently created pid into the array.
    /
    /  Once limit has been reached, shift array over to left...
    /  ...and replace the right most value.
   */
    if( pid != 0 )
    {
      if( pid_num >= MAX_PROCESSES_SHOWN )
      {
        for(int i = 0; i < pid_num - 1; i++)
        {
          pid_arr[i] = pid_arr[i+1];
        }
        pid_arr[pid_num-1] = pid;
      }
      else
      {
        pid_arr[pid_num++] = pid;
      }
    }


    int exec; // For error handling
    /*
    /  Goes into child process and attempts to execute given command.
    /  Prints out error message if failure.
    /  Exits the child process.
   */
    if(pid == 0)
    {
      exec = execvp(token[0], &token[0]);

      if(exec == -1)
        printf("%s: Command not found.\n", token[0]);

      exit( EXIT_SUCCESS );
    }


    // Parent process waits for child to complete its execution.
    if( pid != 0 )
    {
      int status;
      waitpid(pid, &status, 0);
    }


    free( working_root );

  }
  return 0;
}


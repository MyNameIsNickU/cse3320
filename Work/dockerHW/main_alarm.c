// The MIT License (MIT)
//
// Copyright (c) 2017 Trevor Bakker
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

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void signalHandler(int signum)
{
    printf("Alarm rang and child is exiting!");
    exit(EXIT_SUCCESS);
}



/*
  fork() a child and print a message from the parent and 
  a message from the child
*/
int main( void )
{
  /*
    Override default behavior of the specific signal "SIGALRM"
    and instead goes to the specified function
    so when alarm rings, the program doesn't exit.
  */
  signal(SIGALRM, signalHandler);

  pid_t pid = fork();

  alarm(10); // After 10 seconds, SIGALRM will be generated.

  if( pid == -1 )
  {
    // When fork() returns -1, an error happened.
    perror("fork failed: ");
    exit( EXIT_FAILURE );
  }

  else if ( pid == 0 )
  {
    // When fork() returns 0, we are in the child process.
    printf("Hello from the child process\n");
    int i;
    for(i = 10; i >= 0; i--)
    {
      printf("Child counting down...\t%d seconds remaining...\n", i);
      fflush(NULL);
      sleep(1);
    }
  }
  else 
  {
    // When fork() returns a positive number, we are in the parent
  	  // process and the return value is the PID of the newly created
    // child process.
    int status;

    // Force the parent process to wait until the child process 
    // exits
    waitpid(pid, &status, 0 );
    printf("Hello from the parent process\n");
    fflush( NULL );
  }
  return EXIT_SUCCESS;
}

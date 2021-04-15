/*
Nicholas Untrecht
1001745062
CSE 3320 - Sec 003
*/

// The MIT License (MIT)
//
// Copyright (c) 2020 Trevor Bakker
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size


FILE *fp;

 /*
 /
*/
int fat_open( char * filename )
{
  if( fp != NULL )
  {
    printf("Error: File system image already open.\n");
    return 0;
  }

  fp = fopen( filename, "r" );
  if( fp == NULL )
  {
    printf("Error: File system image not found.\n");
    return -1;
  }
  else
  {
   printf("Image opened successfully.\n");
   return 1;
  }
}

 /*
 /
*/
int fat_close()
{
  if( fp != NULL )
  {
    printf("Image closed successfully.\n");
    fclose( fp );
    fp = NULL;
    return 1;
  }
  else
  {
    printf("Error: File system not open.\n");
    return -1;
  }
}

char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;

 /*
 /
*/
void fat_info()
{
  printf("---INFO---\n");


  fseek( fp, 3, SEEK_SET);
  fread( BS_OEMName, 8, 1, fp );
  printf("BS_OEMName = %s\n", BS_OEMName);

  fseek( fp, 11, SEEK_SET );
  fread( &BPB_BytsPerSec, 2, 1, fp );
  printf("BPB_BytsPerSec = %d\n", BPB_BytsPerSec);

  fseek( fp, 13, SEEK_SET );
  fread( &BPB_SecPerClus, 1, 1, fp );
  printf("BPB_SecPerClus = %d\n", BPB_SecPerClus);

  fseek( fp, 14, SEEK_SET );
  fread( &BPB_RsvdSecCnt, 2, 1, fp );
  printf("BPB_RsvdSecCnt = %d\n", BPB_RsvdSecCnt);

  fseek( fp, 16, SEEK_SET );
  fread( &BPB_NumFATs, 2, 1, fp );
  printf("BPB_NumFATs = %d\n", BPB_NumFATs );

  fseek( fp, 36, SEEK_SET );
  fread( &BPB_FATSz32, 4, 1, fp );
  printf("BPB_FATzS32 = %d\n", BPB_FATSz32);

  return;
}
int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

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
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }


    // Trying to use command when file system isn't open
    if( fp == NULL && strcmp( token[0], "open" ) )
    {
      printf("Error: File system image must be opened first.\n");
      continue;
    }

    /*
    /  OPEN THE FILE SYSTEM = 'open'
    /  Attempts to open the file system if the file pointer is empty.
    /  If the file system is already open, pops an error.
    /  If the file system CANNOT be opened, pops an error.
   */
    if( !strcmp( token[0], "open" ) )
    {
      fat_open( token[1] );
    }

    /*
    /  CLOSE THE FILE SYSTEM = 'close'
    /  Attempts to close the file system if the file pointer is set to a value...
    /  ...sets file pointer to NULL after closing
    /  If the file system is already closed, pops an error.
   */
    if( !strcmp( token[0], "close") )
    {
      fat_close();
    }

    /*
    /  LIST IMAGE INFO = 'info'
    /  description
   */
    if( !strcmp( token[0], "info") )
    {
      fat_info();
    }


    free( working_root );

  }
  return 0;
}

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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size


int currPos = 0;
FILE *fp;

struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstCluterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];


int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;
int32_t ClusterSize;

 /*
 /  Function: LBA To Offset
 /  Paramters: Current sector numbers - points to block of data
 /  Returns: Value of address for that block of data
*/
int LBAToOffset( int32_t sector )
{
  return (( sector - 2 ) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

 /*
 /  Function: Next LB
 /  Parameters: Current sector number
 /  Description: Given block addr., look up in first FAT and return block addr. of block in file...
 /  ...returns -1 if no further blocks.
*/
int16_t NextLB( uint32_t sector )
{
  uint32_t FATAddress = ( BPB_BytsPerSec * BPB_RsvdSecCnt ) + ( sector * 4 );
  int16_t val;
  fseek( fp, FATAddress, SEEK_SET );
  fread( &val, 2, 1, fp );
  return val;
}

 /*
 /
*/
void fat_InfoFill()
{
  fseek( fp, 11, SEEK_SET );
  fread( &BPB_BytsPerSec, 2, 1, fp );

  fseek( fp, 13, SEEK_SET );
  fread( &BPB_SecPerClus, 1, 1, fp );

  fseek( fp, 14, SEEK_SET );
  fread( &BPB_RsvdSecCnt, 2, 1, fp );

  fseek( fp, 16, SEEK_SET );
  fread( &BPB_NumFATs, 2, 1, fp );

  fseek( fp, 36, SEEK_SET );
  fread( &BPB_FATSz32, 4, 1, fp );

  ClusterSize = BPB_SecPerClus * BPB_BytsPerSec;

  return;
}

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
   fat_InfoFill();
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

void fat_infoList()
{
  printf("---INFO---\n");
  printf("BPB_BytsPerSec = %7d%15x\n", BPB_BytsPerSec, BPB_BytsPerSec);
  printf("BPB_SecPerClus = %7d%15x\n", BPB_SecPerClus, BPB_SecPerClus);
  printf("BPB_RsvdSecCnt = %7d%15x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
  printf("BPB_NumFATs = %7d%15x\n", BPB_NumFATs, BPB_NumFATs);
  printf("BPB_FATzS32 = %7d%15x\n", BPB_FATSz32, BPB_FATSz32);
  return;
}


 /*
 /
*/
void fat_ls()
{
  int start = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);

  if( currPos == 0 )
    currPos = start;

  fseek( fp, currPos, SEEK_SET );
  int i;
  for(i = 0; i < 16; i++)
  {
    fread( &dir[i], sizeof( struct DirectoryEntry ), 1, fp );
  }

  char filename[12];
  for(i = 0; i < 16; i++)
  {
    if( dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20 )
    {
      strncpy( filename, dir[i].DIR_Name, 11 );
      filename[11] = '\0';
      if( !((filename[0] & 0x000000e5) == 0x000000e5) )
        printf("%s\n", filename);
    }
  }

  return;
}

int file2index( char * filename )
{
  int check = 15;
  char string[12];

  while( check >= 0 )
  {
    strncpy( string, dir[check].DIR_Name, 11 );
    string[11] = '\0';

    strtok( string, " " );

    if( !strcmp( filename, string ) )
      break;

    check--;
  }

  return check;

}

 /*
 /
*/
void fat_cd( char * folder )
{
  int check = file2index( folder );
  int currClus;

  if( check == -1 )
  {
    printf("Error: Directory %s not found.\n", folder);
  }
  else
  {
    if( dir[check].DIR_Attr == 0x10 )
    {
      currClus = dir[check].DIR_FirstClusterLow;
      if( currClus == 0 )
        currClus = 2;
      currPos = LBAToOffset( currClus );
    }
    else
      printf("Error: File found but not subdirectory.\n");
  }

  return;
}

 /*
 /
*/
void fat_stat( char * filename )
{
  int check = file2index( filename );
  if( check == -1 )
  {
    printf("Error: File %s not found.\n", filename);
    return;
  }
  printf("Attribute\tSize\tStarting Cluster Number\n");
  printf("%d\t\t%d\t%d\n", dir[check].DIR_Attr, dir[check].DIR_FileSize, dir[check].DIR_FirstClusterLow );

  return;
}

 /*
 /
*/
void fat_get( char * filename )
{
  printf("Getting file %s...\n", filename);
  int check = file2index( filename );

  if( check == -1 )
  {
    printf("Error: File %s not found.\n", filename);
    return;
  }

  FILE *newFile;
  newFile = fopen( filename, "w" );
  if( newFile == NULL )
  {
    printf("Error: Failed to create file: %s", filename );
  }

  int nextClus, readPos;
  int currClus = dir[check].DIR_FirstClusterLow;

  while( currClus != -1 )
  {
    nextClus = NextLB( currClus );
    printf("The current cluster # is: %d\tThe Next Cluster # is: %d\n", currClus, nextClus);
    readPos = LBAToOffset( currClus );
    fseek( fp, readPos, SEEK_SET );
    currClus = nextClus;

  }

  fseek( fp, currPos, SEEK_SET );

  if( newFile != NULL )
    fclose( newFile );
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
      fat_infoList();
    }

    /*
    /  LIST DIRECTORY CONTENTS = 'ls'
    /  description
   */
    if( !strcmp( token[0], "ls") )
      fat_ls();

    if( !strcmp( token[0], "cd") )
      fat_cd( token[1] );

    if( !strcmp( token[0], "stat") )
      fat_stat( token[1] );

    if( !strcmp( token[0], "get") )
      fat_get( token[1] );

    free( working_root );

  }
  return 0;
}

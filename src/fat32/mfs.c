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
#include <ctype.h>

#define MAX_NUM_ARGUMENTS 4

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

// Tracks the current position in the File System.
int currPos = 0;

// File pointer for opening and navigating File System Image.
FILE *fp;

 /*
 /   Packed struct for each entry in the FAT32 system.
 /   When read, data will go directly into variables without padding.
*/
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

// Array of each entry that is refilled on each movement through file system.
struct DirectoryEntry dir[16];


// Statistic variables for FAT32
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
 /   Seeks the file pointer to the correct position in the file system.
 /   Runs on initial read of filesystem, before each ls, and on each cd move.
*/
void updatePosition()
{
  if( currPos == 0 )
    currPos = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
  fseek( fp, currPos, SEEK_SET );
  int i;
  for(i = 0; i < 16; i++)
    fread( &dir[i], sizeof( struct DirectoryEntry ), 1, fp );
  return;
}

 /*
 /  Fills the statistic variables on the inital read of FAT32 File System.
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
 /   Opens the file system and sets the global file pointer.
 /   Error handling for already open case and can't open case.
 /   If successful, fills the statistic variables and updates the file position to the root directory.
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
   updatePosition();
   return 1;
  }
}

 /*
 /   Closes the file pointer to the FAT32 image.
 /   Handles the case if the fp isn't open.
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


// Lists the FAT32 Stats in decimal and hex.
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
 /   Lists the entires in the current directory.
 /   Updates the entry array and file pointer.
 /   Checks the file attribute for valid printable entries.
*/
void fat_ls()
{
  updatePosition();

  printf(".\n");
  char filename[12];
  int i;
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

 /*
 /   Returns value if the input file name is equal to the entry name.
 /   Tokenizes the input and expands it into the same format as FAT32 entry.
 /   Handles the '..' for 'cd' by checking that input individually and seperately returning.
*/
int isMatch(char * rawIMG, char * input)
{
  char imgName[12];
  strncpy( imgName, rawIMG, 11 );
  imgName[11] = '\0';

  int len = strlen(input);
  char workInput[ len + 1 ];
  strncpy( workInput, input, len + 1 );

  char expanded_name[12];
  memset( expanded_name, ' ', 12 );

  char * token = strtok( workInput, "." );

  // Handler for 'cd ..'
  if( token == NULL )
  {
    strtok(imgName, " ");
    if( !strcmp( imgName, "..") )
      return 1;
    else
      return -1;
  }

  strncpy( expanded_name, token, strlen(token) );

  token = strtok( NULL, "." );

  if( token )
    strncpy( (char*)(expanded_name+8), token, strlen( token ) );

  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
    expanded_name[i] = toupper( expanded_name[i] );

  if( strncmp( expanded_name, imgName, 11) == 0 )
    return 1;
  else
    return -1;

}


 /*
 /   Checks the entry directory array for the given string...
 /   ...and returns the index if found.
 /   Returns -1 if not found.
*/
int file2index( char * filename )
{
  int check = 15;
  char string[12];

  while( check >= 0 )
  {
    strncpy( string, dir[check].DIR_Name, 11 );
    string[11] = '\0';

    strtok( string, " " );

    if( isMatch(dir[check].DIR_Name, filename) == 1 )
      break;

    check--;
  }

  return check;

}

 /*
 /   Changes directory to the specified directory.
 /   Finds the given folder's index and checks if it is a directory.
 /   If directory is found, the LowCluster# is used to find file offset...
 /   ...and the file pointer seeks to the offset location.
*/
void fat_cd( char * folder )
{
  if( folder == NULL )
  {
    printf("Error: 'cd' requires second argument.\n");
    return;
  }
  else if( !strcmp( folder, ".") )
    return;

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

  updatePosition();

  return;
}

 /*
 /   If the file exists, finds the specified filename's index.
 /   Prints out the attribute, file size, and cluster number of file.
*/
void fat_stat( char * filename )
{
  if( filename == NULL )
  {
    printf("Error: 'stat' requires second argument.\n");
    return;
  }
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
 /   Gets a file from the FAT32 File System...
 /   ...and puts the file in the relative directory of the utility.
 /   Creates an empty file with correct type using last 3 chars of filename.
 /   Seeks to the correct cluster and begins reading data into the file.
 /   If the cluster ends and there is more data to read, find the next cluster.
 /   If all of the requested file size has been read, close the file and return.
*/
void fat_get( char * filename )
{
  if( filename == NULL )
  {
    printf("Error: 'get' requires second argument.\n");
    return;
  }
  int check = file2index( filename );

  if( check == -1 )
  {
    printf("Error: File %s not found.\n", filename);
    return;
  }

  if( dir[check].DIR_Attr == 0x10 )
  {
    printf("Error: File is subdirectory, cannot 'get'.\n");
    return;
  }

  FILE *newFile;
  newFile = fopen( filename, "w" );
  if( newFile == NULL )
  {
    printf("Error: Failed to create file: %s", filename );
  }

  uint32_t sizeLeft = dir[check].DIR_FileSize;
  int currClus = dir[check].DIR_FirstClusterLow;
  char transfer;
  int bCount = 0;

  while( currClus != -1 )
  {
    fseek( fp, LBAToOffset( currClus ), SEEK_SET );

    // Reads 32 Bytes from image file and copies it to the newfile.
    while( sizeLeft > 0 && bCount != BPB_BytsPerSec )
    {
      transfer = getc( fp );
      putc( transfer, newFile );
      sizeLeft--;
      bCount++;
    }
    bCount = 0;
    currClus = NextLB( currClus );

  }

  fseek( fp, currPos, SEEK_SET );

  if( newFile != NULL )
    fclose( newFile );
  return;
}

 /*
 /   Reads specified bytes from specified file.
 /   Seeks to the correct initial cluster based on input position.
 /   Prints out the specified bytes.
 /   If the data is in different clusters, move across to the next cluster.
*/
void fat_read( char * filename, int pos, int byt )
{

  int check = file2index( filename );
  if( check == -1 )
  {
    printf("Error: File %s not found.\n", filename);
    return;
  }

  if( dir[check].DIR_Attr == 0x10 )
  {
    printf("Error: File is subdirectory, cannot 'read'.\n");
    return;
  }

  uint32_t sizeLeft = byt;
  int clusNum = pos / ClusterSize;
  int currClus = dir[check].DIR_FirstClusterLow;

  while( clusNum > 0 )
  {
    currClus = NextLB( currClus );
    clusNum--;
  }
  clusNum = pos % ClusterSize;
  fseek( fp, LBAToOffset( currClus ), SEEK_SET );
  fseek( fp, clusNum, SEEK_CUR );

  char transfer;
  int bCount = 0;
  int bytsRead = 0;

  while( bytsRead != byt && currClus != -1 )
  {
    // Reads 32 Bytes from image file and copies it to the newfile.
    while( sizeLeft > 0 && bCount != ClusterSize )
    {
      transfer = getc( fp );
      printf("%x ", transfer);
      sizeLeft--;
      bCount++;
    }
    bCount = 0;
    currClus = NextLB( currClus );
    fseek( fp, LBAToOffset( currClus ), SEEK_SET );
  }
  printf("\n");
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


    if(token[0] == NULL)
      continue;

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
      continue;
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
      continue;
    }

    /*
    /  LIST IMAGE INFO = 'info'
    /  Prints out information about the FAT 32 file system.
   */
    if( !strcmp( token[0], "info") )
    {
      fat_infoList();
      continue;
    }

    /*
    /  LIST DIRECTORY CONTENTS = 'ls'
    /  Lists the directory contents.
    /  Handles 'ls .' by doing a normal, no argument 'ls'
    /  if 'ls ..' is used, do an internal 'cd ..' and normal 'ls'...
    /  ...and then got back to the old position.
   */
    if( !strcmp( token[0], "ls") )
    {
      int oldPos = currPos;
      if( token[1] != NULL )
      {
        if( !strcmp( token[1], ".") )
          fat_ls();
        else if( !strcmp( token[1], ".."))
        {
          fat_cd("..");
          fat_ls();
          currPos = oldPos;
        }
      }
      else
        fat_ls();
      continue;
    }


    /*
    /   CHANGE DIRECTORY = 'cd'
    /   Parses the input for relative pathing (e.g. 'FOLDERA/FOLDERC/../FOLDERC')
    /   While there are still '/' indicating relative paths, input each folder name...
    /   ...into the fat_cd() function.
   */
    if( !strcmp( token[0], "cd") )
    {
      char * cd_array[255];
      char * found, *cd_arg;
      int cd_count = 0;
      cd_arg = strdup (token[1]);

      while( (found = strsep( &cd_arg, "/")) != NULL )
      {
        cd_array[cd_count++] = strdup( found );
      }

      int i;
      for(i = 0; i < cd_count; i++)
        fat_cd( cd_array[i] );
      continue;
    }

    /*
    /   FILE ATTRIBUTES = 'stat'
    /   Prints out data about specified file or directory.
   */
    if( !strcmp( token[0], "stat") )
    {
      fat_stat( token[1] );
      continue;
    }

    /*
    /   GET FILE = 'get'
    /   Pulls file from the file system and puts it in utility directory.
   */
    if( !strcmp( token[0], "get") )
    {
      fat_get( token[1] );
      continue;
    }

    /*
    /   READ FILE = 'read'
    /   Reads specified file for a specified # of bytes.
    /   Checks if the required arguments are inputted.
   */
    if( !strcmp( token[0], "read") )
    {
      if( token[1] == NULL || token[2] == NULL || token[3] == NULL )
      {
        printf("Error: 'read' requires 3, non-zero arguments.\n");
        continue;
      }
      fat_read( token[1], atoi(token[2]), atoi(token[3]) );
      continue;
    }

    /*
    /   Catch-all for unrecognized commands.
   */
    if(token[0] != NULL)
      printf("Error: command '%s' not found.\n", token[0]);

    free( working_root );

  }
  return 0;
}

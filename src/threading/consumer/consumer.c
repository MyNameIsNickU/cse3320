#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>

#define QUEUE_SIZE 5

sem_t waitQueue, printedQueue;

char fileQueue[QUEUE_SIZE];
int spot = 0;

FILE *fp;

int readf( char * filename )
{
  if( (fp = fopen( filename, "r" )) == NULL)
  {
    printf("Can't open file: %s...exiting!", filename);
    exit( EXIT_FAILURE );
  }
  return 0;
}

void * Producer( void * arg )
{
  printf("Producer created!\n");

  char c;

  if( (c = fgetc( fp )) == '\0')
    exit( EXIT_FAILURE );
  else
  {
    fileQueue[spot++] = c;
    //sem_post( &waitQueue );
  }


  while( (c = fgetc( fp )) )
  {
    sem_wait( &printedQueue );


    fileQueue[spot++] = c;
    sem_post( &waitQueue );

    if( spot == QUEUE_SIZE )
      spot = 0;
  }
  return NULL;
}

void * Consumer( void * arg )
{
  printf("Consumer created!\n");

  char printChar = 'a';
  int printSpot = 0;
  int fileStatus;

  while( feof( fp ) == 0 )
  {
    sem_wait( &waitQueue );

    printChar = fileQueue[printSpot++];
    if( printSpot == QUEUE_SIZE )
      printSpot = 0;

    sem_post( &printedQueue );
    //fflush(stdin);
    printf("%c", printChar);
  }
  return NULL;
}


int main( int argc, char * argv[] )
{
  pthread_t producer_tid, consumer_tid;

  if( argc < 2)
  {
    printf("File required in argument, exiting...");
    return 0;
  }

  readf( argv[1] );

  sem_init( &printedQueue, 1, 0);
  sem_init( &waitQueue, 1, 1);

  pthread_create( &producer_tid, NULL, Producer, NULL);
  pthread_create( &consumer_tid, NULL, Consumer, NULL);

  pthread_join( producer_tid, NULL);
  pthread_join( consumer_tid, NULL);
}

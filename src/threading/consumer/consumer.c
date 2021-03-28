#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>

#define QUEUE_SIZE 5

sem_t waitQueue, printedQueue;

char fileQueue[QUEUE_SIZE];

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

// 'Produces' the wait queue by adding chars into the circular queue. Relies on the number of processed chars.
void * Producer( void * arg )
{
  int spot = 0;
  char c;

  /*
  /  Until the file reaches its end, it will add each char of the file to the circ. QUEUE.
  /  Increments the waiting char sem.
  /  If printed (processed) char sem. is 0, it will wait until more finish being processed.
 */
  while( feof( fp ) == 0 )
  {
    c = fgetc( fp );
    sem_wait( &printedQueue );

    fileQueue[spot++] = c;
    sem_post( &waitQueue );

    if( spot == QUEUE_SIZE )
      spot = 0;
  }
  return NULL;
}

// 'Consumes' the chars in the wait circular queue by printing them. Relies on number of chars waiting to be processed.
void * Consumer( void * arg )
{
  char printChar = 'a';
  int printSpot = 0;
  int fileStatus = 0;

  /*
  /  Until the file reaches its end, print out char in the circular queue...
  /  Increments printedQueue sem. each time char processed.
  /  If waiting chars are 0, it will wait until it is given more chars to process.
 */
  while( !fileStatus )
  {
    sem_wait( &waitQueue );

    printChar = fileQueue[printSpot++];
    if( printSpot == QUEUE_SIZE )
      printSpot = 0;

    sem_post( &printedQueue );
    printf("%c", printChar);
    fileStatus = feof( fp );
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

  sem_init( &printedQueue, 1, 1);
  sem_init( &waitQueue, 1, 0);

  pthread_create( &producer_tid, NULL, Producer, NULL);
  pthread_create( &consumer_tid, NULL, Consumer, NULL);

  pthread_join( producer_tid, NULL);
  pthread_join( consumer_tid, NULL);

  fclose( fp );
}

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define MAX 5000000
#define NUM_THREADS 4

int total = 0;
int n1,n2;
char *s1,*s2;
int divisions;

pthread_mutex_t mutex;

FILE *fp;

int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }

    s1=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }

    s2=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }

    /*read s1 s2 from the file*/

    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2)-1; /*length of s2*/

    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }

    return 1;
}

void * num_substring ( void * ptr )
{
    //pthread_mutex_lock( &mutex );
    int i,j,k;
    int count;
    int start, end;

    start = *(int*)ptr;
    end = start + divisions;

    for (i = start; i <= end; i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2)
                total++; /*find a substring in this step*/
         }
    }
    //pthread_mutex_unlock( &mutex );
    return NULL;
}

int main(int argc, char *argv[])
{
    int count;

    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] ) ;

    struct timeval start, end;
    float mtime;
    int secs, usecs;

    gettimeofday(&start, NULL);


    pthread_t substring_thread[NUM_THREADS];

    divisions = n1/NUM_THREADS;
    int thread_start;

    int i ;
    for(i = 0; i < NUM_THREADS; i++)
    {
      thread_start = divisions * i;
      if( pthread_create(&substring_thread[i], NULL, num_substring, (void*) &thread_start ) )
      {
        perror("Thread creation error: ");
        exit( EXIT_FAILURE );
      }
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
      if( pthread_join(substring_thread[i], NULL) )
      {
        perror("Thread join error: ");
      }
    }

    //count = num_substring () ;
    count = total;

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , count) ;
    printf ("Elapsed time is : %f milliseconds\n", mtime );

    if( s1 )
    {
      free( s1 );
    }

    if( s2 )
    {
      free( s2 );
    }

    return 0;
}

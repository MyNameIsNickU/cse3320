//#include <stdlib.h>
#include <stdio.h>

int main()
{
  printf("Running test 1 to test a simple malloc and free\n");

  char * ptr = ( char * ) calloc ( 65535 , sizeof(char));

  int i;
  for(i = 0; i < 65534; i++)
    printf("data = %c", ptr[i]);

  free( ptr ); 

  return 0;
}

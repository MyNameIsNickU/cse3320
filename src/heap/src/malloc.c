#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;

static int num_mallocs       = -1; // Implemented
static int num_frees         = 0; // Implemented
static int num_reuses        = 0; // Implemented
static int num_grows         = 0; // Implemented
//static int num_splits        = 0;
//static int num_coalesces     = 0;
static int num_blocks        = 0; // Implemented
static int num_requested     = 0; // Implemented
static int max_heap          = 0; // Implemented

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  //printf("splits:\t\t%d\n", num_splits );
  //printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *lastUsed = NULL;


/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 */
struct _block *findFreeBlock(struct _block **last, size_t size)
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }

#endif

#if defined BEST && BEST == 0
   /* Best Fit
   /
   /  Start at the beginning of the heap and search the entire heap for the BEST fit.
   /  The best fit is when the potential leftover size is the LEAST.
   /  We loop through each block and check what the best leftover size is, and save that block pointer.
   /
  */
   int leftoverBest = INT_MAX;
   struct _block *best = NULL;

   while (curr)
   {
      if(curr->size - size < leftoverBest && curr->size - size >= 0 && curr->free)
      {
        leftoverBest = curr->size - size;
        best = curr;
      }
      *last = curr;
      curr  = curr->next;
   }
   curr = best;
#endif

#if defined WORST && WORST == 0
   /* Worst Fit
   /
   /  Start at the beginning of the heap and search the entire heap for the WORST fit.
   /  The best fit is when the potential leftover size is the GREATEST.
   /  We loop through each block and check what the worst leftover size is, and save that block pointer.
   /
  */
   int leftoverWorst = 0;
   struct _block *worst = NULL;

   while (curr)
   {
      if(curr->size - size > leftoverWorst && curr->size - size >= 0 && curr->free)
      {
        leftoverWorst = curr->size - size;
        worst = curr;
      }
      *last = curr;
      curr  = curr->next;
   }
   curr = worst;
#endif

#if defined NEXT && NEXT == 0
   /*  Next Fit
   /   The same as first fit except the previous pointer allocated is the starting point...
   /   ...for the first fit search.
   /   First loop starts from the last allocated pointer and goes to the end of the list.
   /   If a spot was not found, start from the beginning of heap and go to the lastly allocated pointer.
   /   Global variable used for previously used pointer.
  */
   if(lastUsed != NULL)
     curr = lastUsed;
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }

   // Didn't find spot from previous to end.
   if( curr == NULL )
   {
     curr = heapList;
     while(curr != lastUsed && !(curr->free && curr->size >= size))
     {
       *last = curr;
       curr = curr->next;
     }
   }

#endif

   // If a fit algorithm succeeded, then a reuse occured.
   if(curr != NULL)
     num_reuses++;

   // Saves the last allocated pointer.
   lastUsed = curr;
   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size)
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1)
   {
      return NULL;
   }

   // Update grow count
   num_grows++;

   /* Update heapList if not set */
   if (heapList == NULL)
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last)
   {
      last->next = curr;
   }

   max_heap += size + sizeof(struct _block);

   /* Update _block metadata (INCLUDING NUMBER OF BLOCKS) */
   curr->size = size;
   curr->next = NULL;
   num_blocks++;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   // The requested size is the input size for the function call.
   num_requested += size;

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }


   // If the NULL checks have succeeded, then a malloc is going to occur.
   num_mallocs++;

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   //lastUsed = last;

   /* TODO: Split free _block if possible */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL
   /  Fixes the problem when growHeap is called and grow/block counters are always allocated.
  */
   if (next == NULL) 
   {
      num_grows--;
      num_blocks--;
      return NULL;
   }

   /* Mark _block as in use */
   next->free = false;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}


/*
 * \brief realloc
 *
 * Takes a pointer to previously allocated memory and changes the size of allocated memory.
 *
 * \param ptr Pointer to previously allocated memory.
 * \param size Desired size of the newly allocated memory.
 *
 * \return Returns the pointer to the newly allocated memory.
 *
 */
void * realloc(void * ptr, size_t size)
{
  void * new_malloc = malloc(size);
  memcpy(new_malloc, ptr, size);
  free(ptr);
  return new_malloc;
}


/*
 * \brief calloc
 *
 * Similar to malloc, except allocated memory is also initialized to 0.
 *
 * \param nmemb Specified number of elements of memory allocated to be initalized.
 * \param size Desired size of the allocated elements.
 *
 * \return Returns the pointer to the newly allocated memory (that is initialized).
 *
 */
void * calloc(size_t nmemb, size_t size)
{
  void * ptr;
  ptr = malloc( nmemb * size );
  memset(ptr, '\0', nmemb * size);
  return ptr;
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   num_frees++;
   return;

   /* TODO: Coalesce free _blocks if needed */
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/

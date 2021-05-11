# cse3320
Code hub/backup for my Operating Systems class.
4 Assignments done over the course of the semester are here.
These are some general notes and descriptions of the more interesting things used in the assignment.

---

# Assignment #1 - Shell
Simple shell that performs a few predetermined commands like `history` and `listpids` that show previous commands and their process id's respectively.
All other command inputs are passed into exec() and treated like normal unix commands.

## `history`
I store the raw input (before tokenizing the arguements and such) into an array; index is tracked with a seperate integer.
There is a max command limit, so a circular queue is used for storing and printing out each previous command.
I use a bool integer to signal when the max limit has been reached so I can specifically handle those instances when printing or replacing a command in the history.
### BANG
`![index]` is supported by pulling the specified entry from the history array and putting that raw string as the "input" to the rest of the code; it gets tokenized and executes normally like the user typed it in. Storing the raw string instead of the tokenized string makes this super easy.

## `listpids`
When the command entered is a normal UNIX command and not something special in the shell (like `history`), I use `exec()` to attempt the command.
The process forks; the child uses `exec()` and the parent - the shell - waits for the child to finish.

When the process forks, I also store the new PID into an array for `listpids`.
Whent the max limit for `listpids` is reached, unlike the `history` command, I shift the array over left one and overwrite the right-most spot (I didn't think about circular queues until this was already done).

---

# Assignment #2 - Threading
This was another fairly simple one about using threads, mutexes, and semaphores to read through a really big text file of classic English Literature.
I used the pthread library since it's included in most c compilers these days.
* One goal was to find substrings in the textfile, but have multiple threads doing it at once.
* The other goal was to have a consumer/producer algorithm with the textfiles, sending in some chars (producers) and printing those chars out (consumer).

## Substring
We initially already had a single threaded "substring finder". 
Making it multi-threaded was just a matter of creating the specified max number of threads and sending them off to search the file.

Finding the starting and stopping point of each thread was just dividing the size of the file by the number of threads we had available (we assumed that it would be even divisions).
I passed in the starting point as void \* data in the pthread and the stopping point was the starting point plus the divisions required by each string. (Divisions had to be a global variable for this to work).

The only problem I had was the occasional discrepancy; likely because the threads were swapping in and out and simply didn't finish counting a substring.

This was solved by mutexing the counter in the substring function so the threads wouldn't override each other's count.

### Elapsed Time
To measure the performance increase in using multi-threading, the time the process took to find each substring was measured and recorded.
Time Elapsed was measured by taking the time of day at the beginning of the search and taking the time again once the search was complete.
The difference was a *rough* estimate.

> Check the [Report.pdf](https://github.com/MyNameIsNickU/cse3320/blob/main/src/threading/substring/Report.pdf) in the threading folder for actual data measurements.

## Consumer/Producer
Semaphores were primary tool (and trouble) for this part.

Two semaphores were intialized, one for the `printedQueue` (for holding the Producer - adding chars to queue) and the `waitQueue` (for holding the Consumer - printing to terminal).

When the Consumer "consumes" a char, the `printedQueue` increments and it will wait until `waitQueue` is positive and decrement it. 
Likewise, when the Producer adds a char to the queue, `waitQueue` increments and it will wait until `printedQueue` is positive and decrement that.

I intialized the `printedQueue` to 1, so the Producer would think a char had been printed and it is okay to start producing more chars.
Meanwhile, the Consumer is waiting until there is a char in the Queue, or rather, the waitQueue is some value above 0.

I did have one strange, rare occurance I couldn't really recreate or figure out why it was happening. Occasionally, the program would initially hang and never recover.
It seems strange because neither of the semaphores should be allowing anything like this to happen; it seemed to be a lucky timing occurance, but I didn't have the time to really break it apart and go into it.
Debugging these things was near impossible with GDB; you can't really test the randomness of threads.

---

# Assignment #3 - Heap

# cse3320
Code hub/backup for my Operating Systems class.
4 Assignments done over the course of the semester are here.
These are some general notes and descriptions of the more interesting things used in the assignment.

# Assignment #1 - Shell
Simple shell that performs a few predetermined commands like 'history' and 'listpids' that show previous commands and their process id's respectively.
All other command inputs are passed into exec() and treated like normal unix commands.

## 'history'
I store the raw input (before tokenizing the arguements and such) into an array; index is tracked with a seperate integer.
There is a max command limit, so a circular queue is used for storing and printing out each previous command.
I use a bool integer to signal when the max limit has been reached so I can specifically handle those instances when printing or replacing a command in the history.
### BANG
![index] is supported by pulling the specified entry from the history array and putting that raw string as the "input" to the rest of the code; it gets tokenized and executes normally like the user typed it in. Storing the raw string instead of the tokenized string makes this super easy.

## 'listpids'
When the command entered is a normal UNIX command and not something special in the shell (like 'history'), I use exec() to attempt the command.
The process forks; the child uses exec() and the parent - the shell - waits for the child to finish.
When the process forks, I also store the new PID into an array for 'listpids'.
Whent the max limit for 'listpids' is reached, unlike the history command, I shift the array over left one and overwrite the right-most spot (I didn't think about circular queues until this was already done).

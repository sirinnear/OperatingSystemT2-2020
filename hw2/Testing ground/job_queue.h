#ifndef CQUEUE_H
#define CQUEUE_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

struct node_struct {
    char *data;
    struct node_struct *next;
};

typedef struct node_struct Node;

struct queue_struct {
    Node *head, *tail;
};

typedef struct queue_struct Queue;

typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
  struct job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  pid_t pgid;                 /* process group ID */
  // char notified;              /* true if user told about stopped job */
  // int stdin, stdout, stderr;  /* standard i/o channels */
} job;

job * find_job (pid_t pgid);
int job_is_stopped (job *j);
int job_is_completed (job *j);

/* Push a word to the back of this queue
 * You must keep a *COPY* of the word.
 * If q is NULL, allocate space for it here
 */
void push(Queue **q, char *word);

/* Returns the data at the front of the queue
 * and remove it from the queue as well.
 * If q is empty, return NULL
 */
char *pop(Queue *q);

/* Checks if the queue is empty */
int isEmpty(Queue *q);

/* Prints out the current queue, front to back.
 * If the queue is empty, prints out "No items"
 */
void print(Queue *q);

/* Deallocates all items in the queue */
void delete(Queue *q);

#endif

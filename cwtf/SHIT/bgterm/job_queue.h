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
  int argc;
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
  int id;                     /* what id*/
  struct job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  pid_t pgid;                 /* process group ID */
} job;

int get_job_id_by_pid(int pid);
job* get_job_by_id(int id) ;
int get_pgid_by_job_id(int id);
int get_next_job_id();
int print_processes_of_job(int id);
int print_job_status(int id);
int release_job(int id);
int insert_job(job *job);
int remove_job(int id);
int is_job_completed(int id);
int set_process_status(int pid, int status);
int set_job_status(int id, int status);
int wait_for_pid(int pid);
int wait_for_job(int id);
job* cmd_to_job(char* input);






#endif

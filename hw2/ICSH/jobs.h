#ifndef CQUEUE_H
#define CQUEUE_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

/* Contain necessary information of a process */
typedef struct process
{
  int* argc;
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  int status;                 /* reported status value */
} process;

/* Job will contain process(es) for execution  */
typedef struct job
{
  int id;                     /* what id*/
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  pid_t pgid;                 /* process group ID */
} job;

/* Basic getters for job structure*/
int get_job_id_by_pid(int pid);
job* get_job_by_id(int id) ; 
int get_pgid_by_job_id(int id);
int get_next_job_id();
int release_job(int id);

/* This function return the job_id and insert job to jobs array*/
int insert_job(job *job);
int remove_job(int id);

/* These functions work similarly but taking different arguments, setting the status of the process*/
int set_process_status(int pid, int status); 
int set_job_status(int id, int status);

int print_processes_of_job(int id);
int print_job_status(int id);

/* This function return true if the status means done*/
int is_job_completed(int id);
int wait_for_pid(int pid);

/* This function return exit status if the child process status is collected*/
int wait_for_job(int id);

/* This function will remove the defunct process and retrieve the exit status number*/
void check_zombie();

/* Execute a job, will execute the process that is contained within the job*/
int execute_job(job* exjob);
int execute_process(job* container, process* exproc, int ro, int ri, int isbg);
job* cmd_to_job(char* input, int* counter);




#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctype.h>
#include"job_queue.h"
#include <errno.h>
#include <fcntl.h>

#define JOBSIZE 20
job *jobs[20];
int *exitNo;
char** tokens;
int latestjob;
const char* STATUS_STRING[] = {
    "running",
    "done",
    "suspended",
    "continued",
    "terminated"
};

char *inputString(FILE* fp, size_t size){
//The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){ //take in stdin eof = \0
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str) return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

char** stringTokenizer(char* string, int* k){
//This is where tokens come from
    // printf("TOKENIZINzG\n");

    tokens = realloc(tokens, sizeof(char*));
    if(!string)return tokens;
    // printf("TOKENIZINzG\n");
    int slen = strlen(string);
    int begin = 0;
    int argv = 1;
    int dsize = 10;
    int toki = 0;
    int len = 0;
    int isMore = 0;
    while(isspace(string[begin])){ begin++; };
    // printf("start at %d\n",begin);
    tokens[toki] = realloc(NULL, sizeof(char)*dsize);
    for (int i = begin; i < slen; i++)
    {
        if(len==dsize){
            // printf("GET BIGGER %d\n",i);
            tokens[toki] = realloc(tokens[toki], sizeof(char)*(dsize+=16));
        }
        if(isspace(string[i])){
            // printf("Found whitespace at %d\n",i);
            isMore = 1;
            tokens[toki][len] = '\0';
            // printf("%s\n",tokens[toki]);
            
        }else{
            if(isMore){
                argv++;
                toki++;
                tokens = realloc(tokens, sizeof(char*)*(argv));
                tokens[toki] = realloc(NULL, sizeof(char)*dsize);
                len = 0;
                isMore = 0;
            }
            tokens[toki][len++] = string[i];
        }
        // printf("%c\n",string[i]);
    }
    tokens[toki][len] = '\0';
    tokens = realloc(tokens, sizeof(char*)*(argv+1));
    tokens[argv] = NULL;
    *k = argv;
    return tokens;
}

process *initpro(char* commands, int* argcount){

    process *newprocess = malloc(sizeof(process));
    // printf("WAIT PROC? \n"); 

    // int* argcount = realloc(NULL,sizeof(int));

    newprocess->argv = stringTokenizer(commands,argcount);
    // printf("MADE PROC HAHA \n"); 
    newprocess->argc = argcount;
    newprocess->pid = -1;
    newprocess->status = 0;
    return newprocess;
}

int get_job_id_by_pid(int pid) {
    int i;
    process *proc;

    for (i = 1; i <= JOBSIZE; i++) {
        if (jobs[i] != NULL) {
            proc = jobs[i]->first_process;
                if (proc->pid == pid) {
                    return i;
                }
            }
        }

    return -1;
}

job* get_job_by_id(int id) {
    if (id > JOBSIZE) {
        return NULL;
    }

    return jobs[id];
}

int get_pgid_by_job_id(int id) {
    job *job = get_job_by_id(id);

    if (job == NULL) {
        return -1;
    }

    return job->pgid;
}

int get_next_job_id() {
    int i;

    for (i = 1; i <= JOBSIZE; i++) {
        if (jobs[i] == NULL) {
            return i;
        }
    }

    return -1;
}

int print_processes_of_job(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }
    printf("[%d]", id);

    process *proc;
    proc = jobs[id]->first_process;        
    printf(" %d\n", proc->pid);
    return 0;
}

int print_job_status(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }

    printf("[%d]", id);

    process *proc;
    proc = jobs[id]->first_process;
    printf("\t%d\t%s\t%s\n", proc->pid,
            STATUS_STRING[proc->status], jobs[id]->command);
    return 0;
}

int release_job(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }
    job *job = jobs[id];
    process *proc;
    proc = job->first_process;
    free(proc);
    free(job->command);
    free(job);
    return 0;
}

int insert_job(job *job) {
    int id = get_next_job_id();

    if (id < 0) {
        return -1;
    }
    job->id = id;
    jobs[id] = job;
    return id;
}

int remove_job(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }

    release_job(id);
    jobs[id] = NULL;

    return 0;
}

int is_job_completed(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return 0;
    }

    process *proc;
    proc = jobs[id]->first_process;
    if (proc->status != 1) {
        return 0;
    }

    return 1;
}

int set_process_status(int pid, int status) {
    int i;
    process *proc;
    for (i = 1; i <= JOBSIZE; i++) {
        if (jobs[i] == NULL) {
            continue;
        }
        proc = jobs[i]->first_process; 
        if (proc->pid == pid) {
            proc->status = status;
            return 0;
        }
        
    }
    return -1;
}

int set_job_status(int id, int status) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }
    process *proc;

    proc = jobs[id]->first_process;
    if (proc->status != 1) {
        proc->status = status;
    }
    return 0;
}

int wait_for_pid(int pid) {
    int status = 0;

    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_process_status(pid, 1);
    } else if (WIFSIGNALED(status)) {
        set_process_status(pid, 4);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_process_status(pid, 2);
    }
    if(status==32512) status = 127;

    return status;
}

int wait_for_job(int id) {
    if (id > JOBSIZE || jobs[id] == NULL) {
        return -1;
    }

    int wait_pid = -1;
    int status = 0;
    wait_pid = waitpid(jobs[id]->pgid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_process_status(wait_pid, 1);
    } else if (WIFSIGNALED(status)) {
        set_process_status(wait_pid, 4);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_process_status(wait_pid, 2);
        print_job_status(id);
    }
    if(status==32512) status = 127;
    return status;
}

int exitStatus(char** cmdarr, int n, int* ex){
    if(strcmp(cmdarr[0],"echo")==0){
        if(n>=2){
            if(strcmp(cmdarr[1],"$?")==0){
                printf("%d\n", *ex);
                for (int i = 0; i < n; i++){ free(cmdarr[i]);}
                free(cmdarr);
                return 0;
        } 
        }
    }
    return 1;
}
// ./idk > know > no
int outputRed(char** cmdarr, int n){
    if(n<2) return 0;
    // printf("size of %d\n", n);

    for (int i = 0; i < n; i++)
    {
        if(strcmp(cmdarr[i],">")==0){
            // printf("let's redirect output at %d\n", i);
            return i;
        }
    }
    return 0;
}
//input occur before output
int inputRed(char** cmdarr, int n){
    if(n<2) return 0;
    int i;
    for (i = 0; i < n; i++)
    {
        if(strcmp(cmdarr[i],"<")==0){
            // printf("let's redirect input at %d\n", i);
            return i;
        }
    }
    return 0;
}
int ampersandBG(char** cmdarr, int n){
    if(n>=2){
        if(strcmp(cmdarr[n-1],"&")==0){
            // printf("running in background %d\n", n);
            cmdarr[n-1] = NULL;  
            return 0;
        } 
    }
    return 1;
}
int jobs_cmd(char** cmdarr, int n){
    if(n>0){
        if(strcmp(cmdarr[0],"jobs")==0){
            for (int i = 0; i < JOBSIZE; i++)
                if(jobs[i]){
                    remove_job(latestjob); 
                    print_job_status(i);
            }
            return 0;

        }
    }  
    
    return 1;
}
int bg_cmd(char** cmdarr, int n){
    if(n>1){
        if(strcmp(cmdarr[0],"bg")==0){
            pid_t pid;
            int job_id = -1;

            if (cmdarr[1][0] == '%') {
                job_id = atoi(cmdarr[1] + 1);
                pid = get_pgid_by_job_id(job_id);
                if (pid < 0) {
                    printf("ICSH: bg %s: no such job\n", cmdarr[1]);
                    return -1;
                }
            } else {
                pid = atoi(cmdarr[1]);
            }

            if (kill(pid, SIGCONT) < 0) {
                printf("ICSH: bg %d: job not found\n", pid);
                return -1;
            }

            if (job_id > 0) {
                set_job_status(job_id, 3);
                print_job_status(job_id);
            }

            return 0;
        }  
    }
    return 1;
    
}
int fg_cmd(char** cmdarr, int n){
    if(n>1){
        if(strcmp(cmdarr[0],"fg")==0){
            pid_t pid;
            int job_id = -1;

            if (cmdarr[1][0] == '%') {
                job_id = atoi(cmdarr[1] + 1); 
                pid = get_pgid_by_job_id(job_id);
                if (pid < 0) {
                    printf("ICSH: fg %s: no such job\n", cmdarr[1]);
                    return -1;
                }
            } else {
                pid = atoi(cmdarr[1]);
            }

            if (kill(pid, SIGCONT) < 0) {
                printf("ICSH: fg %d: job not found\n", pid);
                return -1;
            }

            tcsetpgrp(0, pid);

            if (job_id > 0) {
                //printf("BACK TO WORK!");
                set_job_status(job_id, 3);
                print_job_status(job_id);
                if (wait_for_job(job_id) >= 0) {
                    remove_job(job_id);
                }
            } else {
                wait_for_pid(pid);
            }

            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            signal(SIGTTOU, SIG_DFL);

            return 0;
        }  
    }
    return 1;
    
}
int exit_cmd(char** givencmd, int n){
    if(n!=0){
        if(strcmp(givencmd[0],"exit")==0){
            return 0;
        }  
    }
    return 1;
}

job* cmd_to_job(char* input, int* counter){

    job * newjob = realloc(NULL,sizeof(job));
    newjob->command = input;
    newjob->first_process = initpro(input, counter);
    newjob->id = 0;
    newjob->pgid = -1;

    return newjob;
}

void check_zombie() {
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
        if (WIFEXITED(status)) {
            set_process_status(pid, 1);
            *exitNo = WIFEXITED(status);
        } else if (WIFSTOPPED(status)) {
            set_process_status(pid, 4);
            *exitNo = WIFSTOPPED(status);
        } else if (WIFCONTINUED(status)) {
            set_process_status(pid, 3);
            *exitNo = WIFCONTINUED(status);
        }

        int job_id = get_job_id_by_pid(pid);
        if (job_id > 0 && is_job_completed(job_id)) {
            print_job_status(job_id);
            // printf("ZOMBIE!!!!!!!!!!! KILLLLLLLLLLL!");
            remove_job(job_id);
        }
    }
}

int shellCommands(char**givencmd, int n){
    
    if(exit_cmd(givencmd, n)==0){
        // printf("NOO1O\n");
        exit(0);
    } 
    if(exitStatus(givencmd, n, exitNo)==0){
        // printf("NOO2O\n");
        return 0;
    } 
    if(jobs_cmd(givencmd,n)==0){
        // printf("NOO3O\n");
        return 0;
    }
    if(fg_cmd(givencmd,n)==0){
        // printf("NOO4O\n");
        return 0;
    }
    if(bg_cmd(givencmd,n)==0){
        // printf("NOO5O\n");
        return 0;
    }
    return 1;
}

int execute_process(job* container, process* exproc, int ro, int ri, int isbg){
    int *c = exproc->argc;
    int n = *c;
    char *cmd = container->command;
    char **givencmd = exproc->argv;
    char **redicmd;
    int redirecto = ro;    
    int redirecti = ri;
    int bg = isbg;
    // printf("EXECUTING COMMAND %d\n", getpid());

    // if(shellCommands(givencmd, n)==0) return 0;
    //execute normally
    exproc->status = 0;
    int status=0;
    pid_t childpid = fork();

    if (childpid < -1) {
            printf("Error, cannot fork\n");
    } else if (childpid == 0) {
            // printf("[C] I am the child\n");

            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);

            exproc->pid = getpid();
            if (container->pgid > 0) {
                setpgid(0, container->pgid);
            } else {
                container->pgid = exproc->pid;
                setpgid(0, container->pgid);
            }
            // printf("My pgid is %d, my pid is %d\n", getpgid(getpid()), getpid());

            if((n-redirecti)>1 && redirecti>0){
                redicmd = malloc(sizeof(char*)*(redirecti+1));
                    for (int i = 0; i < redirecti; i++){ 
                        redicmd[i] = givencmd[i];
                    }
                redicmd[redirecti] = NULL;
                
                int in;
                size_t got;
                char buffer[1024];

                in = open(givencmd[redirecti+1], O_RDONLY);

                if ((in <= 0))
                {
                    fprintf (stderr, "Couldn't open a file\n");
                    exit (errno);
                }
                dup2 (in, 0);
                close (in);
                execvp(redicmd[0], redicmd);
                while (1)
                {
                    got = fread (buffer, 1, 1024, stdin);  
                    if (got <=0) break;
                }
                
            }else if((n-redirecto)>1 && redirecto>0){
                //think of <> later
                int out;
                out = open (givencmd[redirecto+1], O_TRUNC | O_CREAT | O_WRONLY, 0666);
                if ((out <= 0))
                {
                    fprintf (stderr, "Couldn't open a file\n");
                    exit (errno);
                }
                redicmd = malloc(sizeof(char*)*(redirecto+1));
                for (int i = 0; i < redirecto; i++){ 
                    redicmd[i] = givencmd[i];
                }
                redicmd[redirecto] = NULL;
                dup2 (out, 1);   
                close (out);
                if(fork()==0)
                execvp(redicmd[0], redicmd);
                
            }else{
                // printf("NO REDIRECT\n");

                execvp(givencmd[0], givencmd); 
            }
            
            printf("ICSH: command not found: %s\n", cmd);
            exit(127);
    } else {
            exproc->pid = childpid;
            if (container->pgid > 0) {
                setpgid(childpid, container->pgid);
            } else {
                container->pgid = exproc->pid;
                setpgid(childpid, container->pgid);
            }
            // printf("[P] I'm waiting for my child\n");
            // printf("My pgid is %d, my pid is %d\n", getpgid(getpid()), getpid());

            if (bg == 1) {
                //printf("NO AMPERSAND")
                tcsetpgrp(0, container->pgid);
                status = wait_for_job(container->id);
                signal(SIGTTOU, SIG_IGN);
                tcsetpgrp(0, getpid());
                signal(SIGTTOU, SIG_DFL);
            }
    }
    // printf("status exit %d", status);
    return status;
}
//mkae job run process bg or fg
int execute_job(job* exjob){
    process* p = exjob->first_process;
    int *c = p->argc;
    int n = *c;
    char **givencmd = exjob->first_process->argv;
    int redirecto = outputRed(givencmd,n);    
    int redirecti = inputRed(givencmd,n);
    int bg = ampersandBG(givencmd, n);
    // printf("check zombies\n");
    check_zombie();
    int job_id;
    int status = 0;

    job_id = insert_job(exjob);
    latestjob = job_id;
    // print_processes_of_job(job_id);
    if(shellCommands(givencmd, n)==0){
        // printf("----------BUILT IN WORK------------");
    }else{
        status = execute_process(exjob, p, redirecto, redirecti, bg);
    }

    if(bg==1 && status>=0){
          remove_job(job_id); 
        //   printf("removed %d\n", job_id); 
    }else{
        print_processes_of_job(job_id);
    }
    return status;
}

int main(){
    exitNo = malloc(sizeof(int));
    char *input;
    for (int i = 0; i < JOBSIZE; i++) {
        jobs[i] = NULL;
    }
    int* argcount = malloc(sizeof(int));
    tokens = malloc(sizeof(char*));
    signal(SIGTSTP, SIG_IGN); //fg is paused
    signal(SIGINT, SIG_IGN); //fg is gone
    while(1){
        printf("ICSH@6081035$>");
        input = inputString(stdin, 10); //a dynamic array
        if (strlen(input) == 0){
            check_zombie();
            free(input);
            continue;
        }
        job* toexecute = cmd_to_job(input, argcount);
        *exitNo = execute_job(toexecute);
        //no cmd mean nothing
        // executeCommand(input,status, exitNo, &jobs, job_fg);
    }

    for (int i = 0; i < *argcount; i++){ free(tokens[i]);}
    free(tokens);
    free(input);
    free(argcount);
    free(exitNo);
    exit(0);
}





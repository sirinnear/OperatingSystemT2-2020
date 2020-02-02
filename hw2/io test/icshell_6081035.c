#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctype.h>
#include"job_queue.c"
#include <errno.h>
#include <fcntl.h>

// int ORed(char *argv[]){

    //  int in;
    //  int out;
    //  size_t got;
    //  char buffer[1024];

    //  in = open (argv[0], O_RDONLY);
    //  out = open (argv[1], O_TRUNC | O_CREAT | O_WRONLY, 0666);

    //  if ((in <= 0) || (out <= 0))
    //  {
    //    fprintf (stderr, "Couldn't open a file\n");
    //    exit (errno);
    //  }

    //  dup2 (in, 0);
    //  dup2 (out, 1);   

    //  close (in);
    //  close (out);

    //  while (1)
    //  {
    //    got = fread (buffer, 1, 1024, stdin);  
    //    if (got <=0) break;
    //    fwrite (buffer, got, 1, stdout);
    //    printf("SUCK MY DICK00");
    //  }
// }

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
    char** tokens = realloc(NULL, sizeof(char*));
    if(!string)return tokens;
    // printf("TOKENIZING\n");
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
int exitStatus(char** cmdarr, int* n, int* ex){
    if(strcmp(cmdarr[0],"echo")==0){
        if(*n>=2){
            if(strcmp(cmdarr[1],"$?")==0){
                printf("%d\n", *ex);
                for (int i = 0; i < *n; i++){ free(cmdarr[i]);}
                free(cmdarr);
                free(n);
                return 0;
        } 
        }
    }
    return 1;
}
// ./idk > know > no
int outputRed(char** cmdarr, int* n){
    int i;
    for (i = 0; i < *n; i++)
    {
        if(strcmp(cmdarr[i],">")==0){
            printf("let's redirect output at %d\n", i);
            return i;
        }
    }
    return 0;
}
//input occur before output
int inputRed(char** cmdarr, int* n){
    int i;
    for (i = 0; i < *n; i++)
    {
        if(strcmp(cmdarr[i],"<")==0){
            printf("let's redirect input at %d\n", i);
            return i;
        }
    }
    return 0;
}
int ampersandBG(char** cmdarr, int* n){
    if(*n>=2){
        if(strcmp(cmdarr[*n-1],"&")==0){
            printf("running in background %d\n", *n);
            cmdarr[*n-1] = NULL;  
            return 0;
        } 
    }
    return 1;
}
int jobs_cmd(char** cmdarr, int* n, job** jobs){
    if(n!=0){
        if(strcmp(cmdarr[0],"jobs")==0){
            printf("checking jobs\n");
            jobs_show(*jobs);
            return 0;
        }  
    } 
    return 1;
}
void jobs_show(job *first){
    int count = 1;
    if(first){
        for (job* j = first; j; j = j->next) printf("[%d] %d %s\n", count++, j->first_process->pid, j->command);
    }

    //When user type "jobs", we must know if there is a job or not?
    //just make a queue of processes first, make sure we know if it is available
    //we are not pipelining
    //know the first know the next
    //when empty, tell user, it is empty
    //when not empty, print it one by one
    //we know that this queue has only one head... it is not a stack
    //i think each job either can be store in job dynamic array and a queue
}
// void jobs_init(){
//     //you have to be able to start the job every time a process is created
// }
// int bg(int i){
//     //it will send signal "SIGCONT" to the first process
//     //we iterate to next job at i where i start at 1
//     //we know that each job has one running process at the surface
// }
// int fg(int i){
//     //not only send signal to "SIGCONT"
//     //fg also has tcsetpgrp(int fildes, pid_t pgid_id) which set tcsetpgrp(STDOUT_FILENO, getpid())
// }
// void suspension(){
//     //CTRL+Z now suspend the process in a manner that the parent do not need to wait
// }
// void exit(){
//     //we know that job should be emptied before leaving
//     //we send sigkill to every job, hoping that it kills the jobs
//     //then we exit with code 0
// }
void bgdone(){
    wait(NULL);
}

int executeCommand(char* cmd, int status,int* ex, job** jobs, job* jfg){
    int *n = realloc(NULL, sizeof(int));
    char **givencmd = stringTokenizer(cmd,n);
    int bg = ampersandBG(givencmd, n);
    char** redicmd;
    int redirecto = outputRed(givencmd,n);    
    int redirecti = inputRed(givencmd,n);
    //execute normally
    pid_t pid = fork();
    setpgid(pid, getppid());
    printf("My pgid is %d, my pid is %d\n", getpgid(getpid()), getpid());
    if (pid < -1) {
            printf("Error, cannot fork\n");
    } else if (pid == 0) {
            printf("[C] I am the child\n");

            //gotta work with signal
            signal(SIGTSTP, SIGTSTP);
            signal(SIGCONT, SIGCONT);
            signal(SIGINT, SIGINT);

            if(exitStatus(givencmd, n, ex)==0){
                for (int i = 0; i < *n; i++){ free(givencmd[i]);}
                free(givencmd);
                free(n);
                exit(0);
            } 
            if(jobs_cmd(givencmd,n, jobs)==0){
                for (int i = 0; i < *n; i++){ free(givencmd[i]);}
                free(givencmd);
                free(n);
                exit(0);
            }
            if((*n-redirecti)>1 && redirecti!=0){
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
                while (1)
                {
                    execvp(redicmd[0], redicmd);
                    got = fread (buffer, 1, 1024, stdin);  
                    if (got <=0) break;
                }
                
            }else if((*n-redirecto)>1 && redirecto!=0){
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
                    // printf("ICSH: command not found: %s\n", cmd);
                    // free(redicmd);
                    // for (int i = 0; i < *n; i++){ free(givencmd[i]);}
                    // free(givencmd);
                    // free(n);
                    // exit(9);
                
            }else{
                printf("NO REDIRECT\n");

                execvp(givencmd[0], givencmd); 
            }
            
            printf("ICSH: command not found: %s\n", cmd);
            for (int i = 0; i < *n; i++){ free(givencmd[i]);}
            free(givencmd);
            free(n);
            exit(9);
    } else {
            process *mychild = initpro(pid, givencmd, status);
            job_push(jobs, cmd, getpgid(getpid()), mychild);
            printf("[P] I'm waiting for my child\n");
            
            if(bg==1){
                waitpid(pid, &status, NULL);
                if (WIFEXITED(status))  {
                int exit_status = WEXITSTATUS(status);
                *ex = exit_status;         
                printf("Exit status of the child was %d\n", exit_status); 
                }
            }
            else{
                jobs_show(*jobs);
                waitpid(pid, &status, WNOHANG);
                signal(SIGCHLD, bgdone);
                if (WIFEXITED(status))  {
                int exit_status = WEXITSTATUS(status);
                *ex = exit_status;         
                printf("FROM BG: Exit status of the child was %d\n", exit_status); 
            }
        }
    }
    
    // if((*n-redirect)>1 && redirect!=0){
    //     for (int i = 0; i < redirect; i++){ 
    //         free(redicmd[i]);
    //     }
    //     free(redicmd);
    // } 

    
    // if((*n-redirecti)>1 && redirecti!=0){
    //     int in;
    //     size_t got;
    //     char buffer[1024];

    //     in = open(givencmd[redirecti+1], O_RDONLY);

    //     if ((in <= 0))
    //     {
    //         fprintf (stderr, "Couldn't open a file\n");
    //         exit (errno);
    //     }
    //     dup2 (in, 0);
    //     close (in);
    //     while (1)
    //     {
    //         got = fread (buffer, 1, 1024, stdin);  
    //         if (got <=0) break;
    //     }
    // }
    for (int i = 0; i < *n; i++){ free(givencmd[i]);}
    free(givencmd);    
    free(n);
}

int main(){
    int status; //exit?
    int *exitNo = malloc(sizeof(int));
    char *input;
    job *jobs;
    job *job_fg;
    // char *path = strdup(getenv("PATH"));
    // printf(path);
    // free(path);
    signal(SIGTSTP, SIG_IGN); //fg is paused
    signal(SIGINT, SIG_IGN); //fg is gone
    while(1){
        printf("ICSH@6081035$>");
        input = inputString(stdin, 10); //a dynamic array
        executeCommand(input,status, exitNo, &jobs, job_fg);
        free(input);
    }
    free(exitNo);
    return 0;
}





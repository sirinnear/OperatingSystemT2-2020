#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int status; 
    char cmd[256];
    while (1) {
        printf("$ ");
        scanf("%s", cmd);
        pid_t pid = fork();
        if (pid < -1) {
            printf("Error, cannot fork\n");
        } else if (pid == 0) {
            printf("[C] I am the child\n");
            execlp(cmd, cmd, NULL);
            printf("muicsh: command not found: %s\n", cmd);
        } else {
            printf("[P] I'm waiting for my child\n");
            wait(&status);
            if (WIFEXITED(status))  {
                int exit_status = WEXITSTATUS(status);         
                printf("Exit status of the child was %d\n", exit_status); 
            }
        }
    }
    return 9;
}
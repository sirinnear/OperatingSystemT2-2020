#include <stdio.h> 
#include <signal.h> 
  
// Signal Handler for SIGTSTP 
void sighandler(int sig_num) 
{ 
    // Reset handler to catch SIGTSTP next time 
    signal(SIGTSTP, sighandler); 
    printf("Cannot execute Ctrl+Z\n"); 
} 
  
int main() 
{ 
    // Set the SIGTSTP (Ctrl-Z) signal handler 
    // to sigHandler 
    signal(SIGTSTP, sighandler); 
    while(1) 
    { 
    } 
    return 0; 
} 
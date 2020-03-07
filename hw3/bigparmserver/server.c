/**
* server.c, copyright 2001 Steve Gribble
*
* The server is a single-threaded program.  First, it opens
* up a "listening socket" so that clients can connect to
* it.  Then, it enters a tight loop; in each iteration, it
* accepts a new connection from the client, reads a request,
* computes for a while, sends a response, then closes the
* connection.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include "lib/socklib.h"
#include "common.h"
#include "threadpool.c"


extern int errno;

int   setup_listen(char *socketNumber);
char *read_request(int fd);
char *process_request(char *request, int *response_length);
void  send_response(int fd, char *response, int response_length);
void dispatch_runner(void* arg);
int date;
int dummy;
int shutdown;
struct timeval start;	/* starting time */
struct timeval end;	/* ending time */
threadpool networkpool;

/**
* This program should be invoked as "./server <socketnumber> POOLSIZE NUMROW", for
* example, "./server 4342 32 100000".
*/
void server_close(int signum){
#ifdef DEBUG
    printf("SERVER IS SHUTTING DOWN!\n");
#endif
    shutdown = 1;
    // clean up allocated memory, if any
}

int main(int argc, char **argv)
{
    char buf[1000];
    volatile int  socket_listen;
    volatile int  socket_talk;
    if (argc != 4)
    {
        fprintf(stderr, "(SERVER): Invoke as  './server socknum poolsize NUMROW'\n");
        fprintf(stderr, "(SERVER): for example, './server 4434 2 100'\n");
        exit(-1);
    }
    long counter = 0;
    long time = 0;
    shutdown = 0;
    /*
    * Set up the 'listening socket'.  This establishes a network
    * IP_address:port_number that other programs can connect with.
    */
    dummy = atoi(argv[3]);
    int poolsize = atoi(argv[2]);

    /* For the sake of humanity, let's limit the tasks*/
    int limit_tasks = 25000;
    socket_listen = setup_listen(argv[1]);
    networkpool = create_threadpool(poolsize);

    // printf("The threadpool size is %d", poolsize);
    
    /*
    * Here's the main loop of our program.  Inside the loop, the
    * one thread in the server performs the following steps:
    *
    *  1) Wait on the socket for a new connection to arrive.  This
    *     is done using the "accept" library call.  The return value
    *     of "accept" is a file descriptor for a new data socket associated
    *     with the new connection.  The 'listening socket' still exists,
    *     so more connections can be made to it later.
    *
    *  2) Read a request off of the listening socket.  Requests
    *     are, by definition, REQUEST_SIZE bytes long.
    *
    *  3) Process the request.
    *
    *  4) Write a response back to the client.
    *
    *  5) Close the data socket associated with the connection
    */
   signal(SIGINT, server_close);
#ifdef DEBUG
   printf("SERVER IS UP!");
#endif
    while(1) {
        // printf("Connected");
        if(shutdown == 1) break;

        int socket_talk = saccept((int) socket_listen);  // step 1
        if (socket_talk < 0) {
            fprintf(stderr, "An error occured in the server; a connection\n");
            fprintf(stderr, "failed because of ");
            perror("");
            exit(1);
        }
#ifdef DEBUG
        printf("WAITING..\n");
        printf("%ld tasks, at time %ld s.\n", counter, time);
#endif
        dispatch(networkpool, dispatch_runner, (void*) socket_talk);

        /* Uncomment to measure tasks done and time */
        if(counter == 0) {
            gettimeofday(&start, 0);	
            printf("Start at time 0\n");
        }
        if(counter % 500 == 0 && counter > 0){
            gettimeofday(&end, 0);	
            time = end.tv_sec - start.tv_sec;
            printf("%ld tasks, at time %ld s.\n", counter, time);
            // usleep(4000);
        } 

        counter++;
        if(counter == limit_tasks) break;
    }
    // //measure the rate
    time = (end.tv_sec - start.tv_sec) * 1000.0;
    time += (end.tv_usec - start.tv_usec) / 1000.0;
    double rate = ((double) counter)*1000 / time;
    printf("The rate is, %f", rate);
    if(shutdown == 0) destroy_threadpool(networkpool);
    close(socket_listen);
    exit(0);
}

void dispatch_runner(void* arg){
    char *request = NULL;
    char *response = NULL;
    int socket_talk = (int) arg;
    request = read_request(socket_talk);  // step 2
        if (request != NULL) {
            int response_length;
            response = process_request(request, &response_length);  // step 3
            if (response != NULL) {
                send_response(socket_talk, response, response_length);  // step 4
            }
        }
    if (request != NULL)
    free(request);
    if (response != NULL)
    free(response);
    close(socket_talk);  // step 5
}

/**
* This function accepts a string of the form "5654", and opens up
* a listening socket on the port associated with that string.  In
* case of error, this function simply bonks out.
*/

int setup_listen(char *socketNumber) {
    int socket_listen;

    if ((socket_listen = slisten(socketNumber)) < 0) {
        perror("(SERVER): slisten");
        exit(1);
    }

    return socket_listen;
}

/**
* This function reads a request off of the given socket.
* This function is thread-safe.
*/

char *read_request(int fd) {
    char *request = (char *) malloc(REQUEST_SIZE*sizeof(char));
    int   ret;

    if (request == NULL) {
        fprintf(stderr, "(SERVER): out of memory!\n");
        exit(-1);
    }

    ret = correct_read(fd, request, REQUEST_SIZE);
    if (ret != REQUEST_SIZE) {
        free(request);
        request = NULL;
    }
    return request;
}

/**
* This function crunches on a request, returning a response.
* This is where all of the hard work happens.
* This function is thread-safe.
*/

#define NUM_LOOPS 0

char *process_request(char *request, int *response_length) {
    char *response = (char *) malloc(RESPONSE_SIZE*sizeof(char));
    int   i,j;

    // just do some mindless character munging here

    for (i=0; i<RESPONSE_SIZE; i++)
    response[i] = request[i%REQUEST_SIZE];

    for (j=0; j<NUM_LOOPS+dummy; j++) {
        for (i=0; i<RESPONSE_SIZE; i++) {
            char swap;

            swap = response[((i+1)%RESPONSE_SIZE)];
            response[((i+1)%RESPONSE_SIZE)] = response[i];
            response[i] = swap;
        }
    }
    *response_length = RESPONSE_SIZE;
    return response;
}

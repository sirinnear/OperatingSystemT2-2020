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
  printf("SERVER IS SHUTTING DOWN!\n");
  shutdown = 1;
//   abandon_command(networkpool);
    destroy_threadpool(networkpool);
}

int main(int argc, char **argv)
{
    char buf[1000];
    int  socket_listen;
    int  socket_talk;
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

   printf("SERVER IS UP!");
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
        // printf("WAITING..\n");
        // printf("%d\n",socket_talk);
        dispatch(networkpool, dispatch_runner, (void*) socket_talk);
        // dispatch(networkpool, dispatch_runner, (void*) socket_listen);

        /* Uncomment to measure tasks done and time */
        if(counter == 1) {
            gettimeofday(&start, 0);	
            printf("Start at time 0\n");
        }
        if(counter % 500 == 0 && counter > 0){
            gettimeofday(&end, 0);	
            time = (end.tv_usec + end.tv_sec*1000000 - (start.tv_usec + (start.tv_sec*1000000)))/1000000;
            // printf("%ld tasks.\n", counter);

            printf("%ld tasks, at time %ld s.\n", counter, time);
        } 
        // close(socket_talk);  // step 5
        // socket_listen = setup_listen(argv[1]);

        counter++;
        // if(counter > 500000) break;
    }
    // //measure the rate
    // long rate = counter / time;
    // printf("The rate is, %ld", rate);
    // clean up allocated memory, if any

    close(socket_listen);
    // exit(0);
}

void dispatch_runner(void* arg){
    char *request = NULL;
    char *response = NULL;
    int socket_talk = (int) arg;
    // int socket_talk = saccept((int) arg);  // step 1
    //     if (socket_talk < 0) {
    //         fprintf(stderr, "An error occured in the server; a connection\n");
    //         fprintf(stderr, "failed because of ");
    //         perror("");
    //         exit(1);
    //     }
    request = read_request(socket_talk);  // step 2
        if (request != NULL) {
            int response_length;
            // printf("HI USER!");
            response = process_request(request, &response_length);  // step 3
            if (response != NULL) {
                send_response(socket_talk, response, response_length);  // step 4
                // printf("Sent\n");
            }
        }
    if (request != NULL)
    free(request);
    if (response != NULL)
    free(response);
    close(socket_talk);  // step 5
            // printf("FINISHED!\n");
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

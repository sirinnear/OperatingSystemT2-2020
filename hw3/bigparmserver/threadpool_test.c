/**
 * threadpool_test.c, copyright 2001 Steve Gribble
 *
 * Just a regression test for the threadpool code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "threadpool.h"

#define TASKS 1000

extern int errno;

void dispatch_to_me(void *arg) {
  int seconds = (int) arg;

  //fprintf(stdout, "  in dispatch %d\n", seconds);
  sleep(1);
  fprintf(stdout, "  done dispatch %d\n", seconds);
}

int main(int argc, char **argv) {
  threadpool tp;

  tp = create_threadpool(20);
  if(!tp)
      return 1;

  for(int i=0; i<TASKS; i++){
      //fprintf(stdout, "**main** dispatch %d\n", i);
      dispatch(tp, dispatch_to_me, (void *) i);
      // dispatch(tp, dispatch_to_me, (void *) i);
      // dispatch(tp, dispatch_to_me, (void *) i);
  }
  
  fprintf(stdout, "**main** done dispatch\n");
  sleep(20);
  destroy_threadpool(tp);

  exit(-1);
}


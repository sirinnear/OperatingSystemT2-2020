/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need
  pthread_t *pthreads_arr; /* Number of threads */
  pthread_mutex_t pthread_locks; 	/* protects the pool data */
  int *threads_status; /* 1 means ready, 0 means busy */
	pthread_cond_t	pool_busycv;	/* synchronization in pool_queue */
	int	pool_nthreads;	/* current number of worker threads */
	int	pool_idle;	/* number of idle workers */
} _threadpool;


void *worker_thread(void *args) {
    while (1) {
        // wait for a signal
        // l
        // mark itself as busy
        // run a given function
        //
    }
}


threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  pool -> pool_nthreads = num_threads_in_pool;
  pool -> pool_idle = num_threads_in_pool;
  pool -> pthreads_arr = malloc(sizeof(pthread_t) * num_threads_in_pool);
  pool -> threads_status = malloc(sizeof(int) * num_threads_in_pool);
  pthread_mutex_init(&pool -> pthread_locks, NULL);
  pthread_cond_init(&pool->pool_busycv, NULL);
  for (int i = 0; i < num_threads_in_pool; i++)
  {
    pool -> threads_status[i] = 1;
  }

  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;

  while(pool -> pool_idle == 0){

     for (int i = 0; i < pool->pool_nthreads; i++)
      {
        if(pool->threads_status[i] == 0){
          pthread_join(pool->pthreads_arr[i], NULL);  
          pthread_mutex_lock(&pool->pthread_locks);
          pool->threads_status[i] = 1;
          pool->pool_idle++;
          pthread_mutex_unlock(&pool->pthread_locks);
      } 
      }
  }
  
  for (int i = 0; i < pool->pool_nthreads; i++)
  {

    if(pool->threads_status[i] == 1){
      pthread_create(pool->pthreads_arr+i, NULL, dispatch_to_here, (void*)arg);
      pthread_mutex_lock(&pool->pthread_locks);
      pool->threads_status[i] = 0;
      pool->pool_idle--;
      pthread_mutex_unlock(&pool->pthread_locks);

      return;
    } 
  }
  
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  for (int i = 0; i < pool -> pool_nthreads; i++)
  {
    pthread_join(pool->pthreads_arr[i], NULL);
    free(&pool -> pthreads_arr[i]);
    free(&pool -> threads_status[i]);
  }
}

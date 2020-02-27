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
  // int *threads_status; /* 1 means ready, 0 means busy */
	pthread_cond_t	pool_busycv;	/* synchronization in pool_queue */
	int	pool_nthreads;	/* current number of worker threads */
	int	pool_idle;	/* number of idle workers */
  int tasks;
  int received;
  int shutdown;
  //the task
  dispatch_fn func;
  void* args;

} _threadpool;


void *worker_thread(void *args) {
    _threadpool *pool = (_threadpool *) args;

    while (1) {
      
        pthread_mutex_lock(&pool->pthread_locks);
        // wait for a signal
        // l
        // mark itself as busy
        while(pool->tasks == 0 && pool -> shutdown == 0){
          pthread_cond_wait(&pool->pool_busycv, &pool->pthread_locks);
        }
        // run a given function
        if(pool -> shutdown == 1) break;
        pool->received = 0;
        pool->tasks--;
        pool->pool_idle++;
        pthread_mutex_unlock(&pool->pthread_locks);
        (*(pool->func))(pool->args);
        
    }
    // printf("Thread %ld down", pthread_self());
    pthread_mutex_unlock(&pool->pthread_locks);
    return NULL;
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
  // pool -> threads_status = malloc(sizeof(int) * num_threads_in_pool);
  pthread_mutex_init(&pool -> pthread_locks, NULL);
  pthread_cond_init(&pool->pool_busycv, NULL);
  pool-> func = NULL;
  pool-> args = NULL;
  pool-> tasks = 0;
  pool-> received = 0;
  pool-> shutdown = 0;
  for (int i = 0; i < num_threads_in_pool; i++)
  {
    // pool -> threads_status[i] = 1;
    pthread_create(pool->pthreads_arr+i, NULL, worker_thread, (void*)pool);
    //worker thread usage!
  }

  return (threadpool) pool;
}

void dispatch(threadpool from_me, dispatch_fn dispatch_to_here, void *arg) {
  _threadpool *pool = (_threadpool *) from_me;
  while(pool->pool_idle == 0 ){
    //don't signal yet
  }
  pthread_mutex_lock(&pool->pthread_locks);
  pool->func = dispatch_to_here;
  pool->args = arg;
  pool->tasks++;
  pool->received++;
  pthread_cond_signal(&pool->pool_busycv);
  pool->pool_idle--;

  pthread_mutex_unlock(&pool->pthread_locks);
  while(pool->received == 1);
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  pthread_mutex_lock(&pool->pthread_locks);
  pool->shutdown = 1;
  // printf("SHUTDOWN\n");
  pthread_cond_broadcast(&pool->pool_busycv);
  pthread_mutex_unlock(&pool->pthread_locks);

  for (int i = 0; i < pool -> pool_nthreads; i++)
  {

    pthread_join(pool->pthreads_arr[i], NULL);
    // free(&pool -> threads_status[i]);
  }
  // printf("DOWN1\n");

  pthread_mutex_destroy(&pool->pthread_locks);
  pthread_cond_destroy(&pool->pool_busycv);
  free(pool -> pthreads_arr);
    // printf("DOWN2\n");

  free(pool);
}


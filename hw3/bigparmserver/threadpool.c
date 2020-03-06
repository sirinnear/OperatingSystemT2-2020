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

// A trick for C programming, comment the line below to enter normal mode.
#define DEBUG 0 
// Assume we have 200 threadpool, we create a queue that will satisfy the size of it.
#define QUEUE_SIZE 200 

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need

  pthread_t *pthreads_arr;          /* Number of threads */
  
  pthread_mutex_t pthread_locks; 	  /* protects the pool data */
	pthread_cond_t	pool_busycv;	    /* synchronization in pool_queue */
  pthread_cond_t	pool_queuecv;	      /* synchronization in queue */

	int	pool_nthreads;	              /* current number of worker threads */
	int	pool_idle;	                  /* number of idle workers */
  
  worker_task *queue;                /* tasks must be queue up here */
  int tasks;                         /* number of pending tasks */
  int Q_size;
  int head;
  int tail;
  
  int shutdown;                       /* Is the pool shutting down? */

} _threadpool;

// Worker will collect their own task!
typedef struct {
    void (*function)(void *);
    void *argument;
} worker_task;

/* DONE */
void *worker_thread(void *args) {
    _threadpool *pool = (_threadpool *) args;
    worker_task task;

//This prints out only when the worker is up
#ifdef DEBUG
    printf("HI! A WORKER IS UP\n");
#endif
    while (1) {

        pthread_cond_signal(&pool->pool_busycv);
        pthread_mutex_lock(&pool->pthread_locks);
        pool->pool_idle++;

        // wait for a signal, mark itself as busy
        while(pool->tasks == 0 && pool -> shutdown == 0){
          pthread_cond_wait(&pool->pool_busycv, &pool->pthread_locks);
          pthread_cond_signal(&pool->pool_busycv);

        }
        // run a given function
        if(pool -> shutdown == 1) break;
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        
        //Make sure the head will loop within the queue
        pool->head = (pool->head + 1) % pool->Q_size;
        pool->pool_idle--;

        // pthread_cond_signal(&pool->pool_runcv);

        pthread_mutex_unlock(&pool->pthread_locks);

        (*(task.function))(task.argument);
        
    }
#ifdef DEBUG
    printf("Thread %ld down", pthread_self());
#endif

    pool->pool_nthreads--;
    pthread_mutex_unlock(&pool->pthread_locks);
    pthread_exit(NULL);
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

  pool -> pool_nthreads = 0;
  pool -> pool_idle = 0;
  pool -> pthreads_arr = malloc(sizeof(pthread_t) * num_threads_in_pool);

  pthread_mutex_init(&pool -> pthread_locks, NULL);
  pthread_cond_init(&pool->pool_busycv, NULL);
  pthread_cond_init(&pool->pool_queuecv, NULL);

  pool-> queue =  malloc(sizeof(worker_task) * QUEUE_SIZE);
  pool-> tasks = 0;
  pool-> Q_size = QUEUE_SIZE;
  pool-> head = 0;
  pool-> tail = 0;
  pool-> shutdown = 0;
#ifdef DEBUG
    printf("****THREADPOOL IS UP! SUMMONNING WORKERS****\n");
#endif
  for (int i = 0; i < num_threads_in_pool; i++)
  {
    pthread_create(pool->pthreads_arr+i, NULL, worker_thread, (void*)pool);
    pool -> pool_nthreads++;
  }

  return (threadpool) pool;
}

/* NOT DONE */

void dispatch(threadpool from_me, dispatch_fn dispatch_to_here, void *arg) {
  _threadpool *pool = (_threadpool *) from_me;
  if( pool->shutdown == 1) return;

  if(pool == NULL) return;
  pthread_mutex_lock(&pool->pthread_locks);

  while(pool->pool_idle == 0 || pool->tasks > 0){
    //don't signal yet
    if( pool->shutdown == 1) return;
    pthread_cond_wait(&pool->pool_runcv, &pool->pthread_locks);
    break;
    // printf("POOL : BUSY AF\n");
  }
  pool->func = dispatch_to_here;
  pool->args = arg;
  pool->tasks++;
  pool->received = 1;
  pthread_cond_signal(&pool->pool_busycv);
  pool->pool_idle--;

  while(pool->received == 1){
    pthread_cond_wait(&pool->pool_runcv, &pool->pthread_locks);
  }
  if( pool->shutdown == 1) return;

  pthread_mutex_unlock(&pool->pthread_locks);

}

/* DONE */

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;
  if(  pool -> shutdown == 1) return;

  pthread_mutex_lock(&pool -> pthread_locks);
  pool->shutdown = 1;
  pthread_cond_broadcast(&pool -> pool_busycv);
  pthread_mutex_unlock(&pool -> pthread_locks);

  for (int i = 0; i < pool -> pool_nthreads; i++)
  {
    pthread_join(pool -> pthreads_arr[i], NULL);
  }

  pthread_mutex_destroy(&pool->pthread_locks);
  pthread_cond_destroy(&pool->pool_busycv);
  pthread_cond_destroy(&pool->pool_queuecv);

  free(pool -> pthreads_arr);
  free(pool -> queue);
  free(pool);
}

/* DONE */

void set_pool_shutdown(threadpool tpool, int status){
  _threadpool *pool = (_threadpool *) tpool;
  pool->shutdown = status;
}

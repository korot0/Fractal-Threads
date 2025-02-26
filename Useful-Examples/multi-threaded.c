#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

#define NUM_THREADS 100

struct parameters
{
  int thread_id;
  int x;
  int y;
};

void *run_me(void *arg)
{
  int i;

  int x;
  int y;

  struct parameters *params = (struct parameters *)arg;

  x = params->x;
  y = params->y;

  int begin = params->thread_id * 10000 / NUM_THREADS;
  int end = (begin + 10000 / NUM_THREADS) - 1;

  for (i = begin; i < end; i++)
  {
    x = x + y;
  }

  return NULL;
}

int main()
{
  pthread_t tid[NUM_THREADS];
  struct parameters params[NUM_THREADS];

  int i;

  for (i = 0; i < NUM_THREADS; i++)
  {
    params[i].x = 10;
    params[i].y = 100;
    params[i].thread_id = i;

    pthread_create(&tid[i], NULL, run_me, (void *)&params[i]);
  }

  for (i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(tid[i], NULL);
  }
};

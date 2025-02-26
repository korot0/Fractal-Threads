#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

void *run_me(int x, int y)
{
  int i;

  for (i = 0; i < 10000; i++)
  {
    x = x + y;
  }

  return NULL;
}

int main()
{
  run_me(10, 100);
};

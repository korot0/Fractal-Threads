#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: ./time <program_to_run>\n");
    return 1;
  }

  struct timeval begin_time, end_time;
  gettimeofday(&begin_time, NULL);

  system(argv[1]); // Run the specified program

  gettimeofday(&end_time, NULL);
  long time_to_execute = (end_time.tv_sec * 1000000 + end_time.tv_usec) -
                         (begin_time.tv_sec * 1000000 + begin_time.tv_usec);

  printf("Execution time: %ld microseconds\n", time_to_execute);
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>

static void print_info(const char *tag)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t sec = tv.tv_sec;
  struct tm tm_info;
  localtime_r(&sec, &tm_info);

  int ms = (int)(tv.tv_usec / 1000);

  printf("[%s] PID=%ld PPID=%ld TIME=%02d : %02d : %02d : %03d\n",
         tag,
         (long)getpid(),
         (long)getppid(),
         tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms);
}

int main(void)
{
  pid_t c1 = fork();
  if (c1 < 0)
  {
    perror("fork c1");
    return 1;
  }

  if (c1 == 0)
  {
    print_info("CHILD1");
    _exit(0);
  }

  pid_t c2 = fork();
  if (c2 < 0)
  {
    perror("fork c2");
    return 1;
  }

  if (c2 == 0)
  {
    print_info("CHILD2");
    _exit(0);
  }

  // Parent only
  print_info("PARENT");
  printf("[PARENT] Running: ps -x\n");
  system("ps -x");
  waitpid(c1, NULL, 0);
  waitpid(c2, NULL, 0);

  return 0;
}

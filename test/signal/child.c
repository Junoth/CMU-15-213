#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define pid_t long int


int main() {
  pid_t pid;
  for (int i = 0; i < 3; ++i) {
    if ((pid = fork()) == 0) {
      exit(0);
    }
  }
  
  while ((pid = wait(NULL)) > 0) {
    printf("%d\n", (int)pid);
    sleep(1);
  }

  return 0;
}

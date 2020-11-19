#include <stdio.h>
#include <unistd.h>

#define pid_t long int

int main() {
  pid_t pid;
  int x = 1;

  pid = fork();
  if (pid == 0) {
    printf("child: x=%d\n", ++x);
    return 0;
  }

  printf("parent: x=%d\n", --x);
  return 0;
}

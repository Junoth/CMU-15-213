#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define pid_t long int

int main() {
  pid_t pid;

  pid = fork();
  if (pid == 0) {
    printf("Child process run\n");
  } else {
    pid = wait(NULL);
    if (pid < 0) {
      printf("Wait error");
    }
    printf("Parent process run\n");
  }

  return 0;
}

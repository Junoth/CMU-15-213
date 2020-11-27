#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
  pid_t pid;
  pid = fork();
  printf("%d - %p - %d\n", getpid(), &pid, pid);
  exit(0);
}

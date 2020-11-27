#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
  char* args[3];
  args[0] = "/bin/echo";
  args[1] = "Hi 18213!";
  args[2] = NULL;
  execve(args[0], args, NULL);
  printf("Hi 15213\n");
  exit(0);
}

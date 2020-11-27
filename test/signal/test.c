#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int flag = 1;

void sigint_handler(int sig) {
  printf("You are killing this program!\n");
  flag = 0;
}

int main() {
  if (signal(SIGINT, sigint_handler) == SIG_ERR) {
    printf("Signal error");
    exit(1);
  }

  while (flag) {

  }

  printf("Safely exit the program\n");

  return 0;
}

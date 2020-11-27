#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
  sigset_t set, oset, pset;

  sigemptyset(&set);
  // sigprocmask(SIG_BLOCK, &set, &oset);
  printf("%d\n", sigismember(&set, SIGINT));
  printf("%d\n", sigismember(&oset, SIGINT));
}

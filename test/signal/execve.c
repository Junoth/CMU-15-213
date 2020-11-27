#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
  int a = 2;
  execve("test.sh", NULL, NULL);

  // All left code will not be executed
  printf("%d", a);
  return 0;
}

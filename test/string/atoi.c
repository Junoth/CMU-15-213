#include<stdio.h>
#include<stdlib.h>

int main() {
  char* s = "%602";
  int i = atoi(s + 1);
  printf("%d", i);
  return 0;
}

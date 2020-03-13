#include<stdio.h>

int main() {
  int cnt = 0;
  for (int i = 100; i < 1000; ++i) {
    if (i % 3 == 0 || i % 4 == 0) {
      cnt++;
    }
  }

  printf("%d", cnt);
}

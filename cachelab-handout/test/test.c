#include<stdio.h>

void main() {
  int a = 0x0030b120;
  int b = 0x0034b120;
  
  // get the set number of a0 -> 9
  // printf("%d", (a >> 5) % 32);
  
  for (int i = 0; i < 64; ++i) {
    int p = b + 64 * 4 * i;
    if ((p >> 5) % 32 == 9) {
      printf("%d\n", i);
    }
  }
}

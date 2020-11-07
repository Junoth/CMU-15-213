#include <stdio.h>

void main() {
  char str[110];
  unsigned val = 0x59b997fa;
  sprintf(str, "%.8x", val);
  puts(str);
}

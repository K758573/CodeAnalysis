#include <stdio.h>
#include "aa.h"

struct x{
  int a;
  float b;
};

int main()
{
  struct x xzz = {.a=100,.b=200};
  int c = xzz.a;
  hello();
  hello();
    for (int examplex = 0; examplex < 10; ++examplex) {
    printf("%d", examplex);
  }
  printf("hello,world");
  return 0;
}
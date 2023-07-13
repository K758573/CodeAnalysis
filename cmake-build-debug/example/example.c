#include <stdio.h>
#include "aa.h"

//匿名枚举
enum{
  ea,
  eb,
  ec,
  ed,
  ee,
};
int dalss;
//结构体
struct x{
  int a;
  //属性
  float b;
};
struct y{
  int ab;
  float bc;
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
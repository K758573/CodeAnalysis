#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include "CodeAnalysis.h"

int main(int argc, const char **argv)
{
  LOG("{}", clang::tooling::runToolOnCode(std::make_unique<clang::SyntaxOnlyAction>(), "class X {};"));
  return 0;
}
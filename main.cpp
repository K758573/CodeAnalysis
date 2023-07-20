#include <QApplication>
#include "CodeAnalysis.h"
#include "mainwindow.h"

int main(int argc, const char **argv)
{
  QApplication app(argc, const_cast<char **>(argv), QApplication::ApplicationFlags);
  LOG("程序开始运行");
  const char* argvx[] = {argv[0],"_"};
  CodeAnalysis::ASTParser parser(2,argvx);
  MainWindow mw(parser);
  mw.show();
  return QApplication::exec();
}
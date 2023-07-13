#include <QApplication>
#include "CodeAnalysis.h"
#include "mainwindow.h"

int main(int argc, const char **argv)
{
  QApplication app(argc, const_cast<char **>(argv), QApplication::ApplicationFlags);
  CodeAnalysis::ASTParser parser(argc,argv);
  MainWindow mw(parser);
  mw.show();
  return QApplication::exec();
}
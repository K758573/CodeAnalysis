#include "mainwindow.h"
#include "src/Log.h"
#include <QApplication>

int main(int argc, const char **argv)
{
  QApplication app(argc, const_cast<char **>(argv), QApplication::ApplicationFlags);
  LOG("程序开始运行");
  MainWindow mw;
  mw.show();
  return QApplication::exec();
}
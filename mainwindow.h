//
// Created by Shiro on 2023/7/6.
//

#ifndef CODEANALYSIS_MAINWINDOW_H
#define CODEANALYSIS_MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow :
    public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  
  ~MainWindow() override;

private:
  Ui::MainWindow *ui;
};


#endif //CODEANALYSIS_MAINWINDOW_H

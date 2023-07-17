//
// Created by Shiro on 2023/7/10.
//

#ifndef CODEANALYSIS_FINDERWINDOW_H
#define CODEANALYSIS_FINDERWINDOW_H

#include <QWidget>
#include "SyntaxHighLighter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FinderWindow; }
QT_END_NAMESPACE

class FinderWindow :
    public QWidget
{
Q_OBJECT

public:
  explicit FinderWindow(QWidget *parent, SyntaxHighLighter &high_lighter);
  
  ~FinderWindow() override;
  
  void setSearchContent(const QString &content);

  signals:
  
  void searchWord(const QString &word);
  //风险
private:
  Ui::FinderWindow *ui;
  SyntaxHighLighter& high_lighter_;
};


#endif //CODEANALYSIS_FINDERWINDOW_H

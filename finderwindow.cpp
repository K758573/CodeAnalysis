//
// Created by Shiro on 2023/7/10.
//

// You may need to build the project (run Qt uic code generator) to get "ui_FinderWindow.h" resolved

#include "finderwindow.h"
#include "ui_FinderWindow.h"


FinderWindow::FinderWindow(QWidget *parent, SyntaxHighLighter &high_lighter) :
    QWidget(parent), ui(new Ui::FinderWindow), high_lighter_(high_lighter)
{
  ui->setupUi(this);
  connect(ui->btn_find, &QPushButton::clicked, this, [this] {
    emit searchWord(ui->edit_content->text());
  });
}

FinderWindow::~FinderWindow()
{
  delete ui;
}

void FinderWindow::setSearchContent(const QString &content)
{
  ui->edit_content->setText(content);
}

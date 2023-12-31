//
// Created by Shiro on 2023/7/6.
//

#ifndef CODEANALYSIS_MAINWINDOW_H
#define CODEANALYSIS_MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include "SyntaxHighLighter.h"
#include "finderwindow.h"
#include "CodeAnalysis.h"
#include "RiskFunctionDb.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow :
    public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(CodeAnalysis::ASTParser &parser, QWidget *parent= nullptr);
  
  ~MainWindow() override;

private slots:
  
  void onActionOpenDirClicked();
  
  void onTreeWidgetItemClicked(const QModelIndex &index);
  
  void onActionFindText();
  
  void onActionSyntaxCheck();
  
  void onActionGenerateReport();
  
  void onTabIndexChanged(int index);
  
  void onCursorPositionChanged();
  
private:
  bool recursiveTraverseDir(QDir *dir, QTreeWidgetItem *parent);
  
  [[nodiscard]] QString getAbsolutePathOfTree(const QModelIndex &index) const;
  
  int findTabOnBar(const QString &text);
  
  void readFileToCache(const QString &filepath);
  
  void updateVarTree(const QString& filepath);
  
  void updateRiskFunction(const QString& filepath);
  
  void onTreeVarItemClicked(const QModelIndex& index);
  
  void highLightAllWord(const QString &word);
  
  void highLightDecl(const clang::NamedDecl *decl);
  
  void highLightLine(const QListWidgetItem* item);
  
  void clearContains();
  
signals:
  
  void print(const QString &msg);
  void clearMessage();

private:
  Ui::MainWindow *ui;
  //根目录
  QDir root_dir_;
  QModelIndex root_index_;
  //文件缓存
  QMap<QString, QString> cache_files_;
  QVector<QString> file_list_;
  SyntaxHighLighter highLighter_;
  FinderWindow finder_window_;
  CodeAnalysis::ASTParser &parser_;
  QRect rect_;
  RiskFunctionDB db_;
  enum {
    TAB_NOT_FOUND = -1
  };
  
  void closeEvent(QCloseEvent *event) override;
};


#endif //CODEANALYSIS_MAINWINDOW_H

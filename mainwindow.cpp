//
// Created by Shiro on 2023/7/6.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "src/Log.h"
#include <QFileDialog>
#include <QSyntaxHighlighter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), finder_window_(nullptr, highLighter_)
{
  ui->setupUi(this);
  ui->tree_file_browser->clear();
  ui->tab_editor->clear();
  ui->text_message->clear();
  highLighter_.setDocument(ui->code_browser->document());
  finder_window_.setWindowFlag(Qt::WindowStaysOnTopHint);
  connect(this, &MainWindow::print, ui->text_message, &QTextBrowser::append);
  connect(ui->action_open_dir, &QAction::triggered, this, &MainWindow::onActionOpenDirClicked);
  connect(ui->action_find, &QAction::triggered, this, &MainWindow::onActionFindText);
  connect(ui->tree_file_browser, &QTreeWidget::clicked, this, &MainWindow::onTreeWidgetItemClicked);
  connect(ui->tab_editor, &QTabWidget::currentChanged, this, &MainWindow::onTabIndexChanged);
  connect(&finder_window_, &FinderWindow::searchWord, this, &MainWindow::highLightAllWord);
  connect(ui->code_browser, &QTextBrowser::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onTabIndexChanged(int index)
{
  auto path = ui->tab_editor->widget(index)->windowFilePath();
  LOG("点击tab_bar的绝对路径:{}", path.toStdString());
  ui->code_browser->setPlainText(cache_files_[path]);
}

void MainWindow::onActionOpenDirClicked()
{
  auto dir_name = QFileDialog::getExistingDirectory(nullptr, "选择项目文件夹");
  LOG("选择的文件夹是:{}", dir_name.toStdString());
  root_dir_ = QDir(dir_name);
  root_dir_.dirName();
  auto root = new QTreeWidgetItem;
  ui->tree_file_browser->addTopLevelItem(root);
  root->setText(0, root_dir_.dirName());
  LOG("遍历文件夹:");
  recursiveTraverseDir(&root_dir_, root);
  root->setExpanded(true);
  root_dir_.cdUp();
}


/**
 * 递归遍历添加文件
 * @param dir 根文件夹
 * @param parent 文件夹节点
 * @return 空文件夹没有合适时的文件返回false
 */
bool MainWindow::recursiveTraverseDir(QDir *dir, QTreeWidgetItem *parent)
{
  bool ret = false;
  dir->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
  dir->setNameFilters(QStringList() << "*.c" << "*.h" << "*.cc"
      //  << "*.lib" << "*.dll" << "*.a"
  );
  const QStringList &list = dir->entryList();
  LOG("文件夹:{}", dir->path().toStdString());
  std::for_each(list.begin(), list.end(), [](const QString &it) {
    LOG("{}", it.toStdString());
  });
  
  std::for_each(list.begin(), list.end(), [this, dir, parent, &ret](const QString &it) {
    QString path = dir->path() + "/" + it;
    QFileInfo file_info(path);
    auto child = new QTreeWidgetItem;
    child->setText(0, it);
    parent->addChild(child);
    if (file_info.isDir()) {
      QDir child_dir(path);
      if (recursiveTraverseDir(&child_dir, child)) {
        ret = true;
      } else {
        parent->removeChild(child);
        LOG("移除空文件夹:{}", child->text(0).toStdString());
        delete child;
      }
    } else {
      LOG("添加文件:{}", child->text(0).toStdString());
      ret = true;
    }
  });
  return ret;
}

void MainWindow::onActionFindText()
{
  rect_ = finder_window_.geometry();
  finder_window_.close();
  QTextCursor textCursor = ui->code_browser->textCursor();
  textCursor.select(QTextCursor::WordUnderCursor);
  const QString &string = textCursor.selectedText();
  if (!string.contains('/')) {
    finder_window_.setSearchContent(string);
  }
  finder_window_.setGeometry(rect_);
  finder_window_.show();
}

void MainWindow::onTreeWidgetItemClicked(const QModelIndex &index)
{
  LOG("单击左侧目录树:{}", index.data().toString().toStdString());
  //拼接绝对路径
  QString path = getAbsolutePathOfTree(index);
  QFileInfo file_info(path);
  if (file_info.isDir()) {
    return;
  }
  //打开，显示文件
  if (!cache_files_.contains(path)) {
    auto wgt = new QWidget;
    wgt->setWindowFilePath(path);
    ui->tab_editor->addTab(wgt, file_info.fileName());
    readFileToCache(file_info.absoluteFilePath());
  }
  int tab_idx = findTabOnBar(file_info.fileName());
  emit ui->tab_editor->currentChanged(tab_idx);
}

QString MainWindow::getAbsolutePathOfTree(const QModelIndex &index) const
{
  QString path;
  auto parent = index.parent();
  while (parent != QModelIndex()) {
    path.push_front("/" + parent.data().toString());
    parent = parent.parent();
  }
  path = root_dir_.path() + path + "/" + index.data().toString();
  return path;
}

int MainWindow::findTabOnBar(const QString &text)
{
  for (int i = 0; i < ui->tab_editor->count(); ++i) {
    if (ui->tab_editor->tabText(i) == text) {
      return i;
    }
  }
  return -1;
}

void MainWindow::readFileToCache(const QString &filepath)
{
  QFile file;
  file.setFileName(filepath);
  file.open(QIODevice::ReadOnly);
  if (file.isOpen()) {
    QTextStream qts(&file);
    cache_files_[filepath] = qts.readAll();
    file.close();
  }
}

void MainWindow::onCursorPositionChanged()
{
  QTextCursor textCursor = ui->code_browser->textCursor();
  textCursor.select(QTextCursor::WordUnderCursor);
  const QString &string = textCursor.selectedText();
  if (string.contains('/')) {
    return;
  }
  ui->code_browser->document();
  emit print("选中单词" + string + ",位置:" + QString::number(textCursor.position()));
  QList<QTextBrowser::ExtraSelection> extra_selections;
  QTextBrowser::ExtraSelection selection;
  selection.format.setBackground(QColor(Qt::lightGray));
  selection.cursor = QTextCursor(ui->code_browser->document()->find(QRegularExpression("\\b" + string + "\\b")));
  while (selection.cursor != QTextCursor()) {
    extra_selections.push_front(selection);
    selection.cursor = ui->code_browser->document()->find(string, selection.cursor);
  }
  ui->code_browser->setExtraSelections(extra_selections);
}

void MainWindow::highLightAllWord(const QString &word)
{
  emit print("搜索" + word + ",位置:");
  QList<QTextBrowser::ExtraSelection> extra_selections;
  QTextBrowser::ExtraSelection selection;
  selection.format.setBackground(QColor(Qt::cyan));
  selection.cursor = QTextCursor(ui->code_browser->document()->find(QRegularExpression(word)));
  while (selection.cursor != QTextCursor()) {
    extra_selections.push_front(selection);
    selection.cursor = ui->code_browser->document()->find(word, selection.cursor);
  }
  ui->code_browser->setExtraSelections(extra_selections);
}

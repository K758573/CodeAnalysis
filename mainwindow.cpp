//
// Created by Shiro on 2023/7/6.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "src/Log.h"
#include <QFileDialog>
#include <QSyntaxHighlighter>

const char *const FILE_PATH_NAME_KEY = "FILE_PATH_NAME_KEY";

MainWindow::MainWindow(CodeAnalysis::ASTParser &parser, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), finder_window_(nullptr, highLighter_), parser_(parser)
{
  ui->setupUi(this);
  ui->tree_file_browser->clear();
  ui->tab_editor->clear();
  ui->text_message->clear();
  ui->tree_var->clear();
  highLighter_.setDocument(ui->code_browser->document());
  finder_window_.setWindowFlag(Qt::WindowStaysOnTopHint);
  connect(this, &MainWindow::print, ui->text_message, &QTextBrowser::append);
  connect(this, &MainWindow::clearMessage, ui->text_message, &QTextBrowser::clear);
  connect(ui->action_open_dir, &QAction::triggered, this, &MainWindow::onActionOpenDirClicked);
  connect(ui->action_find, &QAction::triggered, this, &MainWindow::onActionFindText);
  connect(ui->action_syntax_check, &QAction::triggered, this, &MainWindow::onActionSyntaxCheck);
  connect(ui->tree_file_browser, &QTreeWidget::clicked, this, &MainWindow::onTreeWidgetItemClicked);
  connect(ui->tab_editor, &QTabWidget::currentChanged, this, &MainWindow::onTabIndexChanged);
  connect(ui->tab_editor, &QTabWidget::tabCloseRequested, ui->tab_editor, &QTabWidget::removeTab);
  connect(&finder_window_, &FinderWindow::searchWord, this, &MainWindow::highLightAllWord);
  connect(ui->code_browser, &QTextBrowser::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);
  connect(ui->tree_var, &QTreeWidget::clicked, this, &MainWindow::onTreeVarItemClicked);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onTabIndexChanged(int index)
{
  auto path = ui->tab_editor->widget(index)->property(FILE_PATH_NAME_KEY).toString();
  LOG("点击tab_bar的绝对路径:{}", path.toStdString());
  if (!cache_files_.contains(path)) {
    readFileToCache(path);
  }
  ui->code_browser->setPlainText(cache_files_[path]);
  ui->tab_editor->setCurrentIndex(index);
  //文本内容更改，更新函数树
  updateVarTree(path);
}

void MainWindow::onActionOpenDirClicked()
{
  //只打开一个文件夹
  ui->tree_file_browser->clear();
  file_list_.clear();
  //读取界面目录树
  auto dir_name = QFileDialog::getExistingDirectory(nullptr, "选择项目文件夹");
  LOG("选择的文件夹是:{}", dir_name.toStdString());
  root_dir_ = QDir(dir_name);
  auto root = new QTreeWidgetItem;
  ui->tree_file_browser->addTopLevelItem(root);
  root->setText(0, root_dir_.dirName());
  root->setData(0, Qt::UserRole, root_dir_.absolutePath());
  LOG("遍历文件夹:");
  recursiveTraverseDir(&root_dir_, root);
  root->setExpanded(true);
  root_dir_.cdUp();
  //初始化AST树
  std::vector<std::string> temp_file_list;
  std::transform(file_list_.begin(), file_list_.end(), std::back_inserter(temp_file_list), [](const QString &it) {
    return it.toStdString();
  });
  parser_.initASTs(temp_file_list);
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
    child->setText(0, file_info.fileName());
    child->setData(0, Qt::UserRole, file_info.absoluteFilePath());
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
      file_list_.push_back(file_info.absoluteFilePath());
      LOG("添加文件:{}", file_info.absoluteFilePath().toStdString());
      ret = true;
    }
  });
  return ret;
}

void MainWindow::onActionFindText()
{
  rect_ = finder_window_.geometry();
  finder_window_.close();
  finder_window_.setSearchContent(ui->code_browser->textCursor().selectedText());
  finder_window_.setGeometry(rect_);
  finder_window_.show();
}

void MainWindow::onTreeWidgetItemClicked(const QModelIndex &index)
{
  LOG("单击左侧目录树:{}", index.data(Qt::UserRole).toString().toStdString());
  //从data中获取绝对路径
  auto path = qvariant_cast<QString>(index.data(Qt::UserRole));
  QFileInfo file_info(path);
  if (file_info.isDir()) {
    return;
  }
  //打开，显示文件
  int tab_idx = findTabOnBar(file_info.fileName());
  if (TAB_NOT_FOUND == tab_idx) {
    auto wgt = new QWidget;
    wgt->setProperty(FILE_PATH_NAME_KEY, path);
    ui->tab_editor->addTab(wgt, file_info.fileName());
    tab_idx = findTabOnBar(file_info.fileName());
  }
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
  return TAB_NOT_FOUND;
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

void MainWindow::onActionSyntaxCheck()
{
  emit clearMessage();
  emit print("正在进行语法检查...");
  emit print(parser_.syntaxCheck().data());
  emit print("语法检查结束");
}

void MainWindow::updateVarTree(const QString &filepath)
{
  ui->tree_var->clear();
  auto map = parser_.result().extractFunctionsAndVars(filepath.toStdString());
  for (const auto &decl_decls: map) {
    const auto decl = decl_decls.first;
    auto widget_item = new QTreeWidgetItem;
    widget_item->setData(0, Qt::UserRole, QVariant::fromValue(decl));
    widget_item->setData(1, Qt::UserRole, QVariant::fromValue(decl->getKind()));
    widget_item->setText(0, QString::fromStdString(decl->getNameAsString()));
    widget_item->setText(1, QString::fromStdString(parser_.result().getTypeAsString(decl)));
    LOG("添加到根:{},类型:{}", decl_decls.first->getNameAsString(), parser_.result().getTypeAsString(decl));
    ui->tree_var->addTopLevelItem(widget_item);
    for (const auto &var: decl_decls.second) {
      LOG("变量添加叶:{},类型{}", var->getNameAsString(), parser_.result().getTypeAsString(decl));
      auto child = new QTreeWidgetItem;
      child->setData(0, Qt::UserRole, QVariant::fromValue(var));
      child->setData(1, Qt::UserRole, QVariant::fromValue(var->getKind()));
      child->setText(0, QString::fromStdString(var->getNameAsString()));
      child->setText(1, QString::fromStdString(parser_.result().getTypeAsString(var)));
      widget_item->addChild(child);
    }
  }
}

void MainWindow::onTreeVarItemClicked(const QModelIndex &index)
{
  using clang::Decl;
  using clang::VarDecl;
  using clang::FunctionDecl;
  auto item = static_cast<QTreeWidgetItem *>(index.internalPointer());
  //  QTextCursor cursor;
  auto kind = item->data(1, Qt::UserRole).value<clang::Decl::Kind>();
  switch (kind) {
    case clang::Decl::Kind::Var: {
      auto var_decl =
          clang::dyn_cast<VarDecl>(item->data(0, Qt::UserRole).value<const  clang::NamedDecl*>());
      emit print(var_decl->getNameAsString().data());
      //      var_decl->getASTContext().getSourceManager().getSpellingLoc().
    }
      break;
    case clang::Decl::Kind::Function: {
      auto func_decl =
          clang::dyn_cast<FunctionDecl>(item->data(0, Qt::UserRole).value<const  clang::NamedDecl*>());
      emit print(func_decl->getNameAsString().data());
    }
      break;
    default:
      break;
  }
  
  
}

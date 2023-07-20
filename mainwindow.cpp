//
// Created by Shiro on 2023/7/6.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "src/Log.h"
#include "ReportGenerator.h"
#include <QFileDialog>
#include <QSyntaxHighlighter>

const char *const FILE_PATH_NAME_KEY = "FILE_PATH_NAME_KEY";

MainWindow::MainWindow(CodeAnalysis::ASTParser &parser, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), finder_window_(nullptr, highLighter_), parser_(parser)
{
  ui->setupUi(this);
  LOG("构造主窗口");
  clearContains();
  highLighter_.setDocument(ui->code_browser->document());
  finder_window_.setWindowFlag(Qt::WindowStaysOnTopHint);
  LOG("连接信号和槽");
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
  connect(ui->list_risk_function, &QListWidget::itemClicked, this, &MainWindow::highLightLine);
  connect(ui->action_generate_report, &QAction::triggered, this, &MainWindow::onActionGenerateReport);
}

void MainWindow::clearContains()
{
  ui->tree_file_browser->clear();
  ui->tab_editor->clear();
  ui->text_message->clear();
  ui->tree_var->clear();
  ui->list_risk_function->clear();
  file_list_.clear();
  cache_files_.clear();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onTabIndexChanged(int index)
{
  if (ui->tab_editor->count() == 0) {
    ui->code_browser->clear();
  }
  if (index < 0) {
    return;
  }
  auto path = ui->tab_editor->widget(index)->property(FILE_PATH_NAME_KEY).toString();
  LOG("标签切换:{}", path.toStdString());
  if (!cache_files_.contains(path)) {
    readFileToCache(path);
  }
  ui->code_browser->setPlainText(cache_files_[path]);
  ui->tab_editor->setCurrentIndex(index);
  //文本内容更改，更新函数树
  updateVarTree(path);
  //更新风险函数列表
  updateRiskFunction(path);
}

void MainWindow::onActionOpenDirClicked()
{
  //读取界面目录树
  auto dir_name = QFileDialog::getExistingDirectory(nullptr, "选择项目文件夹");
  if (dir_name.isEmpty()) {
    return;
  }
  //只打开一个文件夹
  clearContains();
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

void MainWindow::onActionGenerateReport()
{
  std::vector<const clang::CallExpr *> callees;
  for (const auto &item: file_list_) {
    const auto temp = parser_.result().callees(item.toStdString());
    callees.insert(callees.end(), temp.begin(), temp.end());
  }
  ReportGenerator generator(callees, db_, parser_.result());
  bool b = generator.generate();
  LOG("文档生成状况:{}", b);
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
    LOG("点击的是文件");
    return;
  }
  //打开，显示文件
  int tab_idx = findTabOnBar(file_info.fileName());
  LOG("查找标签位置:{}", tab_idx);
  if (TAB_NOT_FOUND == tab_idx) {
    LOG("标签不存在，创建新标签");
    auto wgt = new QWidget;
    wgt->setProperty(FILE_PATH_NAME_KEY, path);
    ui->tab_editor->addTab(wgt, file_info.fileName());
    tab_idx = findTabOnBar(file_info.fileName());
  }
  emit
  ui->tab_editor->currentChanged(tab_idx);
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
    LOG("读取文件到缓存:{}", filepath.toStdString());
    file.close();
  }
}

void MainWindow::onCursorPositionChanged()
{
  QTextCursor textCursor = ui->code_browser->textCursor();
  textCursor.select(QTextCursor::WordUnderCursor);
  QTextBrowser::ExtraSelection selection;
  selection.format.setBackground(QColor(Qt::darkGreen));
  selection.cursor = textCursor;
  ui->code_browser->setExtraSelections({selection});
  ui->code_browser->ensureCursorVisible();
  //  const QString &string = textCursor.selectedText();
  //  if (string.contains('/')) {
  //    return;
  //  }
  //  ui->code_browser->document();
  //  emit print("选中单词" + string + ",位置:" + QString::number(textCursor.position()));
  //  QList<QTextBrowser::ExtraSelection> extra_selections;
  //  QTextBrowser::ExtraSelection selection;
  //  selection.format.setBackground(QColor(Qt::lightGray));
  //  selection.cursor = QTextCursor(ui->code_browser->document()->find(QRegularExpression("\\b" + string + "\\b")));
  //  while (selection.cursor != QTextCursor()) {
  //    extra_selections.push_front(selection);
  //    selection.cursor = ui->code_browser->document()->find(string, selection.cursor);
  //  }
  //  ui->code_browser->setExtraSelections(extra_selections);
}

void MainWindow::highLightAllWord(const QString &word)
{
  //  emit print("搜索" + word + ",位置:");
  QList<QTextBrowser::ExtraSelection> extra_selections;
  QTextBrowser::ExtraSelection selection;
  selection.format.setBackground(QColor(Qt::cyan));
  selection.cursor = QTextCursor(ui->code_browser->document()->find(QRegularExpression(word)));
  while (selection.cursor != QTextCursor()) {
    extra_selections.push_front(selection);
    selection.cursor = ui->code_browser->document()->find(QRegularExpression(word), selection.cursor);
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
  LOG("更新代码结构树");
  auto map = parser_.result().extractFunctionsAndVars(filepath.toStdString());
  LOG("提取函数和变量完成:{}",filepath.toStdString());
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

void MainWindow::updateRiskFunction(const QString &filepath)
{
  ui->list_risk_function->clear();
  LOG("更新风险函数列表");
  const std::vector<const clang::CallExpr *> &callees = parser_.result().callees(filepath.toStdString());
  for (const auto &callee: callees) {
    auto risk = db_.selectByName(parser_.result().getName(callee));
    if (risk.name.isEmpty()) {
      continue;
    }
    LOG("找到风险函数:{}", risk.name.toStdString());
    risk.raw = callee;
    auto line_number = parser_.result().getLineNumber(callee);
    //    qDebug() << "函数调用:" << func_name << "行号" << line_number;
    auto item = new QListWidgetItem;
    item->setText(QString("%1 \t :[%2]").arg(risk.name).arg(line_number));
    item->setData(Qt::UserRole, QVariant::fromValue(risk));
    //    item->setData(Qt::WhatsThisRole, QVariant::fromValue(risk));
    if (risk.level > 70) {
      item->setBackground(QBrush(QColor(Qt::red)));
    } else if (risk.level > 30) {
      item->setBackground(QBrush(QColor(Qt::yellow)));
    } else {
      item->setBackground(QBrush(QColor(Qt::blue)));
    }
    ui->list_risk_function->addItem(item);
    //    const clang::CallExpr *pExpr = item->data(Qt::UserRole).value<const clang::CallExpr *>();
    //    const RiskFunction &function = item->data(Qt::WhatsThisRole).value<RiskFunction>();
    //    LOG("callee:{},function:{}",(uint64_t)pExpr,function.name.toStdString());
  }
}


void MainWindow::onTreeVarItemClicked(const QModelIndex &index)
{
  auto widget_item = static_cast<QTreeWidgetItem *>(index.internalPointer());
  auto decl = widget_item->data(0, Qt::UserRole).value<const clang::NamedDecl *>();
  highLightDecl(decl);
}

void MainWindow::highLightDecl(const clang::NamedDecl *decl)
{
  auto line = (int) parser_.result().getLineNumber(decl);
  auto column = (int) parser_.result().getColumnNumber(decl);
  auto textCursor = ui->code_browser->textCursor();
  textCursor.movePosition(QTextCursor::Start);
  textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
  textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column - 1);
  textCursor.select(QTextCursor::WordUnderCursor);
  ui->code_browser->setTextCursor(textCursor);
  const QString &string = textCursor.selectedText();
  QList<QTextBrowser::ExtraSelection> extra_selections;
  QTextBrowser::ExtraSelection selection;
  selection.format.setBackground(QColor(Qt::GlobalColor::green));
  QRegularExpression reg = QRegularExpression(QString("\\b%1\\b").arg(string));
  selection.cursor = QTextCursor(ui->code_browser->document()->find(reg));
  while (selection.cursor != QTextCursor()) {
    extra_selections.push_front(selection);
    selection.cursor = ui->code_browser->document()->find(reg, selection.cursor);
  }
  ui->code_browser->setExtraSelections(extra_selections);
  ui->code_browser->ensureCursorVisible();
}

void MainWindow::highLightLine(const QListWidgetItem *item)
{
  auto risk = item->data(Qt::UserRole).value<RiskFunction>();
  const auto callee = risk.raw;
  auto line = (int) parser_.result().getLineNumber(callee);
  auto textCursor = ui->code_browser->textCursor();
  textCursor.movePosition(QTextCursor::Start);
  textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
  textCursor.select(QTextCursor::LineUnderCursor);
  ui->code_browser->setTextCursor(textCursor);
  ui->code_browser->ensureCursorVisible();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
  // 关闭其他窗口
  for (QWidget *widget: QApplication::topLevelWidgets()) {
    if (widget != this) {
      widget->close();
    }
  }
  
  // 调用父类的 closeEvent() 函数
  QMainWindow::closeEvent(event);
}
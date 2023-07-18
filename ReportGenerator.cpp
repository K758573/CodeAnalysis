//
// Created by Shiro on 2023/7/18.
//

#include "ReportGenerator.h"
#include "src/Log.h"
#include <QFileDialog>
#include <QPlainTextEdit>

const QString MD_HEAD1 = "# ";
const QString MD_HEAD2 = "## ";
const QString MD_LIST1 = "- ";
const QString MD_LIST2 = "  - ";
const QString MD_LIST3 = "    - ";
const QString CRLF = "\r\n";

ReportGenerator::ReportGenerator(std::vector<const clang::CallExpr *> &callees, RiskFunctionDB &db,
                                 CodeAnalysis::MatchHandler &result) :
    callees_(callees), db_(db), result_(result)
{
  LOG("构造报告生成器");
}

bool ReportGenerator::generate()
{
  LOG("生成报告");
  const QString &filename = QFileDialog::getSaveFileName(nullptr, "保存到", "./", "markdown *.md");
  if (filename.isNull()) {
    LOG("未选择文件");
    return false;
  }
  LOG("文件保存到:{}", filename.toStdString());
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    LOG("文件打开失败");
    return false;
  }
  QTextStream qts(&file);
  qts << MD_HEAD1 << "项目风险报告" << CRLF;
  QVector<RiskFunction> risk_high;
  QVector<RiskFunction> risk_mid;
  QVector<RiskFunction> risk_low;
  for (const auto &callee: callees_) {
    RiskFunction risk = db_.selectByName(result_.getName(callee));
    risk.raw = callee;
    if (risk.name.isEmpty()) {
      continue;
    }
    if (risk.level > 80) {
      risk_high.push_back(risk);
    } else if (risk.level > 40) {
      risk_mid.push_back(risk);
    } else {
      risk_low.push_back(risk);
    }
  }
  qts << MD_LIST1 << "高风险函数个数:" << risk_high.count() << CRLF;
  qts << MD_LIST1 << "中风险函数个数:" << risk_mid.count() << CRLF;
  qts << MD_LIST1 << "低风险函数个数:" << risk_low.count() << CRLF;
  qts << CRLF;
  auto writelist = [this,&qts](QVector<RiskFunction>& risks){
    for (const auto &risk: risks) {
    qts << MD_LIST1 << risk.name << CRLF;
    qts << MD_LIST2 << "所在位置:" << result_.getFileName(risk.raw).data() << ":" << result_.getLineNumber(risk.raw)
        << CRLF;
    qts << MD_LIST2 << "函数功能" << CRLF;
    qts << MD_LIST3 << risk.description << CRLF;
    qts << MD_LIST2 << "修改建议" << CRLF;
    qts << MD_LIST3 << risk.suggestion << CRLF;
  }
  };
  qts << MD_HEAD2 << "高风险函数列表" << CRLF;
  writelist(risk_high);
  qts << MD_HEAD2 << "中风险函数列表" << CRLF;
  writelist(risk_mid);
  qts << MD_HEAD2 << "低风险函数列表" << CRLF;
  writelist(risk_low);
  file.close();
  LOG("风险报告生成完毕");
  return true;
}

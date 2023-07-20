//
// Created by Shiro on 2023/7/10.
//

#include <QFile>
#include "SyntaxHighLighter.h"
#include "CodeAnalysis.h"

SyntaxHighLighter::SyntaxHighLighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
  HighlightingRule rule;
  //搜索使用的颜色格式
  keywordFormat.setForeground(Qt::darkRed);
  searchRule_.format = keywordFormat;
  //关键字使用的颜色格式
  keywordFormat.setForeground(Qt::darkBlue);
  keywordFormat.setFontWeight(QFont::Bold);
  //关键字表
  const QString keywordPatterns[] = {
      QStringLiteral("\\bauto\\b"), QStringLiteral("\\bdouble\\b"), QStringLiteral("\\bint\\b"),
      QStringLiteral("\\bstruct\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\belse\\b"),
      QStringLiteral("\\blong\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bcase\\b"),
      QStringLiteral("\\benum\\b"), QStringLiteral("\\bregister\\b"), QStringLiteral("\\btypedef\\b"),
      QStringLiteral("\\bchar\\b"), QStringLiteral("\\bextern\\b"), QStringLiteral("\\breturn\\b"),
      QStringLiteral("\\bunion\\b"), QStringLiteral("\\bconst\\b"), QStringLiteral("\\bfloat\\b"),
      QStringLiteral("\\bshort\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bcontinue\\b"),
      QStringLiteral("\\bfor\\b"), QStringLiteral("\\bsigned\\b"), QStringLiteral("\\bvoid\\b"),
      QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bgoto\\b"), QStringLiteral("\\bsizeof\\b"),
      QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bdo\\b"), QStringLiteral("\\bif\\b"),
      QStringLiteral("\\bwhile\\b"),
  };
  //关键字与外观的对应关系
  for (const QString &pattern: keywordPatterns) {
    rule.pattern = QRegularExpression(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
  }
  functionFormat.setFontItalic(true);
  functionFormat.setForeground(Qt::blue);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
  rule.format = functionFormat;
  highlightingRules.append(rule);
  rule.pattern = QRegularExpression(QStringLiteral("(?<=\\.)[A-Za-z0-9_]+"));
  memberAccessFormat.setForeground(QBrush(QColor(128,0,128)));
  rule.format = memberAccessFormat;
  highlightingRules.append(rule);
}

void SyntaxHighLighter::highlightBlock(const QString &text)
{
  //高亮关键字
  for (const HighlightingRule &rule: highlightingRules) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
//
// Created by Shiro on 2023/7/10.
//

#ifndef CODEANALYSIS_SYNTAXHIGHLIGHTER_H
#define CODEANALYSIS_SYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
class SyntaxHighLighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighLighter(QTextDocument *parent = nullptr);
    
protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat memberAccessFormat;
    HighlightingRule searchRule_;
    
//    QVector<QString>
};


#endif //CODEANALYSIS_SYNTAXHIGHLIGHTER_H

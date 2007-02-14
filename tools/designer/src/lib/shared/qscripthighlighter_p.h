#ifndef QSCRIPTSYNTAXHIGHLIGHTER_H
#define QSCRIPTSYNTAXHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

namespace qdesigner_internal {

class QScriptHighlighter : public QSyntaxHighlighter
{
public:
    QScriptHighlighter(QTextDocument *parent);
    virtual void highlightBlock(const QString &text);

private:
    void highlightKeyword(int currentPos, const QString &buffer);

    QTextCharFormat m_numberFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_typeFormat;
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_labelFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_preProcessorFormat;
};
}
#endif

#ifndef QTEXTGLOBAL_H
#define QTEXTGLOBAL_H

#include <qobject.h>

class QTextFormat;
class QIODevice;

#define QTextParagraphSeparator QChar(0x2029U)
#define QTextLineSeparator QChar(0x2928U)
#define QTextObjectReplacementChar QChar(0xfffcU)

namespace QText
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };

}

struct QTextDocumentConfig
{
    // ### sane defaults?
    inline QTextDocumentConfig() : indentValue(40) {}

    int indentValue;

    QString title;
};

#endif // QTEXTGLOBAL_H

#ifndef QTEXTGLOBAL_H
#define QTEXTGLOBAL_H

#include <qobject.h>

class QTextFormat;
class QIODevice;

#define QTextBeginningOfFrame QChar(0xfdd0)
#define QTextEndOfFrame QChar(0xfdd1)
#define QTextTableCellSeparator QChar(0xfdd2)


struct QTextDocumentConfig
{
    QString title;
};

#endif // QTEXTGLOBAL_H

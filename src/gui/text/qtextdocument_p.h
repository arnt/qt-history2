#ifndef QTEXTDOCUMENT_P_H
#define QTEXTDOCUMENT_P_H

#include <private/qobject_p.h>
#include <qshareddata.h>

typedef QExplicitlySharedDataPointer<QTextPieceTable> QTextPieceTablePointer;

class QTextDocumentPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QTextDocument);
    QTextPieceTablePointer pieceTable;
};


#endif

#ifndef QTEXTTABLE_P_H
#define QTEXTTABLE_P_H

#ifndef QT_H
#include "qtextformat_p.h"
#include "qtextblockiterator.h"
#include "qtextpiecetable_p.h"
#endif

class QTextTablePrivate : public QTextFramePrivate
{
    Q_DECLARE_PUBLIC(QTextTable)
public:
    QTextTablePrivate() : grid(0), nRows(0), dirty(true) {}
    ~QTextTablePrivate();

    static QTextTable *createTable(QTextDocumentPrivate *, int pos, int rows, int cols, const QTextTableFormat &tableFormat);
    void fragmentAdded(const QChar &type, uint fragment);
    void fragmentRemoved(const QChar &type, uint fragment);

    void update() const;

    QList<int> cells;
    mutable int *grid;
    mutable int nRows;
    mutable int nCols;
    mutable bool dirty;
};


#endif

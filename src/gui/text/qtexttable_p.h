#ifndef QTEXTTABLE_P_H
#define QTEXTTABLE_P_H

#ifndef QT_H
#include "qtextformat_p.h"
#include "qtextblockiterator.h"
#include "qtextpiecetable_p.h"
#endif

class QTextTablePrivate : public QTextGroupPrivate
{
    Q_DECLARE_PUBLIC(QTextTable)
public:
    QTextTablePrivate()
        : cell_idx(-1), eor_idx(-1),grid(0), nCols(0), nRows(0), dirty(true) {}
    ~QTextTablePrivate();

    QTextBlockIterator cellStart(int cursor) const;
    QTextBlockIterator cellEnd(int cursor) const;
    QTextBlockIterator rowStart(int cursor) const;
    QTextBlockIterator rowEnd(int cursor) const;
    QTextBlockIterator start() const;
    QTextBlockIterator end() const;

    int rowAt(int cursor) const;

    inline int rows() const { if (dirty) updateGrid(); return nRows; }
    inline int cols() const { if (dirty) updateGrid(); return nCols-1; }

    inline bool operator == (const QTextTablePrivate &o) {
        return (cell_idx == o.cell_idx);
    }
    inline bool operator != (const QTextTablePrivate &o) {
        return (cell_idx != o.cell_idx);
    }

    void updateGrid() const;
    inline QTextBlockIterator cellAt(int r, int c) const {
        // nCols is without the 'eor column', however in the grid we store the
        // eor cells, hence the +1
        return QTextBlockIterator(pieceTable(), grid[r*nCols + c]);
    }

    void addCell(QTextBlockIterator);
    void removeCell(QTextBlockIterator);

    int cell_idx;
    int eor_idx;
    QList<int> columnWidth;
    QList<int> columnMinWidth;
    QList<int> rowHeight;
    inline void setDirty() { dirty = true; }

private:
    inline void setCell(int r, int c, const QTextBlockIterator &block) const
        { grid[r*nCols + c] = block.n; }
    mutable int *grid;
    mutable int nCols;
    mutable int nRows;
    mutable bool dirty;
};


#endif

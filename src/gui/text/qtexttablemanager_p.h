#ifndef QTEXTTABLEMANAGER_H
#define QTEXTTABLEMANAGER_H

#ifndef QT_H
#include <qobject.h>
#include <qstylesheet.h>
#include <qhash.h>
#include "qtextpiecetable_p.h"
#include "qfragmentmap_p.h"
#endif // QT_H


class QTextTableManagerPrivate;
class QTextPieceTable;
class QTextCursor;
class QTextTableFormat;
class QTextTablePrivate;
class QTextTable;

class QTextTableManager : public QObject
{
    Q_OBJECT
public:
    QTextTableManager(QTextPieceTable *table);
    ~QTextTableManager();

    QTextTable *table(int tableIdx) const;
    QTextTable *tableAt(int) const;
    QTextTable *createTable(const QTextCursor &cursor, int rows, int cols, const QTextTableFormat &format);

private slots:
    void blockChanged(int blockPosition, bool added);
    void formatChanged(int position, int length);

private:
    QTextPieceTable *pieceTable;
    typedef QHash<int, QTextTable *> TableHash;
    TableHash tables;

#if defined(Q_DISABLE_COPY)
    QTextTableManager(const QTextTableManager &);
    QTextTableManager &operator=(const QTextTableManager &);
#endif
};



class QTextTablePrivate
{
public:
    QTextTablePrivate()
	: cell_idx(-1), eor_idx(-1),grid(0), nCols(0), nRows(0), dirty(true) {}
    ~QTextTablePrivate();

    bool isEmpty() const { return rowList.isEmpty(); }

    QTextPieceTable::BlockIterator cellStart(int cursor) const;
    QTextPieceTable::BlockIterator cellEnd(int cursor) const;
    QTextPieceTable::BlockIterator rowStart(int cursor) const;
    QTextPieceTable::BlockIterator rowEnd(int cursor) const;
    QTextPieceTable::BlockIterator start() const;
    QTextPieceTable::BlockIterator end() const;

    int rowAt(int cursor) const;

    int rows() const { if (dirty) updateGrid(); return nRows; }
    int cols() const { if (dirty) updateGrid(); return nCols; }

    bool operator == (const QTextTablePrivate &o) {
	return (cell_idx == o.cell_idx);
    }
    bool operator != (const QTextTablePrivate &o) {
	return (cell_idx != o.cell_idx);
    }

    void updateGrid() const;
    QFragmentMap<QTextBlock>::ConstIterator cellAt(int r, int c) const {
	if (dirty) updateGrid();
	// nCols is without the 'eor column', however in the grid we store the
	// eor cells, hence the +1
	return grid[r*(nCols + 1) + c];
    }

    void addCell(QTextPieceTable::BlockIterator);
    void removeCell(QTextPieceTable::BlockIterator);

    typedef QList<QTextPieceTable::BlockIterator> Row;
    typedef QList<Row> RowList;
    RowList rowList;
    int cell_idx;
    int eor_idx;
    QList<int> columnWidth;
    QList<int> columnMinWidth;
    QList<int> rowHeight;
    void setDirty() { dirty = true; }
private:
    void setCell(int r, int c, const QFragmentMap<QTextBlock>::ConstIterator block) const
	{ grid[r*nCols + c] = block; }
    mutable QFragmentMap<QTextBlock>::ConstIterator *grid;
    mutable int nCols;
    mutable int nRows;
    mutable bool dirty;
};

#endif // QTEXTTABLEMANAGER_H

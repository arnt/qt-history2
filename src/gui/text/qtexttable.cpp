#include "qtexttablemanager_p.h"
#include "qtexttable.h"
#include "qtextcursor.h"
#include <qtextformat.h>
#include <qdebug.h>
#include "qtextlist_p.h"
#include <private/qtextformat_p.h>

#define d d_func()
#define q q_func()

QTextTableCellProperties::QTextTableCellProperties()
    : r(-1), c(-1), rSpan(0), cSpan(0)
{}

QTextTableCellProperties::QTextTableCellProperties(const QTextTablePrivate *p, int row, int col)
	: r(row), c(col)
{
    // + 1, see comment in QTextTable::start() why
    s = QTextCursor(p->pieceTable, p->cellAt(r, c).key() + 1);

    QFragmentMap<QTextBlock>::ConstIterator b = p->cellAt(r, col++);
    while (b == p->cellAt(r, col))
	++col;
    // should always work as we have the line end element
    //
    // hmm, not always, at table insertion time we can temporarily
    // run out of them. use end() then, for now.
    int endPos = 0;
    if (r >= p->rows() || col >= p->cols()) {
	if (p->end().atEnd())
	    endPos = p->pieceTable->length();
	else
	    endPos = p->end().key();
    }
    else {
	endPos = p->rowList.at(r).at(col).key();
    }
    e = QTextCursor(p->pieceTable, endPos);

    QTextPieceTable::BlockIterator it(p->cellAt(r, c), p->pieceTable);
    QTextBlockFormat fmt = it.blockFormat();
    rSpan = fmt.tableCellRowSpan();
    cSpan = fmt.tableCellRowSpan();
}

QTextTableCellProperties::~QTextTableCellProperties() {}

QTextTable::QTextTable(QTextPieceTable *pt, QObject *parent)
    : QObject(*new QTextTablePrivate, parent)
{
    Q_ASSERT(pt);
    d->pieceTable = pt;
}

QTextTable::~QTextTable()
{
}


QTextTableCellProperties QTextTable::cellAt(int row, int col) const
{
    if (row < 0 || row >= d->rows() || col < 0 || col >= d->cols())
	return QTextTableCellProperties();
    return QTextTableCellProperties(d, row, col);
}


QTextTableCellProperties QTextTable::cellAt(const QTextCursor &c) const
{
    int row = d->rowAt(c.position());
    if (row != -1) {
	const QTextTablePrivate::Row &r = d->rowList.at(row);
	for (int j = r.size()-1; j >= 0; --j) {
	    QTextPieceTable::BlockIterator cell = r.at(j);
	    if (cell.key() < c.position())
		return QTextTableCellProperties(d, row, j);
	}
	Q_ASSERT(false);
    }
    return QTextTableCellProperties();
}

void QTextTable::resize(int rows, int cols)
{
    d->pieceTable->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (nCols < cols)
	insertCols(nCols, cols - nCols);
    else if (nCols > cols)
	removeCols(cols, nCols - cols);

    if (nRows < rows)
	insertRows(nRows, rows-nRows);
    else if (nRows > rows)
	removeRows(rows, nRows-rows);

    d->pieceTable->endEditBlock();
}

void QTextTable::insertRows(int pos, int num)
{
    d->pieceTable->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();
    if (pos < 0 || pos > nRows)
	pos = nRows;

    int cursorPos;
    if (pos > 0) {
	const QTextTablePrivate::Row &row = d->rowList[pos-1];
	QTextPieceTable::BlockIterator eor = row.last();
	cursorPos = eor.key()+1;
    } else {
	cursorPos = d->rowList.at(0).at(0).key();
    }

    for (int i = 0; i < num; ++i) {
	for (int j = 0; j < nCols; ++j) {
	    d->pieceTable->insertBlockSeparator(cursorPos, d->cell_idx);
	    ++cursorPos;
	}
	d->pieceTable->insertBlockSeparator(cursorPos, d->eor_idx);
	++cursorPos;
    }

    d->pieceTable->endEditBlock();
}

void QTextTable::insertCols(int pos, int num)
{
//     qDebug() << "-------- insertCols" << pos << num;
    d->pieceTable->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (pos < 0 || pos > nCols)
	pos = nCols;

    for (int i = 0; i < nRows; ++i) {
	QTextTablePrivate::Row &row = d->rowList[i];
	QTextPieceTable::BlockIterator cell = row[pos];
	int cursorPos = cell.key();
	for (int j = 0; j < num; ++j)
	    d->pieceTable->insertBlockSeparator(cursorPos+j, d->cell_idx);
    }

//     qDebug() << "-------- end insertCols" << pos << num;
    d->pieceTable->endEditBlock();
}

void QTextTable::removeRows(int pos, int num)
{
//     qDebug() << "-------- removeRows" << pos << num;
    d->pieceTable->beginEditBlock();

    int nRows = d->rows();

    if (pos < 0 || pos >= nRows)
	return;
    if (pos+num > nRows)
	num = nRows-pos;

    QTextTablePrivate::Row &row = d->rowList[pos];
    QTextPieceTable::BlockIterator bit = row[0];
    int from = bit.key();
    row = d->rowList[pos+num-1];
    bit = row.last();
    int end = bit.key();

    d->pieceTable->remove(from, end-from+1);

    d->pieceTable->endEditBlock();
//     qDebug() << "-------- end removeRows" << pos << num;
}

void QTextTable::removeCols(int pos, int num)
{
//     qDebug() << "-------- removeCols" << pos << num;
    d->pieceTable->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (pos < 0 || pos >= nCols)
	return;
    if (pos+num > nCols)
	num = nCols-pos;

    for (int i = 0; i < nRows; ++i) {
	QTextTablePrivate::Row &row = d->rowList[i];
	QTextPieceTable::BlockIterator bit = row[pos];
	int from = bit.key();
	bit = row[pos+num-1];
	++bit;
	int end = bit.key();

	d->pieceTable->remove(from, end-from);
    }

    d->pieceTable->endEditBlock();
//     qDebug() << "-------- end removeCols" << pos << num;
}

int QTextTable::rows() const
{
    return d->rows();
}

int QTextTable::cols() const
{
    return d->cols();
}

void QTextTable::setRowSpan(int row, int col, int rowspan)
{
    QTextBlockFormat modifier;
    modifier.setTableCellRowSpan(rowspan);
    d->pieceTable->setFormat(d->cellAt(row, col).key(), 1, modifier, QTextPieceTable::MergeFormat);
}

void QTextTable::setColSpan(int row, int col, int colspan)
{
    QTextBlockFormat modifier;
    modifier.setTableCellColSpan(colspan);
    d->pieceTable->setFormat(d->cellAt(row, col).key(), 1, modifier, QTextPieceTable::MergeFormat);
}

QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
    return QTextCursor(d->pieceTable, d->rowStart(c.position()).key());
}

QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
    return QTextCursor(d->pieceTable, d->rowEnd(c.position()).key());
}

QTextCursor QTextTable::start() const
{
    // + 1 as the cursor position is always to the right, and for determining the
    // block format we do blocksFind(position - 1) in the cursor.
    return QTextCursor(d->pieceTable, d->start().key() + 1);
}

QTextCursor QTextTable::end() const
{
    return QTextCursor(d->pieceTable, d->end().key());
}

void QTextTable::setFormat(const QTextTableFormat &format)
{
    if (d->isEmpty())
	return;
    int ref = cellAt(0, 0).start().blockFormat().tableFormatIndex();
    Q_ASSERT(ref != -1);

    QTextFormatReferenceChangeCommand *cmd = new QTextFormatReferenceChangeCommand(d->pieceTable, ref, format);
    cmd->redo();
    d->pieceTable->appendUndoItem(cmd);
}

QTextTableFormat QTextTable::format() const
{
    if (d->isEmpty())
	return QTextTableFormat();
    return d->pieceTable->formatCollection()->tableFormat(cellAt(0, 0).start().blockFormat().tableFormatIndex());
}


#include "qtexttablemanager_p.h"
#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qlist.h>
#include <private/qtextformat_p.h>
#include "qtextcursor.h"
#include "qtexttable.h"

#include <private/qobject_p.h>
#include <qdebug.h>

#include <stdlib.h>

QTextTableManager::QTextTableManager(QTextPieceTable *table)
    : QObject(table)
{
    pieceTable = table;

    connect(table, SIGNAL(blockChanged(int, bool)), this, SLOT(blockChanged(int, bool)));
    connect(table, SIGNAL(formatChanged(int, int)), this, SLOT(formatChanged(int, int)));
}


QTextTableManager::~QTextTableManager()
{
}

QTextTable *QTextTableManager::table(int tableIdx) const
{
    return tables.value(tableIdx);
}

QTextTable *QTextTableManager::tableAt(int docPos) const
{
    TableHash::ConstIterator it = tables.constBegin();
    int pos = -1;
    QTextTable *table = 0;
    for (; it != tables.constEnd(); ++it) {
	const QList<QTextTablePrivate::Row> &rows = (*it)->d->rowList;
	int ts = rows[0][0].key();
	if (ts < docPos && ts > pos) {
	    QTextTablePrivate::Row r = rows.last();
	    QTextPieceTable::BlockIterator tend = r.last();
	    if (tend.key() >= docPos-1) {
		pos = ts;
		table = (*it);
	    }
	}
    }
    return table;
}

QTextTable *QTextTableManager::createTable(const QTextCursor &cursor, int rows, int cols,
						  const QTextTableFormat &tableFormat)
{
    QTextFormatCollection *formats = pieceTable->formatCollection();
    int tableIdx = formats->createReferenceIndex(tableFormat);

    int pos = cursor.position();

    QTextBlockFormat fmt = cursor.blockFormat();
    fmt.setNonDeletable(true);

    pieceTable->beginUndoBlock();

    // add block after table
    fmt.setNonDeletable(true);
    int idx = pieceTable->formatCollection()->indexForFormat(fmt);
    pieceTable->insertBlockSeparator(pos, idx);

    // create table formats
    fmt.setTableFormatIndex(tableIdx);
    int cellIdx = formats->indexForFormat(fmt);
    fmt.setTableCellEndOfRow(true);
    int eorIdx = formats->indexForFormat(fmt);

    for (int i = 0; i < rows; ++i) {
	for (int j = 0; j < cols; ++j) {
	    pieceTable->insertBlockSeparator(pos, cellIdx);
	    ++pos;
	}
	pieceTable->insertBlockSeparator(pos, eorIdx);
	++pos;
    }

    pieceTable->endUndoBlock();

    return tables.value(tableIdx);
}


void QTextTableManager::blockChanged(int blockPosition, bool added)
{
    QTextPieceTable::BlockIterator blockIt = pieceTable->blocksFind(blockPosition);
    if (blockIt.atEnd())
	return;

    QTextBlockFormat fmt = blockIt.blockFormat();

    int tableIdx = fmt.tableFormatIndex();
    if (tableIdx == -1)
	return;

    QTextTable *table = tables.value(tableIdx);
    if (!table) {
	table = new QTextTable(pieceTable, this);
	tables.insert(tableIdx, table);
    }

    if (added) {
	table->d->addCell(blockIt);
    } else {
	table->d->removeCell(blockIt);
	if (table->d->isEmpty()) {
	    delete table;
	    tables.remove(tableIdx);
	}
    }
}

void QTextTableManager::formatChanged(int position, int length)
{
    QTextPieceTable::BlockIterator blockIt = pieceTable->blocksFind(position);
    if (blockIt.atEnd())
	return;

    QTextPieceTable::BlockIterator end = pieceTable->blocksFind(position + length);
    if (!end.atEnd())
	++end;

    // there's not much we can do except throwing away what we have that is
    // in the area of change and re-scan manually.

    for (; blockIt != end; ++blockIt) {
	QTextBlockFormat fmt = blockIt.blockFormat();
	int tableIdx = fmt.tableFormatIndex();
	if (tableIdx != -1) {
	    QTextTable *t = tables.value(tableIdx);
	    Q_ASSERT(t != 0);
	    t->d->setDirty();
	}
    }
}


QTextTablePrivate::~QTextTablePrivate()
{
    free(grid);
}

static bool operator<(int cursorPos, const QTextTablePrivate::Row &row)
{
    if (row.isEmpty())
	return false;
    return cursorPos < row.first().key();
}

static bool operator<(const QTextTablePrivate::Row &row, int cursorPos)
{
    if (row.isEmpty())
	return false;
    return row.last().key() < cursorPos;
}

static bool operator<(int cursorPos, const QTextPieceTable::BlockIterator &cell)
{
    if (cell.atEnd())
	return false;
    return cursorPos < cell.key();
}

static bool operator<(const QTextPieceTable::BlockIterator &cell, int cursorPos)
{
    if (cell.atEnd())
	return false;
    return cell.key() < cursorPos;
}

int QTextTablePrivate::rowAt(int cursor) const
{
    RowList::ConstIterator it = qBinaryFind(rowList.begin(), rowList.end(), cursor);
    if (it == rowList.end())
	return -1;

    return it - rowList.begin();

    /*
    // ### use binary search!
    qDebug() << "rowAt" <<cursor;
    for (int i = 0; i < rowList.count(); ++i) {
	const Row &r = rowList.at(i);
	QTextPieceTable::BlockIterator rowEnd = r.last();
	qDebug() << "    " << i << ": " << r.first().key() << "-" << r.last().key();
	if (rowEnd.key() >= cursor) {
	    QTextPieceTable::BlockIterator rowStart = r.first();
	    if (rowStart.key() <= cursor)
		return i;
	}
    }
    return -1;
    */
}

QTextPieceTable::BlockIterator QTextTablePrivate::cellStart(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	for (int j = r.size()-1; j >= 0; --j) {
	    QTextPieceTable::BlockIterator cell = r.at(j);
	    if (cell.key() < cursor)
		return cell;
	}
	Q_ASSERT(false);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextPieceTable::BlockIterator QTextTablePrivate::cellEnd(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	for (int j = 0; j < r.size(); ++j) {
	    QTextPieceTable::BlockIterator cell = r.at(j);
	    if (cell.key() >= cursor)
		return cell;
	}
	Q_ASSERT(false);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}


QTextPieceTable::BlockIterator QTextTablePrivate::rowStart(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	return r.at(0);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextPieceTable::BlockIterator QTextTablePrivate::rowEnd(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	return r.last();
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextPieceTable::BlockIterator QTextTablePrivate::start() const
{
    const Row &r = rowList.at(0);
    return r.at(0);
}

QTextPieceTable::BlockIterator QTextTablePrivate::end() const
{
    const Row &r = rowList.last();
    QTextPieceTable::BlockIterator bit = r.last();
    ++bit;
    return bit;
}

void QTextTablePrivate::updateGrid() const
{
    nRows = qMax(nRows, rowList.size());
    nCols = qMax(nCols, rowList.first().size());

 redo:
    grid = (QFragmentMap<QTextBlock>::ConstIterator *)
	   realloc(grid, sizeof(QFragmentMap<QTextBlock>::ConstIterator)*nRows*nCols);
    memset(grid, 0, sizeof(QFragmentMap<QTextBlock>::ConstIterator)*nRows*nCols);
    int maxRow = 0;
    int maxCol = 0;
    int r = 0;
    for (int i = 0; i < rowList.size(); ++i) {
	int c = 0;
	const Row &row = rowList.at(i);
	for (int j = 0; j < row.size(); ++j) {
	    QTextPieceTable::BlockIterator cell = row.at(j);
	    QTextBlockFormat fmt = cell.blockFormat();
	    int rowspan = fmt.tableCellRowSpan();
	    int colspan = fmt.tableCellColSpan();
	    if (r + rowspan > nRows) {
		grid = (QFragmentMap<QTextBlock>::ConstIterator *)realloc(grid,
			      sizeof(QFragmentMap<QTextBlock>::ConstIterator)*(r + rowspan)*nCols);
		memset(grid + (nRows*nCols), 0,
		       sizeof(QFragmentMap<QTextBlock>::ConstIterator)*(r+rowspan-nRows)*nCols);
		nRows = r + rowspan;
	    }
	    // skip already taken cells
	    while (c < nCols && grid[r*nCols + c].n)
		++c;

	    if (c + colspan <= nCols) {
		for (int ii = 0; ii < rowspan; ++ii)
		    for (int jj = 0; jj < colspan; ++jj)
			setCell(r+ii, c+jj, cell);
		maxRow = qMax(maxRow, r+rowspan);
		maxCol = qMax(maxCol, c+colspan);
	    }
	    c += colspan;
	}
	if (c > nCols) {
	    nCols = c;
	    goto redo;
	}
	++r;
    }
    nRows = maxRow;
    // nCols is without the 'eor column', hence the -1
    nCols = maxCol-1;

    dirty = false;
}

#ifndef QT_NO_DEBUG
static void checkRowList(const QList<QTextTablePrivate::Row> &rowList)
{
    int pos = 0;
    int tableIdx = -1;
    for (int i = 0; i < rowList.size(); ++i) {
	const QTextTablePrivate::Row &r = rowList.at(i);
	for (int j = 0; j < r.size(); ++j) {
	    QTextPieceTable::BlockIterator bi = r.at(j);
	    Q_ASSERT(bi.key() > pos);
	    pos = bi.key();
	    QTextBlockFormat fmt = bi.blockFormat();
	    if (tableIdx == -1)
		tableIdx = fmt.tableFormatIndex();
	    Q_ASSERT(tableIdx == fmt.tableFormatIndex());
	    if (fmt.tableCellEndOfRow())
		Q_ASSERT(j == r.size()-1);
	}
    }
}
#endif

void QTextTablePrivate::addCell(QTextPieceTable::BlockIterator it)
{
//     qDebug("addCell at %d", it.key());
    if (isEmpty()) {
	Row r;
	r.append(it);
	rowList.append(r);

	if (cell_idx == -1)
	    cell_idx = it.blockFormatIndex();

	Q_ASSERT(cell_idx == it.blockFormatIndex());
    } else {
	QTextBlockFormat fmt = it.blockFormat();

	bool eor = fmt.tableCellEndOfRow();
	const int position = it.key();

// 	qDebug() << "position" << position << "end" << end().key();

	/*
	 * This assertion can't be fullfilled anymore, when incrementally
	 * creating tables with multi-block cell content. The problem is
	 * basically that end(), which uses the block of the last entry
	 * in the last rowlist _plus_ one (->next block) . However if that
	 * last cell is (temporarily) not the EOR block but actual multi-block
	 * cell content then end returns a block pointer pointing into the
	 * second block of the last cell.
	 */
	//Q_ASSERT(position <= end().key());

	if (eor) {
	    if (eor_idx == -1)
		eor_idx = it.blockFormatIndex();
	    Q_ASSERT(eor_idx == it.blockFormatIndex());
	}

	int row = 0;
	int cell = 0;
	if (position > start().key()) {

	    RowList::ConstIterator rowIt = qLowerBound(rowList.begin(), rowList.end(), position);
	    if (rowIt == rowList.end()) {
		row = rowList.size() - 1;
		cell = rowList[row].size();
	    } else {
		// find cell
		row = rowIt - rowList.begin();
		const Row &r = *rowIt;

		Row::ConstIterator it = qUpperBound(r.begin(), r.end(), position);
		cell = it - r.begin();
	    }
	}

// 	qDebug() << "inserting at" << row << cell << "eor=" <<eor;
	Row &r = rowList[row];
	if (cell == r.size() && row == rowList.size()-1 &&
	    r.last().blockFormat().tableCellEndOfRow()) {
	    ++row;
	    cell = 0;
// 	    qDebug("creating new row at %d", row);
	    rowList.insert(row, Row());
	}
	if (eor && cell < r.size()) {
// 	    qDebug() << "splitting row";
	    // need to insert a new row
	    Row newRow;
	    while (r.size() > cell)
		newRow.append(r.takeAt(cell));
	    rowList.insert(row+1, newRow);
	}
// 	qDebug() << "  adjusted insert" << row << cell << "eor=" <<eor;
	rowList[row].insert(cell, it);
    }
#ifndef QT_NO_DEBUG
    checkRowList(rowList);
#endif
    dirty = true;
}

void QTextTablePrivate::removeCell(QTextPieceTable::BlockIterator it)
{
//     qDebug() << "removeCell" << it.key();
    QTextBlockFormat fmt = it.blockFormat();

    bool eor = fmt.tableCellEndOfRow();
    int position = it.key();

    int row = rowList.size()-1;
    while (row >= 0) {
	if (rowList.at(row).first().key() <= position)
	    break;
	--row;
    }
    Q_ASSERT(row >= 0);
    // find cell
    const Row &r = rowList.at(row);
    int cell = 0;
    for (; cell < r.size(); ++cell) {
	QTextPieceTable::BlockIterator c = r.at(cell);
	if (c.key() == position)
	    break;
    }
//     qDebug() << "row="<<row<<"cell="<<cell;
    Q_ASSERT(cell < r.size());

    if (eor) {
	Q_ASSERT(row < rowList.size());

	// need to remove a row
	Row &r = rowList[row];
	Q_ASSERT(it.key() == r.last().key());
	r.removeLast();
	if (row+1 < rowList.size()) {
	    const Row &oldRow = rowList.at(row+1);
	    for (int i = 0; i < oldRow.size(); ++i)
		r.append(oldRow.at(i));
	    rowList.removeAt(row+1);
	}
    } else {
	rowList[row].removeAt(cell);
    }

    if (rowList[row].isEmpty())
	rowList.removeAt(row);

#ifndef QT_NO_DEBUG
    checkRowList(rowList);
#endif
    dirty = true;
}

#include "qtexttablemanager_p.h"
#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qlist.h>
#include <private/qtextformat_p.h>
#include "qtextcursor.h"
#include "qtexttable.h"

#include <qdebug.h>

#include <stdlib.h>

#define d d_func()
#define q q_func()

QTextTableManager::QTextTableManager(QTextPieceTable *table)
    : QObject(table)
{
    pt = table;

    connect(table, SIGNAL(blockChanged(int,QText::ChangeOperation)), this, SLOT(blockChanged(int,QText::ChangeOperation)));
    connect(table, SIGNAL(formatChanged(int,int)), this, SLOT(formatChanged(int,int)));
}


QTextTableManager::~QTextTableManager()
{
}

QTextTable *QTextTableManager::table(QTextFormatGroup *group) const
{
    return tables.value(group);
}

QTextTable *QTextTableManager::tableAt(int docPos) const
{
    TableHash::ConstIterator it = tables.constBegin();
    int pos = -1;
    QTextTable *table = 0;
    for (; it != tables.constEnd(); ++it) {
	const QList<QTextTablePrivate::Row> &rows = (*it)->d->rowList;
	int ts = rows[0][0].key();
	if (ts <= docPos && ts > pos) {
	    QTextTablePrivate::Row r = rows.last();
	    QTextBlockIterator tend = r.last();
	    if (tend.key() >= docPos) {
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
    QTextFormatCollection *collection = pt->formatCollection();
    QTextFormatGroup *group = collection->createGroup(tableFormat);

    int pos = cursor.position();

    QTextBlockFormat fmt = cursor.blockFormat();
    fmt.setNonDeletable(true);

    pt->beginEditBlock();

    // add block after table
    fmt.setNonDeletable(true);
    int idx = pt->formatCollection()->indexForFormat(fmt);
    pt->insertBlock(pos, idx, pt->formatCollection()->indexForFormat(QTextCharFormat()));

    // create table formats
    fmt.setGroup(group);
    int cellIdx = collection->indexForFormat(fmt);
    fmt.setTableCellEndOfRow(true);
    int eorIdx = collection->indexForFormat(fmt);

    for (int i = 0; i < rows; ++i) {
	for (int j = 0; j < cols; ++j) {
	    pt->insertBlock(pos, cellIdx, pt->formatCollection()->indexForFormat(QTextCharFormat()));
	    ++pos;
	}
	pt->insertBlock(pos, eorIdx, pt->formatCollection()->indexForFormat(QTextCharFormat()));
	++pos;
    }

    pt->endEditBlock();

    return tables.value(group);
}

QVector<QTextBlockIterator> QTextTableManager::blocksForObject(QTextFormatGroup *group) const
{
    QVector<QTextBlockIterator> blocks;
    QTextTable *tab = table(group);
    if (tab) {
	QTextTablePrivate *tp = tab->d_func();
	blocks.reserve(tp->rows() * tp->cols());

	QTextBlockIterator it = tp->start();
	QTextBlockIterator end = tp->end();
	for (; !it.atEnd() && it != end; ++it)
	    blocks.append(it);
    }
    return blocks;
}

void QTextTableManager::blockChanged(int blockPosition, QText::ChangeOperation op)
{
    QTextBlockIterator blockIt = pt->blocksFind(blockPosition);
    if (blockIt.atEnd())
	return;

    QTextBlockFormat fmt = blockIt.blockFormat();
    QTextFormatGroup *group = fmt.group();
    if (!group)
	return;

    const QTextTableFormat tablefmt = group->commonFormat().toTableFormat();
    if (!tablefmt.isValid())
	return;

    QTextTable *table = tables.value(group);
    if (!table) {
	table = new QTextTable(pt, this);
	tables.insert(group, table);
    }

    if (op == QText::Insert) {
	table->d->addCell(blockIt);
    } else {
	table->d->removeCell(blockIt);
	if (table->d->isEmpty()) {
	    delete table;
	    tables.remove(group);
	}
    }
}

void QTextTableManager::formatChanged(int position, int length)
{
    QTextBlockIterator blockIt = pt->blocksFind(position);
    if (blockIt.atEnd())
	return;

    QTextBlockIterator end = pt->blocksFind(position + length);
    if (!end.atEnd())
	++end;

    // there's not much we can do except throwing away what we have that is
    // in the area of change and re-scan manually.

    for (; blockIt != end; ++blockIt) {
	QTextFormatGroup *group = blockIt.blockFormat().group();
	QTextTable *t = tables.value(group);
	if (t)
	    t->d->setDirty();
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

static bool operator<(int cursorPos, const QTextBlockIterator &cell)
{
    if (cell.atEnd())
	return false;
    return cursorPos < cell.key();
}

/* unused, currently
static bool operator<(const QTextBlockIterator &cell, int cursorPos)
{
    if (cell.atEnd())
	return false;
    return cell.key() < cursorPos;
}
*/

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
	QTextBlockIterator rowEnd = r.last();
	qDebug() << "    " << i << ": " << r.first().key() << "-" << r.last().key();
	if (rowEnd.key() >= cursor) {
	    QTextBlockIterator rowStart = r.first();
	    if (rowStart.key() <= cursor)
		return i;
	}
    }
    return -1;
    */
}

QTextBlockIterator QTextTablePrivate::cellStart(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	for (int j = r.size()-1; j >= 0; --j) {
	    QTextBlockIterator cell = r.at(j);
	    if (cell.key() < cursor)
		return cell;
	}
	Q_ASSERT(false);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextBlockIterator QTextTablePrivate::cellEnd(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	for (int j = 0; j < r.size(); ++j) {
	    QTextBlockIterator cell = r.at(j);
	    if (cell.key() >= cursor)
		return cell;
	}
	Q_ASSERT(false);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}


QTextBlockIterator QTextTablePrivate::rowStart(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	return r.at(0);
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextBlockIterator QTextTablePrivate::rowEnd(int cursor) const
{
    int row = rowAt(cursor);
    if (row != -1) {
	const Row &r = rowList.at(row);
	return r.last();
    }
    return rowList[0][0].pieceTable()->blocksEnd();
}

QTextBlockIterator QTextTablePrivate::start() const
{
    const Row &r = rowList.at(0);
    return r.at(0);
}

QTextBlockIterator QTextTablePrivate::end() const
{
    const Row &r = rowList.last();
    QTextBlockIterator bit = r.last();
    ++bit;
    return bit;
}

void QTextTablePrivate::updateGrid() const
{
    nRows = qMax(nRows, rowList.size());
    nCols = qMax(nCols, rowList.first().size());

 redo:
    grid = (int *)
	   realloc(grid, sizeof(int)*nRows*nCols);
    memset(grid, 0, sizeof(int)*nRows*nCols);
    int maxRow = 0;
    int maxCol = 0;
    int r = 0;
    for (int i = 0; i < rowList.size(); ++i) {
	int c = 0;
	const Row &row = rowList.at(i);
	for (int j = 0; j < row.size(); ++j) {
	    QTextBlockIterator cell = row.at(j);
	    QTextBlockFormat fmt = cell.blockFormat();
	    int rowspan = fmt.tableCellRowSpan();
	    int colspan = fmt.tableCellColSpan();
	    if (r + rowspan > nRows) {
		grid = (int *)realloc(grid, sizeof(int)*(r + rowspan)*nCols);
		memset(grid + (nRows*nCols), 0, sizeof(int)*(r+rowspan-nRows)*nCols);
		nRows = r + rowspan;
	    }
	    // skip already taken cells
	    while (c < nCols && grid[r*nCols + c])
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
    QTextTableFormat table;
    for (int i = 0; i < rowList.size(); ++i) {
	const QTextTablePrivate::Row &r = rowList.at(i);
	for (int j = 0; j < r.size(); ++j) {
	    QTextBlockIterator bi = r.at(j);
	    Q_ASSERT(bi.key() > pos);
	    pos = bi.key();
	    QTextBlockFormat fmt = bi.blockFormat();
	    if (table.isValid())
		table = fmt.tableFormat();
	    Q_ASSERT(table == fmt.tableFormat());
	    if (fmt.tableCellEndOfRow())
		Q_ASSERT(j == r.size()-1);
	}
    }
}
#endif

void QTextTablePrivate::addCell(QTextBlockIterator it)
{
//     qDebug("addCell at %d", it.key());
    if (isEmpty()) {
	Row r;
	r.append(it);
	rowList.append(r);

	if (cell_idx == -1)
	    cell_idx = QTextPieceTable::block(it)->format;

	Q_ASSERT(cell_idx == QTextPieceTable::block(it)->format);
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
		eor_idx = QTextPieceTable::block(it)->format;
	    Q_ASSERT(eor_idx == QTextPieceTable::block(it)->format);
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

void QTextTablePrivate::removeCell(QTextBlockIterator it)
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
	QTextBlockIterator c = r.at(cell);
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

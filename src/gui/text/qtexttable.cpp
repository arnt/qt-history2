#include "qtexttable.h"
#include "qtextcursor.h"
#include "qtextformat.h"
#include "qtextblockiterator.h"
#include <qdebug.h>
#include "qtexttable_p.h"

#include <stdlib.h>

#define d d_func()
#define q q_func()


/*!
    \class QTextTableCellProperties qtexttable.h
    \brief The properties of a table cell in a QTextDocument

    \ingroup text

    QTextTableCellProperties describes a specific table cell in a QTextTable.
    It can be queried with the QTextTable::cellAt method.
*/

/*!
  Constructs an invalid QTextTableCellProperties object.

  \sa isValid
*/
QTextTableCellProperties::QTextTableCellProperties()
    : r(-1), c(-1), rSpan(0), cSpan(0)
{}

/*! \internal
 */
QTextTableCellProperties::QTextTableCellProperties(const QTextTablePrivate *p, int row, int col)
        : r(row), c(col)
{
    QTextBlockIterator it = p->cellAt(r, c);
    QTextBlockFormat fmt = it.blockFormat();
    rSpan = fmt.tableCellRowSpan();
    cSpan = fmt.tableCellRowSpan();

    s = QTextCursor(p->pieceTable(), it.position());
    int index = p->blocks.indexOf(it);
    Q_ASSERT(index < p->blocks.size()-1);
    e = QTextCursor(p->pieceTable(), p->blocks.at(index+1).position());
}

/*!
  \fn int QTextTableCellProperties::row() const

  Returns the row this table cell describes
*/

/*!
  \fn int QTextTableCellProperties::col() const

  Returns the col this table cell describes
*/

/*!
  \fn int QTextTableCellProperties::rowSpan() const

  Returns the rowSpan of this table cell
*/

/*!
  \fn int QTextTableCellProperties::colSpan() const

  Returns the colSpan of this table cell.
*/

/*!
  \fn bool QTextTableCellProperties::isValid() const

  Returns if the table cell queried by QTextTable::cellAt describes a
  valid cell.
*/


/*!
  \fn QTextCursor QTextTableCellProperties::start() const

  Returns if a QTextCursor pointing to the start of the cell.
*/

/*!
  \fn QTextCursor QTextTableCellProperties::end() const

  Returns if a QTextCursor pointing to the end of the cell.
*/


/*!
  \fn QTextCursor QTextTableCellProperties::operator==(const QTextTableCellProperties &other) const

  Returns true if the two objects describe the same table cell
*/

/*!
  \fn QTextCursor QTextTableCellProperties::operator!=(const QTextTableCellProperties &other) const

  Returns true if the two objects do not describe the same table cell
*/

/*!
  destroys the object.
*/
QTextTableCellProperties::~QTextTableCellProperties() {}


/*!
    \class QTextTable qtexttable.h
    \brief A table in a QTextDocument

    \ingroup text

    QTextTable represents a table object in a QTextDocument. Tables
    can be created through QTextCursor::createTable and queried with
    QTextCursor::currentTable.

*/

/*! \internal
 */
QTextTable::QTextTable(QObject *parent)
    : QTextGroup(*new QTextTablePrivate, parent)
{
}

/*! \internal
 */
QTextTable::~QTextTable()
{
}


/*!
  Returns a QTextTableCellProperties object describing the properties
  of the table cell at row \a row and column \a col in the table.
*/
QTextTableCellProperties QTextTable::cellAt(int row, int col) const
{
    if (d->dirty) d->updateGrid();

    if (row < 0 || row >= d->rows() || col < 0 || col >= d->cols())
        return QTextTableCellProperties();
    return QTextTableCellProperties(d, row, col);
}


/*!
  Returns a QTextTableCellProperties object describing the properties
  of the table cell at cursor position \a c.
*/
QTextTableCellProperties QTextTable::cellAt(const QTextCursor &c) const
{
    if (d->dirty) d->updateGrid();

    int pos = c.position();
    int row = d->rowAt(pos);
    if (row != -1) {
	for (int i = d->nCols - 1; i >= 0 ; --i) {
            QTextBlockIterator cell = d->cellAt(row, i);
	    if (cell.atEnd())
		continue;
	    if ((row && d->cellAt(row-1, i) == cell)
		|| (i && d->cellAt(row, i-1) == cell))
		continue;
            if (cell.position() <= c.position())
                return QTextTableCellProperties(d, row, i);
        }
        Q_ASSERT(false);
    }
    return QTextTableCellProperties();
}

/*!
  resizes the table to \a rows rows and \a cols cols.
*/
void QTextTable::resize(int rows, int cols)
{
    d->pieceTable()->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (rows == nRows && cols == nCols)
	return;

    if (nCols < cols)
        insertCols(nCols, cols - nCols);
    else if (nCols > cols)
        removeCols(cols, nCols - cols);

    if (nRows < rows)
        insertRows(nRows, rows-nRows);
    else if (nRows > rows)
        removeRows(rows, nRows-rows);

    Q_ASSERT(d->dirty);
    d->pieceTable()->endEditBlock();
}

/*!
  inserts \a num rows before row \a pos.
*/
void QTextTable::insertRows(int pos, int num)
{
    if (num <= 0)
	return;
    if (d->dirty) d->updateGrid();

    d->pieceTable()->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    int cursorPos;
    if (pos < 0 || pos >= nRows) {
	cursorPos = d->blocks.last().position();
    } else {
        cursorPos = d->rowStart(pos).position() - 1;
    }

    for (int i = 0; i < num; ++i) {
        for (int j = 0; j < nCols; ++j) {
            d->pieceTable()->insertBlock(cursorPos, d->cell_idx, d->pieceTable()->formatCollection()->indexForFormat(QTextCharFormat()));
            ++cursorPos;
        }
        d->pieceTable()->insertBlock(cursorPos, d->eor_idx, d->pieceTable()->formatCollection()->indexForFormat(QTextCharFormat()));
        ++cursorPos;
    }

    Q_ASSERT(d->dirty);
    d->pieceTable()->endEditBlock();
}

/*!
  inserts \a num colums before colum \a pos.
*/
void QTextTable::insertCols(int pos, int num)
{
    if (num <= 0)
	return;
    if (d->dirty) d->updateGrid();

//     qDebug() << "-------- insertCols" << pos << num;
    d->pieceTable()->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (pos < 0 || pos > nCols)
        pos = nCols;

    for (int i = 0; i < nRows; ++i) {
	int col = pos;
	QTextBlockIterator cell = d->cellAt(i, col);
	// in case this is a rowspan cell we'd insert at the wrong position
	if (i && d->cellAt(i-1, col) == cell) {
	    ++col;
	    QTextBlockIterator newCell;
	    while ((newCell = d->cellAt(i, col)) == cell)
		++col;
	    cell = newCell;
	}
	// same for colspan
	if (col && d->cellAt(i, col-1) == cell) {
	    ++col;
	    QTextBlockIterator newCell;
	    while ((newCell = d->cellAt(i, col)) == cell)
		++col;
	    cell = newCell;
	}
        int cursorPos = cell.position()-1;
        for (int j = 0; j < num; ++j)
            d->pieceTable()->insertBlock(cursorPos+j, d->cell_idx, d->pieceTable()->formatCollection()->indexForFormat(QTextCharFormat()));
    }

//     qDebug() << "-------- end insertCols" << pos << num;
    Q_ASSERT(d->dirty);
    d->pieceTable()->endEditBlock();
}

/*!
  removes \a num rows starting at row \a pos.
*/
void QTextTable::removeRows(int pos, int num)
{
    if (d->dirty) d->updateGrid();

//     qDebug() << "-------- removeRows" << pos << num;
    d->pieceTable()->beginEditBlock();

    int nRows = d->rows();

    if (pos < 0 || pos >= nRows || num <= 0)
        return;
    if (pos+num > nRows)
        num = nRows-pos;

    QTextBlockIterator start = d->rowStart(pos);
    QTextBlockIterator end = d->rowEnd(pos+num-1);
    Q_ASSERT(end.blockFormat().tableCellEndOfRow());

//     qDebug("removing from %d length %d", start.position(), end.position()+end.length()-start.position());
    d->pieceTable()->remove(start.position(), end.position()+end.length()-start.position());

    Q_ASSERT(d->dirty);
    d->pieceTable()->endEditBlock();
//     qDebug() << "-------- end removeRows" << pos << num;
}

/*!
  removes \a num columns starting at column \a pos.
*/
void QTextTable::removeCols(int pos, int num)
{
    if (d->dirty) d->updateGrid();

//     qDebug() << "-------- removeCols" << pos << num;
    d->pieceTable()->beginEditBlock();

    int nRows = d->rows();
    int nCols = d->cols();

    if (pos < 0 || pos >= nCols || num <= 0)
	return;
    num = qMin(nCols-pos, num);

    for (int r = nRows-1; r >= 0; --r) {
	// find left edge cell
	int p = pos;
	while (p < pos + num) {
	    QTextBlockIterator cell;
	    while (p < pos + num) {
		cell = d->cellAt(r, p);
		if (!r || d->cellAt(r-1, pos) != cell)
		    break;
		++p;
	    }
	    if (p == pos + num)
		break;

	    // cells spanning to the left
	    if (pos && p == pos && d->cellAt(r, pos-1) == cell) {
		// reduce colspan
		int span = 1;
		while (p + span < pos + num && d->cellAt(r, p + span) == cell)
		    ++span;
		QTextBlockFormat bfmt = cell.blockFormat();
		Q_ASSERT(bfmt.tableCellColSpan() - span > 0);
		bfmt.setTableCellColSpan(bfmt.tableCellColSpan() - span);
		QTextCursor c(cell);
		c.setBlockFormat(bfmt);
		p += span;
		continue;
	    }

	    int span = 1;
	    if (p < pos + num && d->cellAt(r, p+span) == cell)
		++span;

	    // cell spanning to the right
	    if (p + span > pos + num) {
		// reduce colspan
		QTextBlockFormat bfmt = cell.blockFormat();
		Q_ASSERT(bfmt.tableCellColSpan() - span > 0);
		bfmt.setTableCellColSpan(bfmt.tableCellColSpan() - span);
		QTextCursor c(cell);
		c.setBlockFormat(bfmt);
		break;
	    }

	    // remove cell
// 	    qDebug("    removing block %d", cell.n);
	    QTextBlockIterator end = d->blocks.at(d->blocks.indexOf(cell)+1);
	    d->pieceTable()->remove(cell.position(), end.position() - cell.position());
	    p += span;
	}
    }

    Q_ASSERT(d->dirty);
    d->pieceTable()->endEditBlock();
//     qDebug() << "-------- end removeCols" << pos << num;
}

/*!
  Returns the number of rows in the table
*/
int QTextTable::rows() const
{
    if (d->dirty) d->updateGrid();

    return d->rows();
}

/*!
  Returns the number of columns in the table
*/
int QTextTable::cols() const
{
    if (d->dirty) d->updateGrid();

    return d->cols();
}

/*!
  Sets the cell at row \a row and column \a col to span \a rowspan rows.
*/
void QTextTable::setRowSpan(int row, int col, int rowspan)
{
    if (d->dirty) d->updateGrid();

    QTextBlockFormat modifier;
    modifier.setTableCellRowSpan(rowspan);
    QTextBlockIterator cell = d->cellAt(row, col);
    d->pieceTable()->setBlockFormat(cell, cell, modifier, QTextPieceTable::MergeFormat);
}

/*!
  Sets the cell at row \a row and column \a col to span \a colspan columns.
*/
void QTextTable::setColSpan(int row, int col, int colspan)
{
    if (d->dirty) d->updateGrid();

    QTextBlockFormat modifier;
    modifier.setTableCellColSpan(colspan);
    QTextBlockIterator cell = d->cellAt(row, col);
    d->pieceTable()->setBlockFormat(cell, cell, modifier, QTextPieceTable::MergeFormat);
}

/*!
  Returns a QTextCursor pointing to the start of the row that contains \a c.
*/
QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
    int row = d->rowAt(c.position());
    if (row == -1)
	return QTextCursor();
    return QTextCursor(d->rowStart(row));
}

/*!
  Returns a QTextCursor pointing to the end of the row that contains \a c.
*/
QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
    int row = d->rowAt(c.position());
    if (row == -1)
	return QTextCursor();
    return QTextCursor(d->rowEnd(row));
}

/*!
  Returns a QTextCursor pointing to the start of the table.
*/
QTextCursor QTextTable::start() const
{
    return QTextCursor(d->pieceTable(), d->start().position());
}

/*!
  Returns a QTextCursor pointing to the end of the table.
*/
QTextCursor QTextTable::end() const
{
    return QTextCursor(d->pieceTable(), d->end().position());
}

/*! \fn void QTextTable::setFormat(const QTextTableFormat &format)

  Sets the tables format to \a format.
*/

/*! \fn QTextTableFormat QTextTable::format() const

  Returns the format of this table.
*/

/*!
  \reimp
*/
void QTextTable::insertBlock(const QTextBlockIterator &block)
{
    d->dirty = true;
    QTextGroup::insertBlock(block);
    if (d->cell_idx == -1 || d->eor_idx == -1) {
	QTextBlockFormat b = block.blockFormat();
	int idx = d->pieceTable()->formatCollection()->indexForFormat(b);
	if (d->eor_idx == -1 && block.blockFormat().tableCellEndOfRow())
	    d->eor_idx = idx;
	if (d->cell_idx == -1 && !block.blockFormat().tableCellEndOfRow())
	    d->cell_idx = idx;
    }
}

/*!
  \reimp
*/
void QTextTable::removeBlock(const QTextBlockIterator &block)
{
    d->dirty = true;
    QTextGroup::removeBlock(block);
}

void QTextTable::blockFormatChanged(const QTextBlockIterator &)
{
//     qDebug("blockFormatChanged for block %d", it.position());
    d->dirty = true;
}

// ------------------------------------------------------


QTextTablePrivate::~QTextTablePrivate()
{
    free(grid);
}

int QTextTablePrivate::rowAt(int cursor) const
{
    if (dirty) updateGrid();

    for (int i = nRows-1; i >= 0; --i) {
	int c = 0;
	if (i != 0) {
	    // find column
	    for (; c < nCols; ++c) {
		if (cellAt(i-1, c) != cellAt(i, c))
		    break;
	    }
	}
	if (c == nCols)
	    continue;
	if (cellAt(i, c).position() <= cursor)
	    return i;
    }
    return -1;
}

QTextBlockIterator QTextTablePrivate::cellStart(int cursor) const
{
    if (dirty) updateGrid();

    QTextBlockIterator cell;
    for (int i = 0; i < blocks.size(); ++i) {
	QTextBlockIterator it = blocks.at(i);
	if (it.position() > cursor)
	    return cell;
	cell = it;
    }
    return QTextBlockIterator();
}

QTextBlockIterator QTextTablePrivate::cellEnd(int cursor) const
{
    if (dirty) updateGrid();

    if (blocks.at(0).position() > cursor || blocks.last().position() < cursor)
	return QTextBlockIterator();
    for (int i = 0; i < blocks.size(); ++i) {
	QTextBlockIterator it = blocks.at(i);
	if (it.position() > cursor)
	    return --it;
    }
    Q_ASSERT(false);
    return QTextBlockIterator();
}


QTextBlockIterator QTextTablePrivate::rowStart(int row) const
{
    if (dirty) updateGrid();

    if (row >= 0 && row < nRows) {
	// find column
	for (int c = 0; c < nCols; ++c) {
	    QTextBlockIterator cell = cellAt(row, c);
	    if (cell != cellAt(row-1, c))
		return cell;
	}
    }
    return QTextBlockIterator();
}

QTextBlockIterator QTextTablePrivate::rowEnd(int row) const
{
    if (dirty) updateGrid();

    if (row >= 0 && row < nRows) {
	// find column
	for (int c = nCols - 1; c >= 0; --c) {
	    QTextBlockIterator cell = cellAt(row, c);
	    if (!cell.atEnd())
		return cell;
	}
    }
    return QTextBlockIterator();
}

QTextBlockIterator QTextTablePrivate::start() const
{
    return blocks.at(0);
}

QTextBlockIterator QTextTablePrivate::end() const
{
    QTextBlockIterator bit = blocks.last();
    ++bit;
    return bit;
}

void QTextTablePrivate::updateGrid() const
{
//     qDebug("updateGrid:");
    nRows = nCols = 0;

    int maxRow = 0;
    int maxCol = 0;
    int r = 0;
    int c = 0;
    for (int i = 0; i < blocks.size(); ++i) {
	QTextBlockIterator cell = blocks.at(i);
	QTextBlockFormat fmt = cell.blockFormat();
	if (fmt.tableCellEndOfRow()) {
// 	    qDebug("  ... eor: maxCol=%d, c=%d", maxCol, c);
	    maxCol = qMax(c+1, maxCol);
	    c = 0;
	    maxRow++;
	    continue;
	}
	c += fmt.tableCellColSpan();
    }

    nRows = maxRow;
    nCols = maxCol;

 redo:
//     qDebug("   1: nCols = %d, nRows=%d", nCols, nRows);
    grid = (int *)
           realloc(grid, sizeof(int)*nRows*nCols);
    memset(grid, 0, sizeof(int)*nRows*nCols);
    r = c = 0;

    for (int i = 0; i < blocks.size(); ++i) {
	QTextBlockIterator cell = blocks.at(i);
	QTextBlockFormat fmt = cell.blockFormat();
	bool eor = fmt.tableCellEndOfRow();
	int rowspan = eor ? 1 : fmt.tableCellRowSpan();
	int colspan = eor ? 1 : fmt.tableCellColSpan();

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
		for (int jj = 0; jj < colspan; ++jj) {
// 		    qDebug("setting cell %d span=%d/%d at %d/%d", cell.n, rowspan, colspan, r+ii, c+jj);
		    setCell(r+ii, c+jj, cell);
		}
	    maxRow = qMax(maxRow, r+rowspan);
	    maxCol = qMax(maxCol, c+colspan);

	}
	c += colspan;
	if (c > nCols) {
	    nCols = c;
// 	    qDebug(" ---> redo (r=%d, c=%d, colspan=%d)", r, c, colspan);
	    goto redo;
	}
	if (eor) {
	    ++r;
	    c = 0;
	}
    }
    nRows = maxRow;
    // nCols is without the 'eor column', hence the -1
    nCols = maxCol;
//     qDebug("nRows=%d, nCols=%d", nRows, nCols);

    dirty = false;
}

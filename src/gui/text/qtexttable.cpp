#include "qtexttable.h"
#include "qtextcursor.h"
#include "qtextformat.h"
#include "qtextblockiterator.h"
#include <qdebug.h>
#include "qtexttable_p.h"

#include <stdlib.h>

/*!
    \class QTextTableCell qtexttable.h
    \brief The properties of a table cell in a QTextDocument

    \ingroup text

    QTextTableCell describes a specific table cell in a QTextTable.
    It can be queried with the QTextTable::cellAt method.
*/

/*!
  \fn QTextTableCell::QTextTableCell()
  Constructs an invalid QTextTableCell object.

  \sa isValid
*/

/*!
  \fn QTextTableCell::QTextTableCell(const QTextTablePrivate *p, int f)

  \internal
 */


/*!
  Returns the format of this table cell
*/
QTextCharFormat QTextTableCell::format() const
{
    QTextPieceTable *p = d->pieceTable();
    QTextFormatCollection *c = p->formatCollection();
    return c->charFormat(QTextPieceTable::FragmentIterator(&p->fragmentMap(), fragment)->format);
}

int QTextTableCell::row() const
{
    if (d->dirty)
        d->update();

    for (int i = 0; i < d->nCols*d->nRows; ++i) {
        if (d->grid[i] == fragment)
            return i/d->nCols;
    }
    return -1;
}

int QTextTableCell::column() const
{
    if (d->dirty)
        d->update();

    for (int i = 0; i < d->nCols*d->nRows; ++i) {
        if (d->grid[i] == fragment)
            return i%d->nCols;
    }
    return -1;
}

/*!
  \fn int QTextTableCell::rowSpan() const

  Returns the rowSpan of this table cell
*/
int QTextTableCell::rowSpan() const
{
    return format().tableCellRowSpan();
}

/*!
  \fn int QTextTableCell::colSpan() const

  Returns the colSpan of this table cell.
*/
int QTextTableCell::columnSpan() const
{
    return format().tableCellColumnSpan();
}

/*!
  \fn bool QTextTableCell::isValid() const

  Returns if the table cell queried by QTextTable::cellAt describes a
  valid cell.
*/


/*!
  \fn QTextCursor QTextTableCell::start() const

  Returns if a QTextCursor pointing to the start of the cell.
*/
QTextCursor QTextTableCell::start() const
{
    QTextPieceTable *p = d->pieceTable();
    return QTextCursor(p, p->fragmentMap().position(fragment) + 1);
}

/*!
  \fn QTextCursor QTextTableCell::end() const

  Returns if a QTextCursor pointing to the end of the cell.
*/
QTextCursor QTextTableCell::end() const
{
    QTextPieceTable *p = d->pieceTable();
    int index = d->cells.indexOf(fragment) + 1;
    int f = (index == d->cells.size() ? d->fragment_end : d->cells.at(index));
    return QTextCursor(p, p->fragmentMap().position(f));
}


/*!
  \fn QTextCursor QTextTableCell::operator==(const QTextTableCell &other) const

  Returns true if the two objects describe the same table cell
*/

/*!
  \fn QTextCursor QTextTableCell::operator!=(const QTextTableCell &other) const

  Returns true if the two objects do not describe the same table cell
*/

/*!
  \fn QTextTableCell::~QTextTableCell()

  destroys the object.
*/


#define d d_func()
#define q q_func()

QTextTablePrivate::~QTextTablePrivate()
{
}


QTextTable *QTextTablePrivate::createTable(QTextPieceTable *pieceTable, int pos, int rows, int cols, const QTextTableFormat &tableFormat)
{
    QTextTableFormat fmt = tableFormat;
    fmt.setColumns(cols);
    QTextFormatCollection *collection = pieceTable->formatCollection();
    QTextTable *table = qt_cast<QTextTable *>(collection->createGroup(fmt));
    Q_ASSERT(table);

    pieceTable->beginEditBlock();

//     qDebug("---> createTable: rows=%d, cols=%d at %d", rows, cols, pos);
    // add block after table
    QTextCharFormat charFmt;
    charFmt.setNonDeletable(true);
    charFmt.setGroup(table);
    int charIdx = pieceTable->formatCollection()->indexForFormat(charFmt);
    int cellIdx = pieceTable->blockMap().find(pos)->format;

    for (int i = 0; i < rows*cols; ++i) {
        pieceTable->insertBlock(QTextBeginningOfFrame, pos, cellIdx, charIdx);
// 	    qDebug("      addCell at %d", pos);
        ++pos;
    }

    pieceTable->insertBlock(QTextEndOfFrame, pos, cellIdx, charIdx);
// 	qDebug("      addEOR at %d", pos);
    ++pos;

    pieceTable->endEditBlock();

    return table;
}


void QTextTablePrivate::fragmentAdded(const QChar &type, uint fragment)
{
    dirty = true;
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(cells.indexOf(fragment) == -1);
        uint pos = pieceTable()->fragmentMap().position(fragment);
        int i = 0;
        for (; i < cells.size(); ++i) {
            if (pieceTable()->fragmentMap().position(cells.at(i)) > pos)
                break;
        }
        cells.insert(i, fragment);
        if (!d->fragment_start || pos < pieceTable()->fragmentMap().position(d->fragment_start))
            d->fragment_start = fragment;
        return;
    }
    QTextFramePrivate::fragmentAdded(type, fragment);
}

void QTextTablePrivate::fragmentRemoved(const QChar &type, uint fragment)
{
    dirty = true;
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(cells.indexOf(fragment) != -1);
        cells.removeAll(fragment);
        if (d->fragment_start == fragment && cells.size()) {
            d->fragment_start = cells.at(0);
        }
        if (d->fragment_start != fragment)
            return;
    }
    QTextFramePrivate::fragmentRemoved(type, fragment);
}

void QTextTablePrivate::update() const
{
    nCols = q->format().columns();
    nRows = (cells.size() + nCols-1)/nCols;
//     qDebug(">>>> QTextTablePrivate::update, nRows=%d, nCols=%d", nRows, nCols);

    grid = (int *)realloc(grid, nRows*nCols*sizeof(int));
    memset(grid, 0, nRows*nCols*sizeof(int));

    QTextPieceTable *p = pieceTable();
    QTextFormatCollection *c = p->formatCollection();

    int cell = 0;
    for (int i = 0; i < cells.size(); ++i) {
        int fragment = cells.at(i);
        QTextCharFormat fmt = c->charFormat(QTextPieceTable::FragmentIterator(&p->fragmentMap(), fragment)->format);
        int rowspan = fmt.tableCellRowSpan();
        int colspan = fmt.tableCellColumnSpan();

        // skip taken cells
        while (cell < nRows*nCols && grid[cell])
            ++cell;

        int r = cell/nCols;
        int c = cell%nCols;

	if (r + rowspan > nRows) {
	    grid = (int *)realloc(grid, sizeof(int)*(r + rowspan)*nCols);
	    memset(grid + (nRows*nCols), 0, sizeof(int)*(r+rowspan-nRows)*nCols);
	    nRows = r + rowspan;
	}

        Q_ASSERT(c + colspan <= nCols);
        for (int ii = 0; ii < rowspan; ++ii) {
            for (int jj = 0; jj < colspan; ++jj) {
                Q_ASSERT(grid[(r+ii)*nCols + c+jj] == 0);
                grid[(r+ii)*nCols + c+jj] = fragment;
//  		    qDebug("    setting cell %d span=%d/%d at %d/%d", fragment, rowspan, colspan, r+ii, c+jj);
            }
        }
    }
//     qDebug("<<<< end: nRows=%d, nCols=%d", nRows, nCols);

    dirty = false;
}





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
    : QTextFrame(*new QTextTablePrivate, parent)
{
}

/*! \internal
 */
QTextTable::~QTextTable()
{
}


/*!
  Returns a QTextTableCell object describing the properties
  of the table cell at row \a row and column \a col in the table.
*/
QTextTableCell QTextTable::cellAt(int row, int col) const
{
    if (d->dirty)
        d->update();

    if (row < 0 || row >= d->nRows || col < 0 || col >= d->nCols)
        return QTextTableCell();

    return QTextTableCell(d, d->grid[row*d->nCols + col]);
}


/*!
  Returns a QTextTableCell object describing the properties
  of the table cell at cursor position \a c.
*/
QTextTableCell QTextTable::cellAt(const QTextCursor &c) const
{
    if (d->dirty)
        d->update();

    uint pos = c.position();
    const QTextPieceTable::FragmentMap &m = d->pieceTable()->fragmentMap();
    if (m.position(d->fragment_start) >= pos || m.position(d->fragment_end) < pos)
        return QTextTableCell();

    int fragment = d->cells.at(0);
    for (int i = 1; i < d->cells.size(); ++i) {
        int f = d->cells.at(i);
        if (m.position(f) >= pos)
            break;
        fragment = f;
    }
    return QTextTableCell(d, fragment);
}

/*!
  resizes the table to \a rows rows and \a cols cols.
*/
void QTextTable::resize(int rows, int cols)
{
    if (d->dirty)
        d->update();

    d->pieceTable()->beginEditBlock();

    int nRows = this->rows();
    int nCols = this->columns();

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

    d->pieceTable()->endEditBlock();
}

/*!
  inserts \a num rows before row \a pos.
*/
void QTextTable::insertRows(int pos, int num)
{
    if (num <= 0)
	return;

    if (d->dirty)
        d->update();

    if (pos > d->nRows || pos < 0)
        pos = d->nRows;

//     qDebug() << "-------- insertRows" << pos << num;
    QTextPieceTable *p = d->pieceTable();
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    int extended = 0;
    int insert_before = 0;
    if (pos > 0 && pos < d->nRows) {
        for (int i = 0; i < d->nCols; ++i) {
            int cell = d->grid[pos*d->nCols + i];
            if (cell == d->grid[(pos-1)*d->nCols+i]) {
                // cell spans the insertion place, extend it
                QTextPieceTable::FragmentIterator it(&p->fragmentMap(), cell);
                QTextCharFormat fmt = c->charFormat(it->format);
                fmt.setTableCellRowSpan(fmt.tableCellRowSpan() + num);
                p->setCharFormat(it.position(), 1, fmt);
                extended++;
            } else if (!insert_before) {
                insert_before = cell;
            }
        }
    } else {
        insert_before = (pos == 0 ? d->grid[0] : d->fragment_end);
    }
    if (extended < d->nCols) {
        Q_ASSERT(insert_before);
        QTextPieceTable::FragmentIterator it(&p->fragmentMap(), insert_before);
        QTextCharFormat fmt = c->charFormat(it->format);
        fmt.setTableCellRowSpan(1);
        fmt.setTableCellColumnSpan(1);
        Q_ASSERT(fmt.group() == this);
        int pos = it.position();
        int cfmt = p->formatCollection()->indexForFormat(fmt);
        int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());
//         qDebug("inserting %d cells, nCols=%d extended=%d", num*(d->nCols-extended), d->nCols, extended);
        for (int i = 0; i < num*(d->nCols-extended); ++i)
            p->insertBlock(QTextBeginningOfFrame, pos, bfmt, cfmt, UndoCommand::MoveCursor);
    }

//     qDebug() << "-------- end insertRows" << pos << num;
    p->endEditBlock();
}

/*!
  inserts \a num colums before colum \a pos.
*/
void QTextTable::insertCols(int pos, int num)
{
    if (num <= 0)
	return;

    if (d->dirty)
        d->update();

    if (pos > d->nCols || pos < 0)
        pos = d->nCols;

//     qDebug() << "-------- insertCols" << pos << num;
    QTextPieceTable *p = d->pieceTable();
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    for (int i = 0; i < d->nRows; ++i) {
        int cell;
        if (i == d->nRows - 1 && pos == d->nCols)
            cell = d->fragment_end;
        else
            cell = d->grid[i*d->nCols + pos];
        QTextPieceTable::FragmentIterator it(&p->fragmentMap(), cell);
        QTextCharFormat fmt = c->charFormat(it->format);
        if (pos > 0 && pos < d->nCols && cell == d->grid[i*d->nCols + pos - 1]) {
            // cell spans the insertion place, extend it
            fmt.setTableCellColumnSpan(fmt.tableCellColumnSpan() + num);
            p->setCharFormat(it.position(), 1, fmt);
        } else {
            fmt.setTableCellRowSpan(1);
            fmt.setTableCellColumnSpan(1);
            Q_ASSERT(fmt.group() == this);
            int position = it.position();
            int cfmt = p->formatCollection()->indexForFormat(fmt);
            int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());
            for (int i = 0; i < num; ++i)
                p->insertBlock(QTextBeginningOfFrame, position, bfmt, cfmt, UndoCommand::MoveCursor);
        }
    }

    QTextTableFormat tfmt = format();
    tfmt.setColumns(tfmt.columns()+num);
    setFormat(tfmt);

//     qDebug() << "-------- end insertCols" << pos << num;
    p->endEditBlock();
}

/*!
  removes \a num rows starting at row \a pos.
*/
void QTextTable::removeRows(int pos, int num)
{
//     qDebug() << "-------- removeRows" << pos << num;

    if (num <= 0 || pos < 0)
        return;
    if (d->dirty)
        d->update();
    if (pos >= d->nRows)
        return;
    if (pos+num > d->nRows)
        num = d->nRows - pos;

    QTextPieceTable *p = d->pieceTable();
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    for (int r = pos; r < pos + num; ++r) {
        for (int c = 0; c < d->nCols; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextPieceTable::FragmentIterator it(&p->fragmentMap(), cell);
            QTextCharFormat fmt = collection->charFormat(it->format);
            int span = fmt.tableCellRowSpan();
            if (span > 1) {
                fmt.setTableCellRowSpan(span - 1);
                p->setCharFormat(it.position(), 1, fmt);
            } else {
                // remove cell
                int index = d->cells.indexOf(cell) + 1;
                int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
                p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
            }
        }
    }

    p->endEditBlock();
//     qDebug() << "-------- end removeRows" << pos << num;
}

/*!
  removes \a num columns starting at column \a pos.
*/
void QTextTable::removeCols(int pos, int num)
{
//     qDebug() << "-------- removeCols" << pos << num;

    if (num <= 0 || pos < 0)
	return;
    if (d->dirty)
        d->update();
    if (pos >= d->nCols)
        return;
    if (pos + num > d->nCols)
        pos = d->nCols - num;

    QTextPieceTable *p = d->pieceTable();
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    for (int r = 0; r < d->nRows; ++r) {
        for (int c = pos; c < pos + num; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextPieceTable::FragmentIterator it(&p->fragmentMap(), cell);
            QTextCharFormat fmt = collection->charFormat(it->format);
            int span = fmt.tableCellColumnSpan();
            if (span > 1) {
                fmt.setTableCellColumnSpan(span - 1);
                p->setCharFormat(it.position(), 1, fmt);
            } else {
                // remove cell
                int index = d->cells.indexOf(cell) + 1;
                int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
                p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
            }
        }
    }

    QTextTableFormat tfmt = format();
    tfmt.setColumns(tfmt.columns()-num);
    setFormat(tfmt);

    p->endEditBlock();
//     qDebug() << "-------- end removeCols" << pos << num;
}

/*!
  Returns the number of rows in the table
*/
int QTextTable::rows() const
{
    if (d->dirty)
        d->update();

    return d->nRows;
}

/*!
  Returns the number of columns in the table
*/
int QTextTable::columns() const
{
    if (d->dirty)
        d->update();

    return d->nCols;
}

#if 0
void QTextTable::mergeCells(const QTextCursor &selection)
{
}
#endif

/*!
  Returns a QTextCursor pointing to the start of the row that contains \a c.
*/
QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row();
    QTextPieceTable *p = d->pieceTable();
    QTextPieceTable::FragmentIterator it(&p->fragmentMap(), d->grid[row*d->nCols]);
    return QTextCursor(p, it.position());
}

/*!
  Returns a QTextCursor pointing to the end of the row that contains \a c.
*/
QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row() + 1;
    int fragment = row < d->nRows ? d->grid[row*d->nCols] : d->fragment_end;
    QTextPieceTable *p = d->pieceTable();
    QTextPieceTable::FragmentIterator it(&p->fragmentMap(), fragment);
    return QTextCursor(p, it.position() - 1);
}

/*! \fn void QTextTable::setFormat(const QTextTableFormat &format)

  Sets the tables format to \a format.
*/

/*! \fn QTextTableFormat QTextTable::format() const

  Returns the format of this table.
*/


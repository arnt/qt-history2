#include "qtexttable.h"
#include "qtextcursor.h"
#include "qtextformat.h"
#include <qdebug.h>
#include "qtexttable_p.h"

#include <stdlib.h>
#define d d_func()
#define q q_func()

/*!
    \class QTextTableCell qtexttable.h
    \brief The QTextTableCell class represents the properties of a
    cell in a QTextTable.

    \ingroup text

    The QTextTable::cellAt() functions return a QTextTableCell object
    for the given cell. A QTextTableCell holds the format() of a cell,
    its row() and column(), and rowSpan() and columnSpan(), and the
    firstCursorPosition() and lastCursorPosition() for the cell.
*/

/*!
    \fn QTextTableCell::QTextTableCell()

    Constructs an invalid QTextTableCell object.

    \sa isValid()
*/

/*!
    \fn QTextTableCell::QTextTableCell(const QTextTableCell &cell)

    Creates a new QTextTableCell object based on cell \a cell.
*/

/*!
    \fn QTextTableCell& QTextTableCell::operator=(const QTextTableCell &other)

    Assigns the \a other table cell to this table cell.
*/

/*!
  \fn QTextTableCell::QTextTableCell(const QTextTablePrivate *p, int f)

  \internal
 */


/*!
    Returns the cell's character format.
*/
QTextCharFormat QTextTableCell::format() const
{
    QTextDocumentPrivate *p = table->docHandle();
    QTextFormatCollection *c = p->formatCollection();
    return c->charFormat(QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format);
}

/*!
    Returns the row in the table that contains this cell.

    \sa column()
*/
int QTextTableCell::row() const
{
    const QTextTablePrivate *tp = table->d;
    if (tp->dirty)
        tp->update();

    for (int i = 0; i < tp->nCols*tp->nRows; ++i) {
        if (tp->grid[i] == fragment)
            return i/tp->nCols;
    }
    return -1;
}

/*!
    Returns the column in the table that contains this cell.

    \sa row()
*/
int QTextTableCell::column() const
{
    const QTextTablePrivate *tp = table->d;
    if (tp->dirty)
        tp->update();

    for (int i = 0; i < tp->nCols*tp->nRows; ++i) {
        if (tp->grid[i] == fragment)
            return i%tp->nCols;
    }
    return -1;
}

/*!
    Returns the number of rows this cell spans. The default is 1.

    \sa columnSpan()
*/
int QTextTableCell::rowSpan() const
{
    return format().tableCellRowSpan();
}

/*!
    Returns the number of columns this cell spans. The default is 1.

    \sa rowSpan()
*/
int QTextTableCell::columnSpan() const
{
    return format().tableCellColumnSpan();
}

/*!
    \fn bool QTextTableCell::isValid() const

    Returns true if this is a valid table cell; otherwise returns
    false.
*/


/*!
    Returns the first valid cursor position in this cell.

    \sa lastCursorPosition()
*/
QTextCursor QTextTableCell::firstCursorPosition() const
{
    return QTextCursor(table->d->pieceTable, firstPosition());
}

/*!
    Returns the last valid cursor position in this cell.

    \sa firstCursorPosition()
*/
QTextCursor QTextTableCell::lastCursorPosition() const
{
    return QTextCursor(table->d->pieceTable, lastPosition());
}


/*!
    \internal
*/
int QTextTableCell::firstPosition() const
{
    QTextDocumentPrivate *p = table->docHandle();
    return p->fragmentMap().position(fragment) + 1;
}

/*!
    \internal
*/
int QTextTableCell::lastPosition() const
{
    QTextDocumentPrivate *p = table->docHandle();
    int index = table->d->cells.indexOf(fragment) + 1;
    int f = (index == table->d->cells.size() ? table->d->fragment_end : table->d->cells.at(index));
    return p->fragmentMap().position(f);
}


QTextFrame::iterator QTextTableCell::begin() const
{
    QTextDocumentPrivate *p = table->docHandle();
    int b = p->blockMap().findNode(firstPosition());
    int e = p->blockMap().findNode(lastPosition()+1);
    return QTextFrame::iterator(const_cast<QTextTable *>(table), b, b, e);
}

QTextFrame::iterator QTextTableCell::end() const
{
    QTextDocumentPrivate *p = table->docHandle();
    int b = p->blockMap().findNode(firstPosition());
    int e = p->blockMap().findNode(lastPosition()+1);
    return QTextFrame::iterator(const_cast<QTextTable *>(table), b, b, e);
}


/*!
    \fn QTextCursor QTextTableCell::operator==(const QTextTableCell &other) const

    Returns true if this cell object and the \a other cell object
    describe the same cell; otherwise returns false.
*/

/*!
    \fn QTextCursor QTextTableCell::operator!=(const QTextTableCell &other) const

    Returns true if this cell object and the \a other cell object
    describe different cells; otherwise returns false.
*/

/*!
    \fn QTextTableCell::~QTextTableCell()

    Destroys the object.
*/

QTextTablePrivate::~QTextTablePrivate()
{
}


QTextTable *QTextTablePrivate::createTable(QTextDocumentPrivate *pieceTable, int pos, int rows, int cols, const QTextTableFormat &tableFormat)
{
    QTextTableFormat fmt = tableFormat;
    fmt.setColumns(cols);
    QTextTable *table = qt_cast<QTextTable *>(pieceTable->createObject(fmt));
    Q_ASSERT(table);

    pieceTable->beginEditBlock();

//     qDebug("---> createTable: rows=%d, cols=%d at %d", rows, cols, pos);
    // add block after table
    QTextCharFormat charFmt;
    charFmt.setObjectIndex(table->objectIndex());
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
        uint pos = pieceTable->fragmentMap().position(fragment);
        int i = 0;
        for (; i < cells.size(); ++i) {
            if (pieceTable->fragmentMap().position(cells.at(i)) > pos)
                break;
        }
        cells.insert(i, fragment);
        if (!d->fragment_start || pos < pieceTable->fragmentMap().position(d->fragment_start))
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

    QTextDocumentPrivate *p = pieceTable;
    QTextFormatCollection *c = p->formatCollection();

    int cell = 0;
    for (int i = 0; i < cells.size(); ++i) {
        int fragment = cells.at(i);
        QTextCharFormat fmt = c->charFormat(QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format);
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
    \brief The QTextTable class represents a table in a QTextDocument.

    \ingroup text

    Tables can be created using QTextCursor::createTable() and queried
    with QTextCursor::currentTable().

    A table's size can be changed with resize(), or by using
    insertRows(), insertColumns(), removeRows(), or removeColumns().
    The overall format of the table can be changed with setFormat().
    Use cellAt() to retrieve a QTextTableCell object that gives the
    properties of a given cell.

    The cursor position of table rows in the document is available
    from rowStart() and rowEnd().

*/

/*! \internal
 */
QTextTable::QTextTable(QTextDocument *doc)
    : QTextFrame(*new QTextTablePrivate, doc)
{
}

/*! \internal
 */
QTextTable::~QTextTable()
{
}


/*!
    Returns a QTextTableCell object that describes the properties of
    the specified table cell. The cell is identified by its \a row and
    \a col.
*/
QTextTableCell QTextTable::cellAt(int row, int col) const
{
    if (d->dirty)
        d->update();

    if (row < 0 || row >= d->nRows || col < 0 || col >= d->nCols)
        return QTextTableCell();

    return QTextTableCell(this, d->grid[row*d->nCols + col]);
}

/*!
    \overload

    The cell is identified by its \a position.
*/
QTextTableCell QTextTable::cellAt(int position) const
{
    if (d->dirty)
        d->update();

    uint pos = (uint)position;
    const QTextDocumentPrivate::FragmentMap &m = d->pieceTable->fragmentMap();
    if (position < 0 || m.position(d->fragment_start) >= pos || m.position(d->fragment_end) < pos)
        return QTextTableCell();

    int fragment = d->cells.at(0);
    for (int i = 1; i < d->cells.size(); ++i) {
        int f = d->cells.at(i);
        if (m.position(f) >= pos)
            break;
        fragment = f;
    }
    return QTextTableCell(this, fragment);
}

/*!
    \overload

    The cell is identified by its cursor position, \a c.
*/
QTextTableCell QTextTable::cellAt(const QTextCursor &c) const
{
    return cellAt(c.position());
}

/*!
    Resizes the table to \a rows rows and \a cols cols.

    \sa insertRows(), insertColumns(), removeRows(), removeColumns()
*/
void QTextTable::resize(int rows, int cols)
{
    if (d->dirty)
        d->update();

    d->pieceTable->beginEditBlock();

    int nRows = this->rows();
    int nCols = this->columns();

    if (rows == nRows && cols == nCols)
	return;

    if (nCols < cols)
        insertColumns(nCols, cols - nCols);
    else if (nCols > cols)
        removeColumns(cols, nCols - cols);

    if (nRows < rows)
        insertRows(nRows, rows-nRows);
    else if (nRows > rows)
        removeRows(rows, nRows-rows);

    d->pieceTable->endEditBlock();
}

/*!
    Inserts \a num rows before row \a pos.

    \sa resize(), insertColumns(), removeRows(), removeColumns()
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
    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    int extended = 0;
    int insert_before = 0;
    if (pos > 0 && pos < d->nRows) {
        for (int i = 0; i < d->nCols; ++i) {
            int cell = d->grid[pos*d->nCols + i];
            if (cell == d->grid[(pos-1)*d->nCols+i]) {
                // cell spans the insertion place, extend it
                QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
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
        QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), insert_before);
        QTextCharFormat fmt = c->charFormat(it->format);
        fmt.setTableCellRowSpan(1);
        fmt.setTableCellColumnSpan(1);
        Q_ASSERT(fmt.objectIndex() == objectIndex());
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
    Inserts \a num colums before column \a pos.

    \sa insertRows(), resize(), removeRows(), removeColumns()
*/
void QTextTable::insertColumns(int pos, int num)
{
    if (num <= 0)
	return;

    if (d->dirty)
        d->update();

    if (pos > d->nCols || pos < 0)
        pos = d->nCols;

//     qDebug() << "-------- insertCols" << pos << num;
    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    for (int i = 0; i < d->nRows; ++i) {
        int cell;
        if (i == d->nRows - 1 && pos == d->nCols)
            cell = d->fragment_end;
        else
            cell = d->grid[i*d->nCols + pos];
        QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
        QTextCharFormat fmt = c->charFormat(it->format);
        if (pos > 0 && pos < d->nCols && cell == d->grid[i*d->nCols + pos - 1]) {
            // cell spans the insertion place, extend it
            fmt.setTableCellColumnSpan(fmt.tableCellColumnSpan() + num);
            p->setCharFormat(it.position(), 1, fmt);
        } else {
            fmt.setTableCellRowSpan(1);
            fmt.setTableCellColumnSpan(1);
            Q_ASSERT(fmt.objectIndex() == objectIndex());
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
    Removes \a num rows starting at row \a pos.

    \sa insertRows(), insertColumns(), resize(), removeColumns()
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

    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    for (int r = pos; r < pos + num; ++r) {
        for (int c = 0; c < d->nCols; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
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
    Removes \a num columns starting at column \a pos.

    \sa insertRows(), insertColumns(), removeRows(), resize()
*/
void QTextTable::removeColumns(int pos, int num)
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

    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    for (int r = 0; r < d->nRows; ++r) {
        for (int c = pos; c < pos + num; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
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
    Returns the number of rows in the table.

    \sa columns()
*/
int QTextTable::rows() const
{
    if (d->dirty)
        d->update();

    return d->nRows;
}

/*!
    Returns the number of columns in the table.

    \sa rows()
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
    Returns a QTextCursor pointing to the start of the row that
    contains cursor position \a c.

    \sa rowEnd()
*/
QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row();
    QTextDocumentPrivate *p = d->pieceTable;
    QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), d->grid[row*d->nCols]);
    return QTextCursor(p, it.position());
}

/*!
    Returns a QTextCursor pointing to the end of the row that contains
    cursor position \a c.

    \sa rowStart()
*/
QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row() + 1;
    int fragment = row < d->nRows ? d->grid[row*d->nCols] : d->fragment_end;
    QTextDocumentPrivate *p = d->pieceTable;
    QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), fragment);
    return QTextCursor(p, it.position() - 1);
}

/*!
    \fn void QTextTable::setFormat(const QTextTableFormat &format)

    Sets the table's format to \a format.

    \sa format()
*/

/*!
    \fn QTextTableFormat QTextTable::format() const

    Returns the table's format.

    \sa setFormat()
*/


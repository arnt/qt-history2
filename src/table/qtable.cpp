/****************************************************************************
**
** Implementation of QTable widget class
**
** Created : 000607
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the table module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// needed for qsort (at least for Borland)
#include "qplatformdefs.h"

#include "qtable.h"

#ifndef QT_NO_TABLE

#include "qpainter.h"
#include "qkeycode.h"
#include "qlineedit.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qtimer.h"
#include "qobjectlist.h"
#include "qiconset.h"
#include "qcombobox.h"
#include "qcheckbox.h"
#include "qdragobject.h"
#include "qevent.h"

#include <stdlib.h>

class Q_EXPORT QTableHeader : public QHeader
{
    friend class QTable;
    Q_OBJECT

public:
    enum SectionState {
	Normal,
	Bold,
	Selected
    };

    QTableHeader( int, QTable *t, QWidget *parent=0, const char *name=0 );
    ~QTableHeader() {};
    void addLabel( const QString &s , int size );

    void setSectionState( int s, SectionState state );
    void setSectionStateToAll( SectionState state );
    SectionState sectionState( int s ) const;

    int sectionSize( int section ) const;
    int sectionPos( int section ) const;
    int sectionAt( int section ) const;

    void setSectionStretchable( int s, bool b );
    bool isSectionStretchable( int s ) const;

signals:
    void sectionSizeChanged( int s );

protected:
    void paintEvent( QPaintEvent *e );
    void paintSection( QPainter *p, int index, const QRect& fr );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );

private slots:
    void doAutoScroll();
    void sectionWidthChanged( int col, int os, int ns );
    void indexChanged( int sec, int oldIdx, int newIdx );
    void updateStretches();
    void updateWidgetStretches();

private:
    void updateSelections();
    void saveStates();
    void setCaching( bool b );
    void swapSections( int oldIdx, int newIdx );
    bool doSelection( QMouseEvent *e );

private:
    QMemArray<int> states, oldStates;
    QMemArray<bool> stretchable;
    QMemArray<int> sectionSizes, sectionPoses;
    bool mousePressed;
    int pressPos, startPos, endPos;
    QTable *table;
    QTimer *autoScrollTimer;
    QWidget *line1, *line2;
    bool caching;
    int resizedSection;
    bool isResizing;
    int numStretches;
    QTimer *stretchTimer, *widgetStretchTimer;
    QTableHeaderPrivate *d;

};

struct QTablePrivate
{
};

struct QTableHeaderPrivate
{
};


/*! \class QTableSelection qtable.h

  \brief The QTableSelection class provides access to a selected area in a
  QTable.

  \module table

  The selection is a rectangular set of cells.  One of the rectangle's
  cells is called the anchor cell; this is the cell that was selected first.
  The init() function sets the anchor and the selection rectangle
  to exactly this cell; the expandTo() function expands the selection
  rectangle to include additional cells.

  There are various access functions to find out about the area:
  anchorRow() and anchorCol() return the anchor's position; leftCol(),
  rightCol(), topRow() and bottomRow() return the rectangle's four
  edges. All four are part of the selection.

  A newly created QTableSelection is inactive -- isActive() returns
  FALSE.  You must use init() and expandTo() to activate it.

  \sa QTable QTable::addSelection() QTable::selection().
*/

/*! Creates an inactive selection. Use init() and expandTo() to
  activate it.
*/

QTableSelection::QTableSelection()
    : active( FALSE ), inited( FALSE ), tRow( -1 ), lCol( -1 ),
      bRow( -1 ), rCol( -1 ), aRow( -1 ), aCol( -1 )
{
}

/*! Sets the selection anchor to cell \a row, \a col and
  the selection to contain only this cell.

  To extend the selection to include additional cells, call expandTo().

  \sa isActive()
*/

void QTableSelection::init( int row, int col )
{
    aCol = lCol = rCol = col;
    aRow = tRow = bRow = row;
    active = FALSE;
    inited = TRUE;
}

/*! Expands the selection to include cell \a row, \a col. The new
    selection rectangle is the bounding rectangle of \a row, \a col and
    the previous selection rectangle. After calling this function the
    selection is active.

  If you haven't called init(), this function does nothing.

  \sa init() isActive()
*/

void QTableSelection::expandTo( int row, int col )
{
    if ( !inited )
	return;
    active = TRUE;

    if ( row < aRow ) {
	tRow = row;
	bRow = aRow;
    } else {
	tRow = aRow;
	bRow = row;
    }

    if ( col < anchorCol() ) {
	lCol = col;
	rCol = aCol;
    } else {
	lCol = aCol;
	rCol = col;
    }
}

/*! Returns TRUE if \a s includes the same cells as \e this selection;
 otherwise returns FALSE.
*/

bool QTableSelection::operator==( const QTableSelection &s ) const
{
    return ( s.active == active &&
	     s.tRow == tRow && s.bRow == bRow &&
	     s.lCol == lCol && s.rCol == rCol );
}

/*! \fn int QTableSelection::topRow() const

  Returns the top row of the selection.

  \sa bottomRow() leftCol() rightCol()
*/

/*! \fn int QTableSelection::bottomRow() const

  Returns the bottom row of the selection.

  \sa topRow() leftCol() rightCol()
*/

/*! \fn int QTableSelection::leftCol() const

  Returns the left column of the selection.

  \sa topRow() bottomRow() rightCol()
*/

/*! \fn int QTableSelection::rightCol() const

  Returns the right column of the selection.

  \sa topRow() bottomRow() leftCol()
*/

/*! \fn int QTableSelection::anchorRow() const

  Returns the anchor row of the selection.

  \sa anchorCol() expandTo()
*/

/*! \fn int QTableSelection::anchorCol() const

  Returns the anchor column of \e this selection.

  \sa anchorRow() expandTo()
*/

/*! \fn bool QTableSelection::isActive() const

  Returns whether the selection is active or not. A selection is
  active after init() and expandTo() have been called.
*/


/*! \class QTableItem qtable.h

  \brief The QTableItem class provides the cell content for QTable cells.

  \module table

  For many applications QTableItems are ideal for presenting and editing
  the contents of table cells. In situations where you need to create
  very large tables you may prefer an alternative approach to using
  QTableItems: see the notes on <a href="#bigtables">large tables</a>.

    A QTableItem contains a cell's data, by default, a string and a
    pixmap. The table item also holds the cell's display size and how
    the data should be aligned. The table item specifies the cell's
    EditType and the editor used for in-place editing (by default a
    QLineEdit). If you want checkboxes use \l{QCheckTableItem}, and if
    you want comboboxes use \l{QComboTableItem}. The \l EditType (set in
    the constructor) determines whether the cell's contents may be
    edited; setReplaceable() sets whether the cell's contents may be
    replaced by another cell's contents.

    If a pixmap is specified it is displayed to the left of any text.
    You can change the text or pixmap with setText() and setPixmap()
    respectively. For text you can use setWordWrap(). A table item's
    alignment is set in the constructor.

    Reimplement createEditor() and setContentFromEditor() if you want to
    use your own widget instead of a QLineEdit for editing cell
    contents. Reimplement paint() if you want to display custom content.
    If you want a checkbox table item use QCheckTableItem, and if you
    want a combo table item use \l{QComboTableItem}.

    When sorting table items the key() function is used; by default this
    returns the table item's text(). Reimplement key() to customize how your
    table items will sort.

    Table items are inserted into a table using QTable::setItem(). If
    you insert an item into a cell that already contains a table item
    the original item will be deleted.

    Example:
    \code
    for ( int row = 0; row < table->numRows(); row++ ) {
	for ( int col = 0; col < table->numCols(); col++ ) {
	    table->setItem( row, col,
		new QTableItem( table, WhenCurrent, QString::number( row * col ) ) );
	}
    }
    \endcode

    You can move a table item from one cell to another, in the same or a
    different table, using QTable::takeItem() and QTable::setItem(); but
    see also QTable::swapCells().

    Table items can be deleted with delete in the standard way; the
    table and cell will be updated accordingly.

*/

/*! \fn QTable *QTableItem::table() const

  Returns the QTable the table item belongs to.

  \sa QTable::setItem() QTableItem()
*/

/*! \enum QTableItem::EditType

  <a name="wheneditable">
  This enum is used to define whether a cell is editable or read-only
  (in conjunction with other settings), and how the cell should
  be displayed.

  \value Always
    The cell always looks editable.<sup>1.</sup>
    <br>
    Using this EditType ensures that the editor created with
    createEditor() (by default a QLineEdit) is always visible. This has
    implications for the alignment of the content: the default editor
    aligns everything (even numbers) to the left whilst numerical values
    in the cell are by default aligned to the right.
    <br>
    If a cell with the edit type \c Always looks misaligned you could
    reimplement createEditor() for these items.
    <br><br>

  \value WhenCurrent
    The cell looks editable only when it has keyboard
    focus (see QTable::setCurrentCell()).<sup>1.</sup>
    <br><br>

  \value OnTyping
    The cell only looks editable when the user types in it or
    double-clicks it.<sup>1.</sup> It resembles the \c WhenCurrent
    functionality but can look a bit cleaner.
    <br>
    The \c OnTyping edit type is the default when QTableItem objects
    are created by the convenience functions QTable::setText()
    and QTable::setPixmap().
    <br><br>

  \value Never  The cell is not editable.

  <sup>1.</sup> The cell is editable only if QTable::isRowReadOnly() is
  FALSE for its row, QTable::isColumnReadOnly() is FALSE for its column,
  and QTable::isReadOnly() is FALSE.

  Note that QComboTableItems have an isEditable() property. This
  property is used to indicate whether the user may enter their own text
  or are restricted to choosing one of the choices in the list.
  QComboTableItems may be edited (i.e. interacted with) only if they are
  editable in accordance with their EditType as described above.

*/

/*! Creates a child item of table \a table with text \a text.
  The item has the \l EditType \a et.

    The table item will use a QLineEdit for its editor, will not
    word-wrap and will occupy a single cell. Insert the table item into
    a table with QTable::setItem().

    The table takes ownership of the table item, so a table item should
    not be inserted into more than one table at a time.
*/

QTableItem::QTableItem( QTable *table, EditType et, const QString &text )
    : txt( text ), pix(), t( table ), edType( et ), wordwrap( FALSE ),
      tcha( TRUE ), rw( -1 ), cl( -1 ), rowspan( 1 ), colspan( 1 )
{
    enabled = TRUE;
}

/*! Creates a child item of table \a table with text \a text and
  pixmap \a p. The item has the \l EditType \a et.

    The table item will display the pixmap to the left of the text. It
    will use a QLineEdit for editing the text, will not word-wrap and
    will occupy a single cell. Insert the table item into a table with
    QTable::setItem().

    The table takes ownership of the table item, so a table item should
    not be inserted in more than one table at a time.
*/

QTableItem::QTableItem( QTable *table, EditType et,
			const QString &text, const QPixmap &p )
    : txt( text ), pix( p ), t( table ), edType( et ), wordwrap( FALSE ),
      tcha( TRUE ), rw( -1 ), cl( -1 ), rowspan( 1 ), colspan( 1 )
{
    enabled = TRUE;
}

/*! The destructor deletes this item and frees all allocated resources.

    If the table item is in a table (i.e. was inserted with setItem()),
    it will be removed from the table and the cell it occupied.
*/

QTableItem::~QTableItem()
{
    table()->takeItem( this );
}

int QTableItem::RTTI = 0;

/*!
    Returns the Run Time Type Identification number of this table item which
    for QTableItems is 0.

  Although often frowned upon by purists, Run Time Type Identification
  is very useful for QTables as it allows for an efficient indexed
  storage mechanism.

  When you create subclasses based on QTableItem make sure that each
  subclass returns a unique rtti() value. It is advisable to use values
  greater than 1000, preferably large random numbers, to allow for
  extensions to this class.

  \sa QCheckTableItem::rtti() QComboTableItem::rtti()
*/

int QTableItem::rtti() const
{
    return RTTI;
}

/*! Returns the table item's pixmap or a null-pixmap if no pixmap has been
  set.

  \sa setPixmap() text()
*/

QPixmap QTableItem::pixmap() const
{
    return pix;
}


/*! Provides the text of the table item or an empty string if there's no text.

  \sa setText() pixmap()
*/

QString QTableItem::text() const
{
    if ( edType == Always ) //### why only always?
	((QTableItem*)this)->setContentFromEditor(table()->cellWidget(rw,cl));
    return txt;
}

/*! Sets pixmap \a p to be this item's pixmap.

  Note that setPixmap() does not update the cell the table item belongs
  to. Use QTable::updateCell() to repaint the cell's contents.

  For \l{QComboTableItem}s and \l{QCheckTableItem}s this function has no
  visible effect.

  \sa QTable::setPixmap() pixmap() setText()
*/

void QTableItem::setPixmap( const QPixmap &p )
{
    pix = p;
}

/*! Changes the text of the table item to \a str.

  Note that setText() does not update the cell the table item belongs
  to. Use QTable::updateCell() to repaint the cell's contents.

  \sa QTable::setText() text() setPixmap() QTable::updateCell()
*/

void QTableItem::setText( const QString &str )
{
    txt = str;
}

/*! This virtual function is used to paint the contents of an item
  on the painter \a p in the rectangular area \a cr using
  the color group \a cg.

  If \a selected is TRUE the cell is displayed in a way that indicates
  that it is highlighted.

  You don't usually need to use this function but if you want
  to draw custom content in a cell you will need to reimplement it.
*/

void QTableItem::paint( QPainter *p, const QColorGroup &cg,
			const QRect &cr, bool selected )
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
		 selected ? cg.brush( QColorGroup::Highlight )
			  : cg.brush( QColorGroup::Base ) );

    int w = cr.width();
    int h = cr.height();

    int x = 0;
    if ( !pix.isNull() ) {
	p->drawPixmap( 0, ( cr.height() - pix.height() ) / 2, pix );
	x = pix.width() + 2;
    }

    if ( selected )
	p->setPen( cg.highlightedText() );
    else
	p->setPen( cg.text() );
    p->drawText( x + 2, 0, w - x - 4, h,
		 wordwrap ? (alignment() | WordBreak) : alignment(), txt );
}

/*! This virtual function creates an editor which the user can interact
    with to edit the cell's contents. The default implementation creates a
    QLineEdit.

    If the function returns 0, the cell is read-only.

  The returned widget should preferably be invisible, ideally with
  QTable::viewport() as parent.

  If you reimplement this function you'll almost certainly need to
  reimplement setContentFromEditor(), and may need to reimplement
  sizeHint().

    \walkthrough statistics/statistics.cpp
    \skipto createEditor
    \printto }

  \sa QTable::createEditor() setContentFromEditor() QTable::viewport()
*/

QWidget *QTableItem::createEditor() const
{
    QLineEdit *e = new QLineEdit( table()->viewport() );
    e->setFrame( FALSE );
    e->setText( text() );
    QObject::connect( e, SIGNAL( returnPressed() ), table(), SLOT( doValueChanged() ) );
    return e;
}

/*! Whenever the content of a cell has been edited by the editor \a
  w, QTable calls this virtual function to copy the new values into
  the QTableItem.

  If you reimplement createEditor() and return something that is not a
  QLineEdit you will almost certainly have to reimplement this function.

    \walkthrough statistics/statistics.cpp
    \skipto setContentFromEditor
    \printto }

  \sa QTable::setCellContentFromEditor()
*/

void QTableItem::setContentFromEditor( QWidget *w )
{
    if ( w && w->inherits( "QLineEdit" ) )
	setText( ( (QLineEdit*)w )->text() );
}

/*! The alignment function returns how the text contents of the cell are
  aligned when drawn. The default implementation aligns numbers to the
  right and any other text to the left.

  \sa Qt::AlignmentFlags
*/

// ed: For consistency reasons a setAlignment() should be provided
// as well.

int QTableItem::alignment() const
{
    bool num;
    bool ok1 = FALSE, ok2 = FALSE;
    (void)txt.toInt( &ok1 );
    if ( !ok1 )
	(void)txt.toDouble( &ok2 ); // ### should be .-aligned
    num = ok1 || ok2;

    return ( num ? AlignRight : AlignLeft ) | AlignVCenter;
}

/*! If \a b is TRUE, the cell's text will be wrapped over multiple lines,
 when necessary, to fit the width of the cell; otherwise the text will
 be written as a single line.

  \sa wordWrap() QTable::adjustColumn() QTable::setColumnStretchable()
*/

void QTableItem::setWordWrap( bool b )
{
    wordwrap = b;
}

/*!
    Returns TRUE if the word wrap is enabled for the cell, otherwise
    returns FALSE.

  \sa setWordWrap()
*/

bool QTableItem::wordWrap() const
{
    return wordwrap;
}

/*! \internal */

void QTableItem::updateEditor( int oldRow, int oldCol )
{
    if ( edType != Always )
	return;
    if ( oldRow != -1 && oldCol != -1 )
	table()->clearCellWidget( oldRow, oldCol );
    if ( rw != -1 && cl != -1 )
	table()->setCellWidget( rw, cl, createEditor() );
}

/*! Returns the table item's edit type.

  This is set when the table item is constructed.

  \sa EditType QTableItem()
*/

QTableItem::EditType QTableItem::editType() const
{
    return edType;
}

/*!
    If \a b is TRUE it is acceptable to replace the contents of the cell
    with the contents of another QTableItem. If \a b is FALSE the
    contents of the cell may not be replaced by the contents of another
    table item. Table items that span more than one cell may not have
    their contents replaced by another table item.

    (This differs from \l EditType because EditType is concerned with
    whether the \e user is able to change the contents of a cell.)

  \sa isReplaceable()
*/

void QTableItem::setReplaceable( bool b )
{
    tcha = b;
//ed: what the heck does tcha stand for?
}

/*! This function returns whether the contents of the cell may be
    replaced with the contents of another table item. Regardless of this
    setting, table items that span more than one cell may not have their
    contents replaced by another table item.

    (This differs from \l EditType because EditType is concerned with
    whether the \e user is able to change the contents of a cell.)

 \sa setReplaceable() EditType
*/

bool QTableItem::isReplaceable() const
{
    if ( rowspan > 1 || colspan > 1 )
	return FALSE;
    return tcha;
}

/*! This virtual function returns the key that should be used for
  sorting. The default implementation returns the text() of the
  relevant item.

  \sa QTable::setSorting
*/

QString QTableItem::key() const
{
    return text();
}

/*! This virtual function returns the size a cell needs to show its
  entire content.

  If you subclass QTableItem you will often need to reimplement this
  function.
*/

QSize QTableItem::sizeHint() const
{
    if ( edType == Always && table()->cellWidget( rw, cl ) )
	return table()->cellWidget( rw, cl )->sizeHint();

    QSize s;
    if ( !pix.isNull() ) {
	s = pix.size();
	s.setWidth( s.width() + 2 );
    }

    if ( !wordwrap )
	return QSize( s.width() + table()->fontMetrics().width( text() ) + 10, QMAX( s.height(), table()->fontMetrics().height() ) );
    QRect r = table()->fontMetrics().boundingRect( 0, 0, table()->columnWidth( col() ), 0, wordwrap ? (alignment() | WordBreak) : alignment(), txt );
    return QSize( s.width() + r.width(), QMAX( s.height(), r.height() ) );
}

/*!
    Changes the extent of the QTableItem so that it spans multiple cells
    covering \a rs rows and \a cs columns. The top left cell is the
    original cell.

  \sa rowSpan() colSpan()
*/

void QTableItem::setSpan( int rs, int cs )
{
    int rrow = rw;
    int rcol = cl;
    if ( rowspan > 1 || colspan > 1 ) {
	table()->takeItem( this );
	table()->setItem( rrow, rcol, this );
    }

    rowspan = rs;
    colspan = cs;

    for ( int r = 0; r < rowspan; ++r ) {
	for ( int c = 0; c < colspan; ++c ) {
	    if ( r == 0 && c == 0 )
		continue;
	    table()->setItem( r + rw, c + cl, this );
	    rw = rrow;
	    cl = rcol;
	}
    }

    table()->updateCell( rw, cl );
}

/*! Returns the row span of the table item, usually 1.

  \sa setSpan() colSpan()
*/

int QTableItem::rowSpan() const
{
    return rowspan;
}

/*! Returns the column span of the table item, usually 1.

  \sa setSpan() rowSpan()
*/

int QTableItem::colSpan() const
{
    return colspan;
}

/*! Sets row \a r as the table item's row. Usually you do not need to
  call this function.

  If the cell spans multiple rows, this function sets the top row and
  retains the height of the multi-cell table item.

  \sa row() setCol() rowSpan()
*/

void QTableItem::setRow( int r )
{
    rw = r;
}

/*! Sets column \a c as the table item's column. Usually you will not
    need to call this function.

  If the cell spans multiple columns, this function sets the left-most
  column and retains the width of the multi-cell table item.

  \sa col() setRow() colSpan()
*/

void QTableItem::setCol( int c )
{
    cl = c;
}

/*! Returns the row where the table item is located. If the cell spans
  multiple rows, this function returns the top-most row.

  \sa col() setRow()
*/

int QTableItem::row() const
{
    return rw;
}

/*! Returns the column where the table item is located. If the cell spans
  multiple columns, this function returns the left-most column.

  \sa row() setCol()
*/

int QTableItem::col() const
{
    return cl;
}

/*! If \a b is TRUE, the table item is enabled, otherwise it is disabled.

  A disabled item doesn't respond to user interaction.

  \sa isEnabled()
*/

void QTableItem::setEnabled( bool b )
{
    if ( b == (bool)enabled )
	return;
    enabled = b;
    table()->updateCell( row(), col() );
}

/*! Returns TRUE if the table item is enabled; otherwise returns FALSE.

  \sa setEnabled()
*/

bool QTableItem::isEnabled() const
{
    return (bool)enabled;
}

/*! \class QComboTableItem qtable.h

  \brief The QComboTableItem class provides a means of using comboboxes
  in QTables.

  \module table

  A QComboTableItem is a table item which looks and behaves like a
  combobox. The advantage of using QComboTableItems rather than real
  comboboxes is that a QComboTableItem uses far less resources than a real
  combobox. When the cell has the focus it displays a real combobox
  which the user can interact with. When the cell does not have the
  focus the cell \e looks like a combobox. Only strings (i.e. no
  pixmaps) may be used in QComboTableItems.

  QComboTableItem items have the edit type \c WhenCurrent (see
  \l{EditType}). The QComboTableItem's list of items is provided by a
  QStringList passed to the constructor.

  The list of items may be changed using setStringList(). The current
  item can be set with setCurrentItem() and retrieved with
  currentItem(). The text of the current item can be obtained with
  currentText(), and the text of a particular item can be retrieved with
  text().

  If isEditable() is TRUE the QComboTableItem will permit the user to
  either choose an existing list item or create a new list item by
  entering their own text; otherwise the user may only choose one of the
  existing list items.

  To populate a table cell with a QComboTableItem use QTable::setItem():

  \walkthrough table/small-table-demo/main.cpp
  \skipto comboEntries
  \printuntil four
  \skipto for
  \printuntil TRUE
  \skipto setItem
  \printuntil }
  (Code from \link small-table-demo-example.html
  table/small-table-demo/main.cpp \endlink).

  QComboTableItems may be deleted with QTable::clearCell().

  QComboTableItems can be distinguished from \l{QTableItem}s and
  \l{QCheckTableItem}s using their Run Time Type Identification number
  (see rtti()).
*/

/*! Creates a combo table item for the table \a table. The combobox's
    list of items is passed in the \a list argument. If \a editable is
    TRUE the user may type in new list items; if \a editable is FALSE
    the user may only select from the list of items provided.

  By default QComboTableItems cannot be replaced by other table items
  since isReplaceable() returns FALSE by default.

  \sa QTable::clearCell() EditType
*/

QComboTableItem::QComboTableItem( QTable *table, const QStringList &list, bool editable )
    : QTableItem( table, WhenCurrent, "" ), entries( list ), current( 0 ), edit( editable )
{
    setReplaceable( FALSE );
}

/*! Sets the list items of this QComboTableItem to the strings
   in the string list \a l.
*/

void QComboTableItem::setStringList( const QStringList &l )
{
    entries = l;
    current = 0;
    table()->updateCell( row(), col() );
}

/*! \reimp */

QWidget *QComboTableItem::createEditor() const
{
    // create an editor - a combobox in our case
    ( (QComboTableItem*)this )->cb = new QComboBox( edit, table()->viewport() );
    cb->insertStringList( entries );
    cb->setCurrentItem( current );
    QObject::connect( cb, SIGNAL( activated( int ) ), table(), SLOT( doValueChanged() ) );
    return cb;
}

/*! \reimp */

void QComboTableItem::setContentFromEditor( QWidget *w )
{
    if ( w->inherits( "QComboBox" ) ) {
	QComboBox *cb = (QComboBox*)w;
	entries.clear();
	for ( int i = 0; i < cb->count(); ++i )
	    entries << cb->text( i );
	current = cb->currentItem();
	setText( currentText() );
    }
}

/*! \reimp */

void QComboTableItem::paint( QPainter *p, const QColorGroup &cg,
			   const QRect &cr, bool selected )
{
    int w = cr.width();
    int h = cr.height();

    // table()->style().drawComboButton( p, 0, 0, w, h, cg, FALSE, TRUE, TRUE, selected ? &cg.brush( QColorGroup::Highlight ) : 0  );
    QRect tmpR(0, 0, w, h); // = table()->style().comboButtonRect( 0, 0, w, h );
    QRect textR( tmpR.x() + 1, tmpR.y() + 1, tmpR.width() - 2, tmpR.height() - 2 );

    if ( selected )
	p->setPen( cg.highlightedText() );
    else
	p->setPen( cg.text() );
    p->drawText( textR, wordWrap() ? ( alignment() | WordBreak ) : alignment(), currentText() );
}

/*!
    Sets list item \a i to be the combo table item's current list
    item.

  \sa currentItem()
*/

void QComboTableItem::setCurrentItem( int i )
{
    current = i;
    setText( currentText() );
    table()->updateCell( row(), col() );
}

/*! \overload

    Sets the list item whose text is \a s to be the combo table
    item's current list item. Does nothing if no list item has text \a
    s.

  \sa currentItem()
*/

void QComboTableItem::setCurrentItem( const QString &s )
{
    int i = entries.findIndex( s );
    if ( i != -1 )
	setCurrentItem( i );
}

/*!
    Returns the index of the combo table item's current list item.

  \sa setCurrentItem()
*/

int QComboTableItem::currentItem() const
{
    return current;
}

/*!
    Returns the text of the combo table item's current list item.

  \sa currentItem() text()
*/

QString QComboTableItem::currentText() const
{
    return *entries.at( current );
}

/*! Returns the total number of list items in the combo table item.
*/

int QComboTableItem::count() const
{
    return entries.count();
}

/*!
    Returns the text of the combo's list item at index \a i.

  \sa currentText()
*/

QString QComboTableItem::text( int i ) const
{
    return *entries.at( i );
}

/*!
    If \a b is TRUE the combo table item can be edited, i.e. the user
    may enter a new text item themselves. If \a b is FALSE the user may
    may only choose one of the existing items.

  \sa isEditable()
*/

void QComboTableItem::setEditable( bool b )
{
    edit = b;
}

/*! Returns whether the user may add their own list items to the
    combo's list of items.

  \sa setEditable()
*/

bool QComboTableItem::isEditable() const
{
    return edit;
}

int QComboTableItem::RTTI = 1;

/*! \fn int QComboTableItem::rtti() const

  For QComboTableItems this function returns a Run Time Identification
  number of 1.

  \sa QTableItem::rtti()
*/

int QComboTableItem::rtti() const
{
    return RTTI;
}

/*! \class QCheckTableItem qtable.h

  \brief The QCheckTableItem class provides checkboxes in QTables.

  \module table

  A QCheckTableItem is a table item which looks and behaves like a
  checkbox. The advantage of using QCheckTableItems rather than real
  checkboxes is that a QCheckTableItem uses far less resources than a real
  checkbox. When the cell has the focus it displays a real checkbox
  which the user can interact with. When the cell does not have the
  focus the cell \e looks like a checkbox. Pixmaps may not be used in
  QCheckTableItems.

    QCheckTableItem items have the edit type \c WhenCurrent (see
    \l{EditType}).

  To change the checkbox's label use setText(). The checkbox can be
  checked and unchecked with setChecked() and its state retrieved using
  isChecked().

  To populate a table cell with a QCheckTableItem use QTable::setItem():

  \walkthrough table/small-table-demo/main.cpp
  \skipto for ( int j
  \printuntil Check me
  (Code from \link small-table-demo-example.html
  table/small-table-demo/main.cpp \endlink).

  QCheckTableItems can be distinguished from \l{QTableItem}s and
  \l{QComboTableItem}s using their Run Time Type Identification (rtti)
  number.

  \sa rtti() EditType
*/

/*! Creates a QCheckTableItem with an \l{EditType} of \c WhenCurrent
  as a child of \a table. The checkbox is initially unchecked and its
  label is set to the string \a txt.

*/

QCheckTableItem::QCheckTableItem( QTable *table, const QString &txt )
    : QTableItem( table, WhenCurrent, txt ), checked( FALSE )
{
}

/*! \reimp */

QWidget *QCheckTableItem::createEditor() const
{
    // create an editor - a combobox in our case
    ( (QCheckTableItem*)this )->cb = new QCheckBox( table()->viewport() );
    cb->setChecked( checked );
    cb->setText( text() );
    cb->setBackgroundMode( table()->viewport()->backgroundMode() );
    QObject::connect( cb, SIGNAL( toggled( bool ) ), table(), SLOT( doValueChanged() ) );
    return cb;
}

/*! \reimp */

void QCheckTableItem::setContentFromEditor( QWidget *w )
{
    if ( w->inherits( "QCheckBox" ) ) {
	QCheckBox *cb = (QCheckBox*)w;
	checked = cb->isChecked();
    }
}

/*! \reimp */

void QCheckTableItem::paint( QPainter *p, const QColorGroup &cg,
				const QRect &cr, bool selected )
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
		 selected ? cg.brush( QColorGroup::Highlight )
			  : cg.brush( QColorGroup::Base ) );

//     int w = cr.width();
//     int h = cr.height();
//     QSize sz = table()->style().indicatorSize();
//     table()->style().drawIndicator( p, 0, ( h - sz.height() ) / 2, sz.width(), sz.height(), cg, checked ? QButton::On : QButton::Off, FALSE, TRUE );
//     int x = sz.width() + 6;
//     w = w - x;
//     if ( selected )
// 	p->setPen( cg.highlightedText() );
//     else
// 	p->setPen( cg.text() );
//     p->drawText( x, 0, w, h, wordWrap() ? ( alignment() | WordBreak ) : alignment(), text() );
}

/*!
    If \a b is TRUE the checkbox is checked; if \a b is FALSE the
    checkbox is unchecked.

  \sa isChecked()
*/

void QCheckTableItem::setChecked( bool b )
{
    checked = b;
    table()->updateCell( row(), col() );
}

/*! Returns TRUE if the checkbox table item is checked; otherwise
    returns FALSE.

  \sa setChecked()
*/

bool QCheckTableItem::isChecked() const
{
    return checked;
}

int QCheckTableItem::RTTI = 2;

/*! \fn int QCheckTableItem::rtti() const

    Returns 2.

  Make your derived classes return their own values for rtti()to
  distinguish between different table item subclasses. You should use
  values greater than 1000 preferably a large random number, to allow
  for extensions to this class.

  \sa QTableItem::rtti()
*/

int QCheckTableItem::rtti() const
{
    return RTTI;
}

/*! \file table/small-table-demo/main.cpp */
/*! \file table/bigtable/main.cpp */
/*! \file statistics/statistics.cpp */

/*! \class QTable qtable.h
  \module table

  \brief The QTable class provides a flexible editable table widget.

    QTable is easy to use, although it does have a large API because of
    the comprehensive functionality that it provides. QTable includes
    functions for manipulating <a href="#headers">headers</a>, <a
    href="#columnsrows">rows and columns</a>, <a href="#cells">cells</a>
    and <a href="#selections">selections</a>. QTable also provides
    in-place editing and <a href="dnd.html">drag and drop</a>, as well
    as a useful set of <a href="#signals">signals</a>. QTable
    efficiently supports very large tables, for example, tables one
    million by one million cells are perfectly possible. QTable is
    economical with memory, using none for unused cells.

    \code
    QTable *table = new QTable( 100, 250, this );
    table->setPixmap( 3, 2, pix );
    table->setText( 3, 2, "A pixmap" );
    \endcode

    The first line constructs the table specifying its size in rows and
    columns. We then insert a pixmap and some text into the \e same
    cell, with the pixmap appearing to the left of the text. By default
    a vertical header appears at the left of the table showing row
    numbers and a horizontal header appears at the top of the table
    showing column numbers. (The numbers displayed start at 1, although
    row and column numbers within QTable begin at 0.)

    <a name="headers"><h4>Headers</h4>
    QTable supports a header column, e.g. to display row numbers, and a
    header row, e.g to display column titles. To set row or column
    labels use QHeader::setLabel() on the pointers returned by
    verticalHeader() and horizontalHeader() respectively. The vertical
    header is displayed within the table's left margin whose width is
    set with setLeftMargin(). The horizontal header is displayed within
    the table's top margin whose height is set with setTopMargin(). The
    table's grid can be switched off with setShowGrid().

    <a name="columnsrows"><h4>Rows and Columns</h4>
    Row and column sizes are set with setRowHeight() and
    setColumnWidth().  If you want a row high enough to show the tallest
    item in its entirety, use adjustRow(). Similarly, to make a column
    wide enough to show the widest item use adjustColumn(). If you want
    the row height and column width to adjust automatically as the
    height and width of the table changes use setRowStretchable() and
    setColumnStretchable().

    Rows and columns can be hidden and shown with hideRow(),
    hideColumn(), showRow() and showColumn().  New rows and columns are
    inserted using insertRows() and insertColumns(). Additional rows and
    columns are added at the  bottom (rows) or right (columns) if you
    set setNumRows() or setNumCols() to be larger than numRows() or
    numCols(). Existing rows and columns are removed with removeRow()
    and removeColumn(). Multiple rows and columns can be removed with
    removeRows() and removeColumns().

    Rows and columns can be set to be moveable, i.e. the user can drag
    them to reorder them, using rowMovingEnabled() and
    columnMovingEnabled(). The table can be sorted using sortColumn().
    Users can sort a column by clicking its header if setSorting() is
    set to TRUE. Rows can be swapped with swapRows(), columns with
    swapColumns() and cells with swapCells().

    For editable tables (see setReadOnly()) you can set the read-only
    property of individual rows and columns with setRowReadOnly() and
    setColumnReadOnly(). (Whether a cell is editable or read-only
    depends on these settings and the cell's <a
    href="qtableitem.html#wheneditable">QTableItem::EditType</a>.)

    The row and column which have the focus are returned by
    currentRow() and currentColumn() respectively.

    Although many QTable functions operate in terms of rows and columns
    the indexOf() function returns a single integer identifying a
    particular cell.

    <a name="cells"><h4>Cells</h4>
    All of a QTable's cells are empty when the table is constructed.

    There are two approaches to populating the table's cells. The first
    and simplest approach is to use QTableItems or QTableItem
    subclasses. The second approach doesn't use QTableItems at all which
    is useful for very large sparse tables but requires you to
    reimplement a number of functions. We'll look at each approach in
    turn.

    To put a string in a cell use setText(). This function will create a
    new QTableItem for the cell if one doesn't already exist, and
    displays the text in it. By default the table item's widget will be a
    QLineEdit. A pixmap may be put in a cell with setPixmap(), which
    also creates a table item if required. A cell may contain \e both a
    pixmap and text; the pixmap is displayed to the left of the text.
    Another approach is to construct a QTableItem or QTableItem
    subclass, set its properties, then insert it into a cell with
    setItem().

    If you want cells which contain comboboxes use the
    QComboTableItem class. Similarly if you require cells containing
    checkboxes use the QCheckTableItem class. These table items look and
    behave just like the combobox or checkbox widgets but consume far
    less memory.

    \walkthrough table/small-table-demo/main.cpp
    \skipto int j
    \printuntil QCheckTableItem
    In the example above we create a column of QCheckTableItems and
    insert them into the table using setItem().

    QTable takes ownership of its QTableItems and will delete them when
    the table itself is destroyed. You can take ownership of a table
    item using takeItem() which you use to move a cell's contents from
    one cell to another, either within the same table, or from one table
    to another. (See also, swapCells()).

    In-place editing of the text in QTableItems, and the values in
    QComboTableItems and QCheckTableItems works automatically. Cells may
    be editable or read-only,
    see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

    The contents of a cell can be retrieved as a QTableItem using
    item(), or as a string with text() or as a pixmap (if there is one)
    with pixmap(). A cell's bounding rectangle is given by
    cellGeometry(). Use updateCell() to repaint a cell, for example to
    clear away a cell's visual representation after it has been deleted
    with clearCell(). The table can be forced to scroll to show a
    particular cell with ensureCellVisible(). The isSelected() function
    indicates if a cell is selected.

    It is possible to use your own widget as a cell's widget using
    setCellWidget(), but subclassing QTableItem might be a simpler
    approach. The cell's widget (if there is one) can be removed with
    clearCellWidget().

    <a name="bigtables">
    <h5>Large tables</h5>
    For large, sparse, tables using QTableItems or other widgets is
    inefficient. The solution is to \e draw the cell as it should appear
    and to create and destroy cell editors on demand.

    This approach requires that you reimplement various functions.
    Reimplement paintCell() to display your data, and createEditor() and
    setCellContentFromEditor() to facilitate in-place editing. It is
    very important to reimplement resizeData() to have no functionality,
    to prevent QTable from attempting to create a huge array. You will
    also need to reimplement item(), setItem(), clearCell(), and
    insertWidget(), cellWidget() and clearCellWidget(). If you wish to
    support sorting you should also reimplement swapRows(), swapCells()
    and possibly swapColumns().

    If you represent active cells with a dictionary of QTableItems and
    QWidgets, i.e. only store references to cells that are actually
    used, most of the functions can be implemented with a single line of
    code. (See the \l table/bigtable/main.cpp example.)

    For more information on cells see the QTableItem documenation.

    <a name="selections"><h4>Selections</h4>

    QTable's support single selection, multi-selection (multiple cells)
    or no selection. The selection mode is set with setSelectionMode().
    Use isSelected() to determine if a particular cell is selected, and
    isRowSelected() and isColumnSelected() to see if a row or column is
    selected.

    QTable's support multiple selections. You can programmatically
    select cells with addSelection(). The number of selections is given
    by numSelections(). The current selection is returned by
    currentSelection(). You can remove a selection with
    removeSelection() and remove all selections with clearSelection().
    Selections are QTableSelection objects.

    <a name="signals"><h4>Signals</h4>

    When the user clicks a cell the currentChanged() signal is emitted.
    You can also connect to the lower level clicked(), doubleClicked()
    and pressed() signals. If the user changes the selection the
    selectionChanged() signal is emitted; similarly if the user changes
    a cell's value the valueChanged() signal is emitted. If the
    user right-clicks (or presses the platform-specific key sequence)
    the contextMenuRequested() signal is emitted. If the user drops a
    drag and drop object the dropped() signal is emitted with the drop
    event.
*/

/*! \fn void QTable::currentChanged( int row, int col )

  This signal is emitted when the current cell has changed to \a
  row, \a col.
*/

/*! \fn void QTable::valueChanged( int row, int col )

  This signal is emitted when the user changed the value in the cell at
  \a row, \a col.
*/

/*! \fn int QTable::currentRow() const

  Returns the current row.

  \sa currentColumn()
*/

/*! \fn int QTable::currentColumn() const

  Returns the current column.

  \sa currentRow()
*/

/*! \enum QTable::EditMode

  \value NotEditing  No cell is currently being edited.
  \value Editing  A cell is currently being edited. The editor was
  initialised with the cell's contents.
  \value Replacing  A cell is currently being edited. The editor was
  not initialised with the cell's contents.
*/

/*! \enum QTable::SelectionMode

  \value NoSelection  No cell can be selected by the user.
  \value Single  The user may only select a single range of cells.
  \value Multi  The user may select multiple ranges of cells.
*/

/*! \fn void QTable::clicked( int row, int col, int button, const QPoint &mousePos )

  This signal is emitted when mouse button \a button is clicked. The
  cell where the event took place is at \a row, \a col, and
  the mouse's position is in \a mousePos.
*/

/*! \fn void QTable::doubleClicked( int row, int col, int button, const QPoint &mousePos )

  This signal is emitted when mouse button \a button is double-clicked.
  The cell where the event took place is at \a row, \a col,
  and the mouse's position is in \a mousePos.
*/

/*! \fn void QTable::pressed( int row, int col, int button, const QPoint &mousePos )

  This signal is emitted when mouse button \a button is pressed. The
  cell where the event took place is at \a row, \a col, and
  the mouse's position is in \a mousePos.
*/

/*! \fn void QTable::selectionChanged()

  This signal is emitted whenever a selection changes.

  \sa QTableSelection
*/

/*!
  \fn void QTable::contextMenuRequested( int row, int col, const QPoint & pos )

  This signal is emitted when the user invokes a context menu with the
  right mouse button (or with a system-specific keyboard key). The
  cell where the event took place is at \a row, \a col.
  \a pos is the position where the context menu will appear in the
  global coordinate system.
*/

/*! Creates an empty table object called \a name as a child of \a parent.

  Call setNumRows() and setNumCols() to set the table size before
  populating the table if you're using QTableItems.

  \sa QWidget::clearWFlags() Qt::WidgetFlags
*/

QTable::QTable( QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity ),
      currentSel( 0 ), lastSortCol( -1 ), sGrid( TRUE ), mRows( FALSE ), mCols( FALSE ),
      asc( TRUE ), doSort( TRUE ), readOnly( FALSE )
{
    init( 0, 0 );
}

/*! Constructs an empty table called \a name with \a numRows rows and \a
 numCols columns. The table is a child of \a parent.

  If you're using QTableItems to populate the table's cells, you can
  create QTableItem, QComboTableItem and QCheckTableItem items and
  insert them into the table using setItem(). (See the notes on <a
  href="#bigtables">large tables</a> for an alternative to using
  QTableItems.)

  \sa QWidget::clearWFlags() Qt::WidgetFlags
*/

QTable::QTable( int numRows, int numCols, QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity ),
      currentSel( 0 ), lastSortCol( -1 ), sGrid( TRUE ), mRows( FALSE ), mCols( FALSE ),
      asc( TRUE ), doSort( TRUE ), readOnly( FALSE )
{
    init( numRows, numCols );
}

/*! \internal
*/

void QTable::init( int rows, int cols )
{
#ifndef QT_NO_DRAGANDDROP
    setDragAutoScroll( FALSE );
#endif
    d = 0;
    shouldClearSelection = FALSE;
    dEnabled = FALSE;
    roRows.setAutoDelete( TRUE );
    roCols.setAutoDelete( TRUE );
    setSorting( FALSE );

    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( WheelFocus );
    mousePressed = FALSE;

    selMode = Multi;

    contents.setAutoDelete( TRUE );
    widgets.setAutoDelete( TRUE );

    // Enable clipper and set background mode
    enableClipper( TRUE );
    viewport()->setBackgroundMode( PaletteBase );
    setBackgroundMode( PaletteBackground, PaletteBase );
    setResizePolicy( Manual );
    selections.setAutoDelete( TRUE );

    // Create headers
    leftHeader = new QTableHeader( rows, this, this, "left table header" );
    leftHeader->setOrientation( Vertical );
    leftHeader->setTracking( TRUE );
    leftHeader->setMovingEnabled( TRUE );
    topHeader = new QTableHeader( cols, this, this, "right table header" );
    topHeader->setOrientation( Horizontal );
    topHeader->setTracking( TRUE );
    topHeader->setMovingEnabled( TRUE );
    setMargins( 30, fontMetrics().height() + 4, 0, 0 );

    topHeader->setUpdatesEnabled( FALSE );
    leftHeader->setUpdatesEnabled( FALSE );
    // Initialize headers
    int i = 0;
    for ( i = 0; i < numCols(); ++i )
	topHeader->resizeSection( i, 100 );
    for ( i = 0; i < numRows(); ++i )
	leftHeader->resizeSection( i, 20 );
    topHeader->setUpdatesEnabled( TRUE );
    leftHeader->setUpdatesEnabled( TRUE );

    // Prepare for contents
    contents.setAutoDelete( FALSE );

    // Connect header, table and scrollbars
    connect( horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
	     topHeader, SLOT( setOffset( int ) ) );
    connect( verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     leftHeader, SLOT( setOffset( int ) ) );
    connect( topHeader, SIGNAL( sectionSizeChanged( int ) ),
	     this, SLOT( columnWidthChanged( int ) ) );
    connect( topHeader, SIGNAL( indexChange( int, int, int ) ),
	     this, SLOT( columnIndexChanged( int, int, int ) ) );
    connect( topHeader, SIGNAL( sectionClicked( int ) ),
	     this, SLOT( columnClicked( int ) ) );
    connect( leftHeader, SIGNAL( sectionSizeChanged( int ) ),
	     this, SLOT( rowHeightChanged( int ) ) );
    connect( leftHeader, SIGNAL( indexChange( int, int, int ) ),
	     this, SLOT( rowIndexChanged( int, int, int ) ) );

    // Initialize variables
    autoScrollTimer = new QTimer( this );
    connect( autoScrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    curRow = curCol = 0;
    edMode = NotEditing;
    editRow = editCol = -1;

    context_menu = FALSE;

    installEventFilter( this );

    // Initial size
    resize( 640, 480 );
}

/*!  Destructor. Deletes all the resources used by a QTable object,
 including all QTableItems and their widgets.
*/

QTable::~QTable()
{
    delete d;
    contents.setAutoDelete( TRUE );
    contents.clear();
    widgets.clear();
}

void QTable::setReadOnly( bool b )
{
    readOnly = b;
}

/*!
    If \a ro is TRUE, row \a row is set to be read-only; otherwise the
    row is set to be editable.

    Whether a cell in this row is editable or read-only depends on the
    cell's EditType, and this setting:
    see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

  \sa isRowReadOnly() setColumnReadOnly() setReadOnly()
*/

void QTable::setRowReadOnly( int row, bool ro )
{
    if ( ro )
	roRows.replace( row, new int( 0 ) );
    else
	roRows.remove( row );
}

/*!
    If \a ro is TRUE, column \a col is set to be read-only; otherwise
    the column is set to be editable.

    Whether a cell in this column is editable or read-only depends on
    the cell's EditType, and this setting:
    see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

    \sa isColumnReadOnly() setRowReadOnly() setReadOnly()

*/

void QTable::setColumnReadOnly( int col, bool ro )
{
    if ( ro )
	roCols.replace( col, new int( 0 ) );
    else
	roCols.remove( col );
}

/*! \property QTable::readOnly
  \brief whether the table is read-only

  Whether a cell in the table is editable or read-only depends on the
  cell's EditType, and this setting:
  see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

  \sa QWidget::enabled setColumnReadOnly() setRowReadOnly()
*/

bool QTable::isReadOnly() const
{
    return readOnly;
}

/*! Returns whether row \a row is read-only.

  Whether a cell in this row is editable or read-only depends on the
  cell's EditType, and this setting:
  see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

  \sa setRowReadOnly isColumnReadOnly()
*/

bool QTable::isRowReadOnly( int row ) const
{
    return (roRows.find( row ) != 0);
}

/*! Returns whether column \a col is read-only.

  Whether a cell in this column is editable or read-only depends on the
  cell's EditType, and this setting:
  see <a href="qtableitem.html#wheneditable">QTableItem::EditType</a>.

  \sa setColumnReadOnly() isRowReadOnly()
*/

bool QTable::isColumnReadOnly( int col ) const
{
    return (roCols.find( col ) != 0);
}

/*! Sets the table's selection mode to \a mode. The default mode is \c
    Multi which allows the user to select multiple ranges of cells.

  \sa SelectionMode selectionMode()
*/

void QTable::setSelectionMode( SelectionMode mode )
{
    selMode = mode;
}

/*! Returns the current selection mode.

  \sa SelectionMode setSelectionMode()
*/

QTable::SelectionMode QTable::selectionMode() const
{
    return selMode;
}

/*! Returns the table's top QHeader.

    This header contains the column labels.

  To modify a column label use QHeader::setLabel(), e.g.
  \walkthrough statistics/statistics.cpp
  \skipto horizontalHeader
  \printline

  \sa verticalHeader() setTopMargin() QHeader
*/

QHeader *QTable::horizontalHeader() const
{
    return (QHeader*)topHeader;
}

/*! Returns the table's left QHeader.

    This header contains the row labels.

  \sa horizontalHeader() setLeftMargin() QHeader
*/

QHeader *QTable::verticalHeader() const
{
    return (QHeader*)leftHeader;
}

void QTable::setShowGrid( bool b )
{
    if ( sGrid == b )
	return;
    sGrid = b;
    viewport()->repaint( FALSE );
}

/*! \property QTable::showGrid
  \brief whether the table's grid is displayed

  The grid is shown by default.
*/

bool QTable::showGrid() const
{
    return sGrid;
}

/*! \property QTable::columnMovingEnabled
 \brief whether columns can be moved by the user

  \sa rowMovingEnabled
*/

void QTable::setColumnMovingEnabled( bool b )
{
    mCols = b;
}

bool QTable::columnMovingEnabled() const
{
    return mCols;
}

/*! \property QTable::rowMovingEnabled
 \brief whether rows can be moved by the user

  \sa columnMovingEnabled
*/

void QTable::setRowMovingEnabled( bool b )
{
    mRows = b;
}

bool QTable::rowMovingEnabled() const
{
    return mRows;
}

/*! This is called when QTable's internal array needs to be resized
  to \a len elements.

  If you don't use QTableItems you should reimplement this as an empty
  method to avoid wasting memory.
  See the notes on <a href="#bigtables">large tables</a> for further details.
*/

void QTable::resizeData( int len )
{
    contents.resize( len );
}

/*! Swaps data of \a row1 and \a row2.

    This function is used to swap the positions of two rows. It is
    called when the user changes the order of rows (see
    setRowMovingEnabled()), and when rows are sorted.

  If you don't use QTableItems and want your users to be able to swap
  rows, e.g. for sorting, you will need to reimplement this function.
  (See the notes on <a href="#bigtables">large tables</a>.)

  \sa swapColumns() swapCells()
*/

void QTable::swapRows( int row1, int row2 )
{
    QPtrVector<QTableItem> tmpContents;
    tmpContents.resize( numCols() );
    QPtrVector<QWidget> tmpWidgets;
    tmpWidgets.resize( numCols() );
    int i;

    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( FALSE );
    for ( i = 0; i < numCols(); ++i ) {
	QTableItem *i1, *i2;
	i1 = item( row1, i );
	i2 = item( row2, i );
	if ( i1 || i2 ) {
	    tmpContents.insert( i, i1 );
	    contents.remove( indexOf( row1, i ) );
	    contents.insert( indexOf( row1, i ), i2 );
	    contents.remove( indexOf( row2, i ) );
	    contents.insert( indexOf( row2, i ), tmpContents[ i ] );
	    if ( contents[ indexOf( row1, i ) ] )
		contents[ indexOf( row1, i ) ]->setRow( row1 );
	    if ( contents[ indexOf( row2, i ) ] )
		contents[ indexOf( row2, i ) ]->setRow( row2 );
	}

	QWidget *w1, *w2;
	w1 = cellWidget( row1, i );
	w2 = cellWidget( row2, i );
	if ( w1 || w2 ) {
	    tmpWidgets.insert( i, w1 );
	    widgets.remove( indexOf( row1, i ) );
	    widgets.insert( indexOf( row1, i ), w2 );
	    widgets.remove( indexOf( row2, i ) );
	    widgets.insert( indexOf( row2, i ), tmpWidgets[ i ] );
	}
    }
    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( TRUE );

    updateRowWidgets( row1 );
    updateRowWidgets( row2 );
    if ( curRow == row1 )
	curRow = row2;
    else if ( curRow == row2 )
	curRow = row1;
    if ( editRow == row1 )
	editRow = row2;
    else if ( editRow == row2 )
	editRow = row1;
}

/*! Sets the left margin to be \a m pixels wide.

    The verticalHeader(), which displays row labels, occupies this
    margin.

  \sa leftMargin() setTopMargin() verticalHeader()
*/

void QTable::setLeftMargin( int m )
{
    setMargins( m, topMargin(), rightMargin(), bottomMargin() );
    updateGeometries();
}

/*! Sets the top margin to be \a m pixels high.

    The horizontalHeader(), which displays column labels, occupies this
    margin.

  \sa topMargin() setLeftMargin()
*/

void QTable::setTopMargin( int m )
{
    setMargins( leftMargin(), m, rightMargin(), bottomMargin() );
    updateGeometries();
}

/*! Exchanges \a col1 with \a col2.

    This function is used to swap the positions of two columns. It is
    called when the user changes the order of columns (see
    setColumnMovingEnabled(), and when columns are sorted.

    If you don't use QTableItems and want your users to be able to swap
    columns you will need to reimplement this function.
  (See the notes on <a href="#bigtables">large tables</a>.)

  \sa swapCells()
*/

void QTable::swapColumns( int col1, int col2 )
{
    QPtrVector<QTableItem> tmpContents;
    tmpContents.resize( numRows() );
    QPtrVector<QWidget> tmpWidgets;
    tmpWidgets.resize( numRows() );
    int i;

    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( FALSE );
    for ( i = 0; i < numRows(); ++i ) {
	QTableItem *i1, *i2;
	i1 = item( i, col1 );
	i2 = item( i, col2 );
	if ( i1 || i2 ) {
	    tmpContents.insert( i, i1 );
	    contents.remove( indexOf( i, col1 ) );
	    contents.insert( indexOf( i, col1 ), i2 );
	    contents.remove( indexOf( i, col2 ) );
	    contents.insert( indexOf( i, col2 ), tmpContents[ i ] );
	    if ( contents[ indexOf( i, col1 ) ] )
		contents[ indexOf( i, col1 ) ]->setCol( col1 );
	    if ( contents[ indexOf( i, col2 ) ] )
		contents[ indexOf( i, col2 ) ]->setCol( col2 );
	}

	QWidget *w1, *w2;
	w1 = cellWidget( i, col1 );
	w2 = cellWidget( i, col2 );
	if ( w1 || w2 ) {
	    tmpWidgets.insert( i, w1 );
	    widgets.remove( indexOf( i, col1 ) );
	    widgets.insert( indexOf( i, col1 ), w2 );
	    widgets.remove( indexOf( i, col2 ) );
	    widgets.insert( indexOf( i, col2 ), tmpWidgets[ i ] );
	}
    }
    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( TRUE );

    columnWidthChanged( col1 );
    columnWidthChanged( col2 );
    if ( curCol == col1 )
	curCol = col2;
    else if ( curCol == col2 )
	curCol = col1;
    if ( editCol == col1 )
	editCol = col2;
    else if ( editCol == col2 )
	editCol = col1;
}

/*! Swaps the contents of the cell at \a row1, \a col1 with
 the contents of the cell at \a row2, \a col2.

    This function is also called when the table is sorted.

    If you don't use QTableItems and want your users to be able to swap
    cells, you will need to reimplement this function.
  (See the notes on <a href="#bigtables">large tables</a>.)

  \sa swapColumns() swapRows()
*/

void QTable::swapCells( int row1, int col1, int row2, int col2 )
{
    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( FALSE );
    QTableItem *i1, *i2;
    i1 = item( row1, col1 );
    i2 = item( row2, col2 );
    if ( i1 || i2 ) {
	QTableItem *tmp = i1;
	contents.remove( indexOf( row1, col1 ) );
	contents.insert( indexOf( row1, col1 ), i2 );
	contents.remove( indexOf( row2, col2 ) );
	contents.insert( indexOf( row2, col2 ), tmp );
	if ( contents[ indexOf( row1, col1 ) ] ) {
	    contents[ indexOf( row1, col1 ) ]->setRow( row1 );
	    contents[ indexOf( row1, col1 ) ]->setCol( col1 );
	}
	if ( contents[ indexOf( row2, col2 ) ] ) {
	    contents[ indexOf( row2, col2 ) ]->setRow( row2 );
	    contents[ indexOf( row2, col2 ) ]->setCol( col2 );
	}
    }

    QWidget *w1, *w2;
    w1 = cellWidget( row1, col1 );
    w2 = cellWidget( row2, col2 );
    if ( w1 || w2 ) {
	QWidget *tmp = w1;
	widgets.remove( indexOf( row1, col1 ) );
	widgets.insert( indexOf( row1, col1 ), w2 );
	widgets.remove( indexOf( row2, col2 ) );
	widgets.insert( indexOf( row2, col2 ), tmp );
    }

    updateRowWidgets( row1 );
    updateRowWidgets( row2 );
    updateColWidgets( col1 );
    updateColWidgets( col2 );
    contents.setAutoDelete( FALSE );
    widgets.setAutoDelete( TRUE );
}

/*! Draws the table contents on the painter \a p. This function is
  optimized so that it only draws the cells inside the \a cw pixels wide
  and \a ch pixels high clipping rectangle at position \a cx, \a cy.

  Additionally, drawContents() highlights the current cell.
*/

void QTable::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    int colfirst = columnAt( cx );
    int collast = columnAt( cx + cw );
    int rowfirst = rowAt( cy );
    int rowlast = rowAt( cy + ch );

    if ( rowfirst == -1 || colfirst == -1 ) {
	paintEmptyArea( p, cx, cy, cw, ch );
	return;
    }

    if ( rowlast == -1 )
	rowlast = numRows() - 1;
    if ( collast == -1 )
	collast = numCols() - 1;

    // Go through the rows
    for ( int r = rowfirst; r <= rowlast; ++r ) {
	// get row position and height
	int rowp = rowPos( r );
	int rowh = rowHeight( r );

	// Go through the columns in row r
	// if we know from where to where, go through [colfirst, collast],
	// else go through all of them
	for ( int c = colfirst; c <= collast; ++c ) {
	    // get position and width of column c
	    int colp, colw;
	    colp = columnPos( c );
	    colw = columnWidth( c );
	    int oldrp = rowp;
	    int oldrh = rowh;

	    QTableItem *itm = item( r, c );
	    if ( itm &&
		 ( itm->colSpan() > 1 || itm->rowSpan() > 1 ) ) {
		bool goon = r == itm->row() && c == itm->col() ||
			r == rowfirst && c == itm->col() ||
			r == itm->row() && c == colfirst;
		if ( !goon )
		    continue;
		rowp = rowPos( itm->row() );
		rowh = 0;
		int i;
		for ( i = 0; i < itm->rowSpan(); ++i )
		    rowh += rowHeight( i + itm->row() );
		colp = columnPos( itm->col() );
		colw = 0;
		for ( i = 0; i < itm->colSpan(); ++i )
		    colw += columnWidth( i + itm->col() );
	    }

	    // Translate painter and draw the cell
	    p->translate( colp, rowp );
	    paintCell( p, r, c, QRect( colp, rowp, colw, rowh ), isSelected( r, c ) );
	    p->translate( -colp, -rowp );

	    rowp = oldrp;
	    rowh = oldrh;

	    QWidget *w = cellWidget( r, c );
	    if ( w && w->geometry() !=
		 QRect( contentsToViewport( QPoint( colp, rowp ) ), QSize( colw - 1, rowh - 1 ) ) ) {
		moveChild( w, colp, rowp );
		w->resize( colw - 1, rowh - 1 );
	    }
	}
    }

    // draw indication of current cell
    QRect focusRect = cellGeometry( curRow, curCol );
    p->translate( focusRect.x(), focusRect.y() );
    paintFocus( p, focusRect );
    p->translate( -focusRect.x(), -focusRect.y() );

    // Paint empty rects
    paintEmptyArea( p, cx, cy, cw, ch );
}

/*! Paints the cell at \a row, \a col on the painter \a
  p. The painter has already been translated to the cell's origin. \a
  cr describes the cell coordinates in the content coordinate system.

  If \a selected is TRUE the cell is highlighted.

  If you want to draw custom cell content, for example right-aligned
  text, you must either reimplement paintCell(), or subclass QTableItem
  and reimplement QTableItem::paint() to do the custom drawing.

    If you're using a QTableItem subclass, for example, to store a data
    structure, then reimplementing QTableItem::paint() may be the best
    approach. For data you want to draw immediately, e.g. data retrieved
    from a database, it is probably best to reimplement paintCell().
    Note that if you reimplement paintCell, i.e. don't use QTableItems,
    you will have to reimplement other functions: see the notes on <a
    href="#bigtables">large tables</a>.

*/

void QTable::paintCell( QPainter* p, int row, int col,
			const QRect &cr, bool selected )
{
    if ( selected &&
	 row == curRow &&
	 col == curCol )
	selected = FALSE;

    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;

    QColorGroup cg;
#if defined(Q_WS_WIN)
    bool drawActiveSelection = hasFocus() || style() != WindowsStyle;
    if ( !drawActiveSelection ) {
	QWidget *fw = qApp->focusWidget();
	while ( fw ) {
	    fw = fw->parentWidget();
	    if ( fw == this ) {
		drawActiveSelection = TRUE;
		break;
	    }
	}
    }
    if ( !drawActiveSelection && ( qWinVersion() == WV_98 || qWinVersion() == WV_2000 || qWinVersion() == WV_XP ) )
	cg = palette().inactive();
    else
#endif
	cg = colorGroup();

    QTableItem *itm = item( row, col );
    if ( itm ) {
	p->save();
	QColorGroup cg = colorGroup();
	if ( !itm->isEnabled() )
	    cg = palette().disabled();
	itm->paint( p, cg, cr, selected );
	p->restore();
    } else {
	p->fillRect( 0, 0, w, h, selected ? cg.brush( QColorGroup::Highlight ) : cg.brush( QColorGroup::Base ) );
    }

    if ( sGrid ) {
	// Draw our lines
	QPen pen( p->pen() );
	p->setPen( colorGroup().mid() );
	p->drawLine( x2, 0, x2, y2 );
	p->drawLine( 0, y2, x2, y2 );
	p->setPen( pen );
    }
}

/*! Draws the focus rectangle of the current cell (see currentRow(),
  currentColumn()).

  The painter \a p is already translated to the
  cell's origin, while \a cr specifies the cell's geometry in content
  coordinates.
*/

void QTable::paintFocus( QPainter *p, const QRect &cr )
{
    if ( !hasFocus() && !viewport()->hasFocus() )
	return;
    QRect focusRect( 0, 0, cr.width(), cr.height() );
    p->setPen( QPen( black, 1 ) );
    p->setBrush( NoBrush );
    bool focusEdited = FALSE;
    if ( edMode != NotEditing &&
	 curRow == editRow && curCol == editCol )
	focusEdited = TRUE;
    if ( !focusEdited ) {
	QTableItem *i = item( curRow, curCol );
	focusEdited = ( i &&
			( i->editType() == QTableItem::Always ||
			  ( i->editType() == QTableItem::WhenCurrent &&
			    curRow == i->row() && curCol == i->col() ) ) );
    }
    p->drawRect( focusRect.x(), focusRect.y(), focusRect.width() - 1, focusRect.height() - 1 );
    if ( !focusEdited ) {
	p->drawRect( focusRect.x() - 1, focusRect.y() - 1, focusRect.width() + 1, focusRect.height() + 1 );
    } else {
	p->drawRect( focusRect.x() - 1, focusRect.y() - 1, focusRect.width() + 1, focusRect.height() + 1 );
    }
}

/*! This function fills the \a cw pixels wide and \a ch pixels high
  rectangle starting at position \a cx, \a cy with the
  background color using the painter \a p.

  paintEmptyArea() is invoked by drawContents() to erase
  or fill unused areas.
*/

void QTable::paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch )
{
    // Region of the rect we should draw
    contentsToViewport( cx, cy, cx, cy );
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( QRect( contentsToViewport( QPoint( 0, 0 ) ), tableSize() ) );

    // And draw the rectangles (transformed as needed)
    QMemArray<QRect> r = reg.rects();
    for ( int i = 0; i < (int)r.count(); ++i)
	p->fillRect( r[ i ], colorGroup().brush( QColorGroup::Base ) );
}

/*! Returns the QTableItem representing the contents of the cell at \a
  row, \a col.

  If \a row or \a col are out of range or no content has
  been set for this cell, item() returns 0.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.

    \sa setItem()
*/

QTableItem *QTable::item( int row, int col ) const
{
    if ( row < 0 || col < 0 || row > numRows() - 1 ||
	 col > numCols() - 1 || row * col >= (int)contents.size() )
	return 0;

    return contents[ indexOf( row, col ) ];	// contents array lookup
}

/*!
    Inserts the table item \a item into the table at row \a row,
    column \a col, and repaints the cell. If a table item already exists
    in this cell it is deleted and replaced with \a item. The table
    takes ownership of the table item.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.

  \sa item() takeItem()
*/

void QTable::setItem( int row, int col, QTableItem *item )
{
    if ( !item )
	return;

    if ( (int)contents.size() != numRows() * numCols() )
	resizeData( numRows() * numCols() );

    int orow = item->row();
    int ocol = item->col();
    clearCell( row, col );

    contents.insert( indexOf( row, col ), item );
    item->setRow( row );
    item->setCol( col );
    updateCell( row, col );
    item->updateEditor( orow, ocol );
}

/*! Removes the QTableItem at \a row, \a col.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.
*/

void QTable::clearCell( int row, int col )
{
    if ( (int)contents.size() != numRows() * numCols() )
	resizeData( numRows() * numCols() );
    contents.remove( indexOf( row, col ) );
}

/*! Sets the text in the cell at \a row, \a col to \a text.

    If the cell does not contain a table item a QTableItem is created
    with an EditType of \c OnTyping, otherwise the existing table item's
    text (if any) is replaced with \a text.

  \sa text() setPixmap() setItem() QTableItem::setText()
*/

void QTable::setText( int row, int col, const QString &text )
{
    QTableItem *itm = item( row, col );
    if ( itm ) {
	itm->setText( text );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, QTableItem::OnTyping,
					text, QPixmap() );
	setItem( row, col, i );
    }
}

/*! Sets the pixmap in the cell at \a row, \a col to \a pix.

    If the cell does not contain a table item a QTableItem is created
    with an EditType of \c OnTyping, otherwise the existing table item's
    pixmap (if any) is replaced with \a pix.

  \walkthrough table/small-table-demo/main.cpp
  \skipto QImage
  \printuntil setPixmap

  (Code from \link small-table-demo-example.html
  table/small-table-demo/main.cpp \endlink.) In the example we create an
  image, scale it to the height of the table's fourth row (row 3), then
  set the pixmap for the cell at row 3, column 2 to the scaled pixmap.

  Note that QComboTableItems and QCheckTableItems don't show pixmaps.

  \sa pixmap() setText() setItem() QTableItem::setPixmap()
*/

void QTable::setPixmap( int row, int col, const QPixmap &pix )
{
    QTableItem *itm = item( row, col );
    if ( itm ) {
	itm->setPixmap( pix );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, QTableItem::OnTyping,
					QString::null, pix );
	setItem( row, col, i );
    }
}

/*! Returns the text in cell at \a row, \a col, or an empty string if
  the relevant item does not exist or has no text.

  \sa setText() setPixmap()
*/

QString QTable::text( int row, int col ) const
{
    QTableItem *itm = item( row, col );
    if ( itm )
	return itm->text();
    return QString::null;
}

/*! Returns the pixmap set for the cell at \a row, \a col, or a
  null-pixmap if the cell contains no pixmap.

  \sa setPixmap()
*/

QPixmap QTable::pixmap( int row, int col ) const
{
    QTableItem *itm = item( row, col );
    if ( itm )
	return itm->pixmap();
    return QPixmap();
}

/*! Moves the focus to the cell at \a row, \a col.

  \sa currentRow() currentColumn()
*/

void QTable::setCurrentCell( int row, int col )
{
    QTableItem *itm = item( row, col );
    QTableItem *oldIitem = item( curRow, curCol );
    if ( itm && itm->rowSpan() > 1 && oldIitem == itm && itm->row() != row ) {
	if ( row > curRow )
	    row = itm->row() + itm->rowSpan();
	else if ( row < curRow )
	    row = QMAX( 0, itm->row() - 1 );
    }
    if ( itm && itm->colSpan() > 1 && oldIitem == itm && itm->col() != col ) {
	if ( col > curCol )
	    col = itm->col() + itm->colSpan();
	else if ( col < curCol )
	    col = QMAX( 0, itm->col() - 1 );
    }
    if ( curRow != row || curCol != col ) {
	itm = oldIitem;
	if ( itm && itm->editType() != QTableItem::Always )
	    endEdit( curRow, curCol, TRUE, FALSE );
	int oldRow = curRow;
	int oldCol = curCol;
	curRow = row;
	curCol = col;
	repaintCell( oldRow, oldCol );
	repaintCell( curRow, curCol );
	ensureCellVisible( curRow, curCol );
	emit currentChanged( row, col );
	if ( !isColumnSelected( oldCol ) && !isRowSelected( oldRow ) ) {
	    topHeader->setSectionState( oldCol, QTableHeader::Normal );
	    leftHeader->setSectionState( oldRow, QTableHeader::Normal );
	}
	topHeader->setSectionState( curCol, isColumnSelected( curCol, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
	leftHeader->setSectionState( curRow, isRowSelected( curRow, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
	itm = item( curRow, curCol );

	QPoint cellPos( columnPos( curCol ) + leftMargin() - contentsX(), rowPos( curRow ) + topMargin() - contentsY() );
	setMicroFocusHint( cellPos.x(), cellPos.y(), columnWidth( curCol ), rowHeight( curRow ), ( itm && itm->editType() != QTableItem::Never ) );

	if ( cellWidget( oldRow, oldCol ) &&
	     cellWidget( oldRow, oldCol )->hasFocus() )
	    viewport()->setFocus();

	if ( itm && itm->editType() == QTableItem::WhenCurrent ) {
	    if ( beginEdit( curRow, curCol, FALSE ) )
		setEditMode( Editing, row, col );
	} else if ( itm && itm->editType() == QTableItem::Always ) {
	    if ( cellWidget( itm->row(), itm->col() ) )
		cellWidget( itm->row(), itm->col() )->setFocus();
	}
    }
}

/*! Scrolls the table until the cell at \a row, \a col becomes
  visible.
*/

void QTable::ensureCellVisible( int row, int col )
{
    int cw = columnWidth( col );
    int rh = rowHeight( row );
    ensureVisible( columnPos( col ) + cw / 2, rowPos( row ) + rh / 2, cw / 2, rh / 2 );
}

/*!
    Returns TRUE if the cell at \a row, \a col is selected;
    otherwise returns FALSE.

  \sa isRowSelected() isColumnSelected()
*/

bool QTable::isSelected( int row, int col ) const
{
    QPtrListIterator<QTableSelection> it( selections );
    QTableSelection *s;
    while ( ( s = it.current() ) != 0 ) {
	++it;
	if ( s->isActive() &&
	     row >= s->topRow() &&
	     row <= s->bottomRow() &&
	     col >= s->leftCol() &&
	     col <= s->rightCol() )
	    return TRUE;
	if ( row == currentRow() && col == currentColumn() )
	    return TRUE;
    }
    return FALSE;
}

/*!
    Returns TRUE if row \a row is selected; otherwise returns FALSE.

    If \a full is FALSE (the default), 'row is selected' means that at
    least one cell in the row is selected. If \a full is TRUE, then 'row
    is selected' means every cell in the row is selected.

  \sa isColumnSelected() isSelected()
*/

bool QTable::isRowSelected( int row, bool full ) const
{
    if ( !full ) {
	QPtrListIterator<QTableSelection> it( selections );
	QTableSelection *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->isActive() &&
		 row >= s->topRow() &&
		 row <= s->bottomRow() )
	    return TRUE;
	if ( row == currentRow() )
	    return TRUE;
	}
    } else {
	QPtrListIterator<QTableSelection> it( selections );
	QTableSelection *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->isActive() &&
		 row >= s->topRow() &&
		 row <= s->bottomRow() &&
		 s->leftCol() == 0 &&
		 s->rightCol() == numCols() - 1 )
		return TRUE;
	}
    }
    return FALSE;
}

/*!

    Returns TRUE if column \a col is selected; otherwise returns FALSE.

    If \a full is FALSE (the default), 'col is selected' means that at
    least one cell in the column is selected. If \a full is TRUE, then
    'col is selected' means every cell in the column is selected.

  \sa isRowSelected() isSelected()
*/

bool QTable::isColumnSelected( int col, bool full ) const
{
    if ( !full ) {
	QPtrListIterator<QTableSelection> it( selections );
	QTableSelection *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->isActive() &&
		 col >= s->leftCol() &&
		 col <= s->rightCol() )
	    return TRUE;
	if ( col == currentColumn() )
	    return TRUE;
	}
    } else {
	QPtrListIterator<QTableSelection> it( selections );
	QTableSelection *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->isActive() &&
		 col >= s->leftCol() &&
		 col <= s->rightCol() &&
		 s->topRow() == 0 &&
		 s->bottomRow() == numRows() - 1 )
		return TRUE;
	}
    }
    return FALSE;
}

/*! Returns the number of selections.

  \sa currentSelection()
*/

int QTable::numSelections() const
{
    return selections.count();
}

/*! Returns selection number \a num, or an empty QTableSelection
  if \a num is out of range (see QTableSelection::isNull()).
*/

QTableSelection QTable::selection( int num ) const
{
    if ( num < 0 || num >= (int)selections.count() )
	return QTableSelection();

    QTableSelection *s = ( (QTable*)this )->selections.at( num );
    return *s;
}

/*! Adds a selection described by \a s to the table and returns its
 number or -1 if the selection is invalid.

 Remember to call QTableSelection::init() and
 QTableSelection::expandTo() to make the selection valid (see also
 QTableSelection::isActive()).

 \sa numSelections() removeSelection() clearSelection()
*/

int QTable::addSelection( const QTableSelection &s )
{
    if ( !s.isActive() )
	return -1;
    QTableSelection *sel = new QTableSelection( s );
    selections.append( sel );
    viewport()->repaint( FALSE );
    return selections.count() - 1;
}

/*!
    If the table has a selection, \a s, this selection is removed from
    the table.

  \sa addSelection() numSelections()
*/

void QTable::removeSelection( const QTableSelection &s )
{
    for ( QTableSelection *sel = selections.first(); sel; sel = selections.next() ) {
	if ( s == *sel ) {
	    selections.removeRef( sel );
	    viewport()->repaint( FALSE );
	}
    }
}

/*! \overload

  Removes selection number \a num from the table.

  \sa numSelections() addSelection()
*/

void QTable::removeSelection( int num )
{
    if ( num < 0 || num >= (int)selections.count() )
	return;

    QTableSelection *s = selections.at( num );
    selections.removeRef( s );
    viewport()->repaint( FALSE );
}

/*! Returns the number of the current selection or -1 if there is
  no current selection.

  \sa numSelection()
*/

int QTable::currentSelection() const
{
    if ( !currentSel )
	return -1;
    return ( (QTable*)this )->selections.findRef( currentSel );
}

/*! \reimp
*/

void QTable::contentsMousePressEvent( QMouseEvent* e )
{
    shouldClearSelection = FALSE;
    mousePressed = TRUE;
    if ( isEditing() )
	endEdit( editRow, editCol, TRUE, edMode != Editing );

    int tmpRow = rowAt( e->pos().y() );
    int tmpCol = columnAt( e->pos().x() );
    pressedRow = tmpRow;
    pressedCol = tmpCol;
    fixRow( tmpRow, e->pos().y() );
    fixCol( tmpCol, e->pos().x() );
    startDragCol = -1;
    startDragRow = -1;
    if ( e->button() != LeftButton ) {
	emit pressed( tmpRow, tmpCol, e->button(), e->pos() );
	if ( context_menu )
	    emit contextMenuRequested( tmpRow, tmpCol, mapToGlobal( contentsToViewport( e->pos() ) ) );
	return;
    }

    if ( isSelected( tmpRow, tmpCol ) ) {
	startDragCol = tmpCol;
	startDragRow = tmpRow;
	dragStartPos = e->pos();
    }

    QTableItem *itm = item( pressedRow, pressedCol );
    if ( itm && !itm->isEnabled() )
	return;

    if ( ( e->state() & ShiftButton ) == ShiftButton ) {
	if ( selMode != NoSelection ) {
	    if ( !currentSel ) {
		currentSel = new QTableSelection();
		selections.append( currentSel );
		currentSel->init( curRow, curCol );
	    }
	    QTableSelection oldSelection = *currentSel;
	    currentSel->expandTo( tmpRow, tmpCol );
	    repaintSelections( &oldSelection, currentSel );
	    emit selectionChanged();
	}
	setCurrentCell( tmpRow, tmpCol );
    } else if ( ( e->state() & ControlButton ) == ControlButton ) {
	if ( selMode != NoSelection ) {
	    if ( selMode == Single )
		clearSelection();
	    currentSel = new QTableSelection();
	    selections.append( currentSel );
	    currentSel->init( tmpRow, tmpCol );
	    emit selectionChanged();
	}
	setCurrentCell( tmpRow, tmpCol );
    } else {
	setCurrentCell( tmpRow, tmpCol );
	if ( isSelected( tmpRow, tmpCol ) ) {
	    shouldClearSelection = TRUE;
	} else {
	    clearSelection();
	    if ( selMode != NoSelection ) {
		currentSel = new QTableSelection();
		selections.append( currentSel );
		currentSel->init( tmpRow, tmpCol );
		emit selectionChanged();
	    }
	}
    }

    emit pressed( tmpRow, tmpCol, e->button(), e->pos() );
}

/*! \reimp
*/

void QTable::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    int tmpRow = rowAt( e->pos().y() );
    int tmpCol = columnAt( e->pos().x() );
    QTableItem *itm = item( tmpRow, tmpCol );
    if ( itm && !itm->isEnabled() )
	return;
    if ( tmpRow != -1 && tmpCol != -1 ) {
	if ( beginEdit( tmpRow, tmpCol, FALSE ) )
	    setEditMode( Editing, tmpRow, tmpCol );
    }

    emit doubleClicked( tmpRow, tmpCol, e->button(), e->pos() );
}

/*! Sets the current edit mode to \a mode, the current edit row to \a
  row and the current edit column to \a col.

  \sa EditMode
*/

void QTable::setEditMode( EditMode mode, int row, int col )
{
    edMode = mode;
    editRow = row;
    editCol = col;
}


/*! \reimp
*/

void QTable::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    int tmpRow = rowAt( e->pos().y() );
    int tmpCol = columnAt( e->pos().x() );
    fixRow( tmpRow, e->pos().y() );
    fixCol( tmpCol, e->pos().x() );

#ifndef QT_NO_DRAGANDDROP
    if ( dragEnabled() && startDragRow != -1 && startDragCol != -1 ) {
	if ( QPoint( dragStartPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
	    mousePressed = FALSE;
	    startDrag();
	}
	return;
    }
#endif

    if ( shouldClearSelection ) {
	clearSelection();
	if ( selMode != NoSelection ) {
	    currentSel = new QTableSelection();
	    selections.append( currentSel );
	    currentSel->init( tmpRow, tmpCol );
	    emit selectionChanged();
	}
	shouldClearSelection = FALSE;
    }

    QPoint pos = mapFromGlobal( e->globalPos() );
    pos -= QPoint( leftHeader->width(), topHeader->height() );
    autoScrollTimer->stop();
    doAutoScroll();
    if ( pos.x() < 0 || pos.x() > visibleWidth() || pos.y() < 0 || pos.y() > visibleHeight() )
	autoScrollTimer->start( 100, TRUE );
}

/*! \internal
 */

void QTable::doValueChanged()
{
    emit valueChanged( ( (QTableItem*)sender() )->row(),
		       ( (QTableItem*)sender() )->col() );
}

/*! \internal
*/

void QTable::doAutoScroll()
{
    if ( !mousePressed )
	return;
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal( pos );
    pos -= QPoint( leftHeader->width(), topHeader->height() );

    int tmpRow = curRow;
    int tmpCol = curCol;
    if ( pos.y() < 0 )
	tmpRow--;
    else if ( pos.y() > visibleHeight() )
	tmpRow++;
    if ( pos.x() < 0 )
	tmpCol--;
    else if ( pos.x() > visibleWidth() )
	tmpCol++;

    pos += QPoint( contentsX(), contentsY() );
    if ( tmpRow == curRow )
	tmpRow = rowAt( pos.y() );
    if ( tmpCol == curCol )
	tmpCol = columnAt( pos.x() );
    pos -= QPoint( contentsX(), contentsY() );

    fixRow( tmpRow, pos.y() );
    fixCol( tmpCol, pos.x() );

    if ( tmpRow < 0 || tmpRow > numRows() - 1 )
	tmpRow = currentRow();
    if ( tmpCol < 0 || tmpCol > numCols() - 1 )
	tmpCol = currentColumn();

    ensureCellVisible( tmpRow, tmpCol );

    if ( currentSel && selMode != NoSelection ) {
	QTableSelection oldSelection = *currentSel;
	currentSel->expandTo( tmpRow, tmpCol );
	setCurrentCell( tmpRow, tmpCol );
	repaintSelections( &oldSelection, currentSel );
	emit selectionChanged();
    } else {
	setCurrentCell( tmpRow, tmpCol );
    }

    if ( pos.x() < 0 || pos.x() > visibleWidth() || pos.y() < 0 || pos.y() > visibleHeight() )
	autoScrollTimer->start( 100, TRUE );
}

/*! \reimp
*/

void QTable::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    if ( shouldClearSelection ) {
	int tmpRow = rowAt( e->pos().y() );
	int tmpCol = columnAt( e->pos().x() );
	fixRow( tmpRow, e->pos().y() );
	fixCol( tmpCol, e->pos().x() );
	clearSelection();
	if ( selMode != NoSelection ) {
	    currentSel = new QTableSelection();
	    selections.append( currentSel );
	    currentSel->init( tmpRow, tmpCol );
	    emit selectionChanged();
	}
	shouldClearSelection = FALSE;
    }
    mousePressed = FALSE;
    autoScrollTimer->stop();
    if ( pressedRow == curRow && pressedCol == curCol )
	emit clicked( curRow, curCol, e->button(), e->pos() );
}

/*!
  \reimp
*/

void QTable::contentsContextMenuEvent( QContextMenuEvent *e )
{
    if ( e->reason() == QContextMenuEvent::Keyboard ) {
	QRect r = cellGeometry( curRow, curCol );
	r.moveBy( -contentsX(), -contentsY() );
	emit contextMenuRequested( curRow, curCol, mapToGlobal( contentsToViewport( r.center() ) ) );
    } else {
	QMouseEvent me( QEvent::MouseButtonPress, e->pos(), e->globalPos(), RightButton, e->state() );
	context_menu = TRUE;
	contentsMousePressEvent( &me );
	context_menu = FALSE;
    }
    e->accept();
}


/*! \reimp
*/

bool QTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QScrollView::eventFilter( o, e );

    QWidget *editorWidget = cellWidget( editRow, editCol );
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	QTableItem *itm = item( curRow, curCol );

	if ( isEditing() && editorWidget && o == editorWidget ) {
	    itm = item( editRow, editCol );
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape ) {
		if ( !itm || itm->editType() == QTableItem::OnTyping )
		    endEdit( editRow, editCol, FALSE, edMode != Editing );
		return TRUE;
	    }

	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
		if ( !itm || itm->editType() == QTableItem::OnTyping )
		    endEdit( editRow, editCol, TRUE, edMode != Editing );
		activateNextCell();
		return TRUE;
	    }

	    if ( ke->key() == Key_Tab || ke->key() == Key_BackTab ) {
		if ( !itm || itm->editType() == QTableItem::OnTyping )
		    endEdit( editRow, editCol, TRUE, edMode != Editing );
		if ( ke->key() == Key_Tab && currentColumn() >= numCols() - 1 )
		    return TRUE;
		if ( ke->key() == Key_BackTab && currentColumn() == 0 )
		    return TRUE;
		if ( ke->key() == Key_Tab ) {
		    int cc  = QMIN( numCols() - 1, currentColumn() + 1 );
		    while ( cc < numCols() ) {
			if ( !isColumnReadOnly( cc ) )
			    break;
			++cc;
		    }
		    setCurrentCell( currentRow(), cc );
		} else { // Key_BackTab
		    int cc  = QMAX( 0, currentColumn() - 1 );
		    while ( cc >= 0 ) {
			if ( !isColumnReadOnly( cc ) )
			    break;
			--cc;
		    }
		    setCurrentCell( currentRow(), QMAX( 0, currentColumn() - 1 ) );
		}
		itm = item( curRow, curCol );
		if ( beginEdit( curRow, curCol, FALSE ) )
		    setEditMode( Editing, curRow, curCol );
		return TRUE;
	    }

	    if ( ( edMode == Replacing ||
		   itm && itm->editType() == QTableItem::WhenCurrent ) &&
		 ( ke->key() == Key_Up || ke->key() == Key_Prior ||
		   ke->key() == Key_Home || ke->key() == Key_Down ||
		   ke->key() == Key_Next || ke->key() == Key_End ||
		   ke->key() == Key_Left || ke->key() == Key_Right ) ) {
		if ( !itm || itm->editType() == QTableItem::OnTyping ) {
		    endEdit( editRow, editCol, TRUE, edMode != Editing );
		}
		keyPressEvent( ke );
		return TRUE;
	    }
	} else {
	    QObjectList *l = viewport()->queryList( "QWidget" );
	    if ( l && l->find( o ) != -1 ) {
		QKeyEvent *ke = (QKeyEvent*)e;
		if ( ( ke->state() & ControlButton ) == ControlButton ||
		     ( ke->key() != Key_Left && ke->key() != Key_Right &&
		       ke->key() != Key_Up && ke->key() != Key_Down &&
		       ke->key() != Key_Prior && ke->key() != Key_Next &&
		       ke->key() != Key_Home && ke->key() != Key_End ) )
		    return FALSE;
		keyPressEvent( (QKeyEvent*)e );
		return TRUE;
	    }
	    delete l;
	}

	} break;
    case QEvent::FocusOut:
	if ( o == this || o == viewport() ) {
	    updateCell( curRow, curCol );
#if defined(Q_WS_WIN)
	    if ( style() == WindowsStyle &&
		 ( qWinVersion() == WV_98 || qWinVersion() == WV_2000 || qWinVersion() == WV_XP ) )
		repaintSelections();
#endif
	    return TRUE;
	}
	if ( isEditing() && editorWidget && o == editorWidget && ( (QFocusEvent*)e )->reason() != QFocusEvent::Popup ) {
	    QTableItem *itm = item( editRow, editCol );
	    if ( !itm || itm->editType() == QTableItem::OnTyping ) {
		endEdit( editRow, editCol, TRUE, edMode != Editing );
		return TRUE;
	    }
	}
	break;
    case QEvent::FocusIn:
	if ( o == this || o == viewport() ) {
	    updateCell( curRow, curCol );
#if defined(Q_WS_WIN)
	    if ( style() == WindowsStyle &&
		 ( qWinVersion() == WV_98 || qWinVersion() == WV_2000 || qWinVersion() == WV_XP ) )
		repaintSelections();
#endif
	    if ( isEditing() && editorWidget )
		editorWidget->setFocus();
	    return TRUE;
	}
	break;
    case QEvent::Wheel:
	if ( o == this || o == viewport() ) {
	    QWheelEvent* we = (QWheelEvent*)e;
	    scrollBy( 0, -we->delta() );
	    we->accept();
	    return TRUE;
	}
    default:
	break;
    }

    return QScrollView::eventFilter( o, e );
}

/*! \reimp
*/

void QTable::keyPressEvent( QKeyEvent* e )
{
    if ( isEditing() && item( editRow, editCol ) &&
	 item( editRow, editCol )->editType() == QTableItem::OnTyping )
	return;

    int tmpRow = curRow;
    int tmpCol = curCol;
    int oldRow = tmpRow;
    int oldCol = tmpCol;

    bool navigationKey = FALSE;
    int r;
    switch ( e->key() ) {
    case Key_Left:
	tmpCol = QMAX( 0, tmpCol - 1 );
	navigationKey = TRUE;
	break;
    case Key_Right:
	tmpCol = QMIN( numCols() - 1, tmpCol + 1 );
	navigationKey = TRUE;
	break;
    case Key_Up:
	tmpRow = QMAX( 0, tmpRow - 1 );
	navigationKey = TRUE;
	break;
    case Key_Down:
	tmpRow = QMIN( numRows() - 1, tmpRow + 1 );
	navigationKey = TRUE;
	break;
    case Key_Prior:
	r = QMAX( 0, rowAt( rowPos( tmpRow ) - visibleHeight() ) );
	if ( r < tmpRow )
	    tmpRow = r;
	navigationKey = TRUE;
	break;
    case Key_Next:
	r = QMIN( numRows() - 1, rowAt( rowPos( tmpRow ) + visibleHeight() ) );
	if ( r > tmpRow )
	    tmpRow = r;
	else
	    tmpRow = numRows() - 1;
	navigationKey = TRUE;
	break;
    case Key_Home:
	tmpRow = 0;
	navigationKey = TRUE;
	break;
    case Key_End:
	tmpRow = numRows() - 1;
	navigationKey = TRUE;
	break;
    case Key_F2:
	if ( beginEdit( tmpRow, tmpCol, FALSE ) )
	    setEditMode( Editing, tmpRow, tmpCol );
	break;
    default: // ... or start in-place editing
	if ( e->text()[ 0 ].isPrint() ) {
	    QTableItem *itm = item( tmpRow, tmpCol );
	    if ( !itm || itm->editType() == QTableItem::OnTyping ) {
		QWidget *w;
		if ( ( w = beginEdit( tmpRow, tmpCol,
				      itm ? itm->isReplaceable() : TRUE ) ) ) {
		    setEditMode( ( !itm || itm && itm->isReplaceable()
				   ? Replacing : Editing ), tmpRow, tmpCol );
		    QApplication::sendEvent( w, e );
		}
	    }
	}
    }

    if ( navigationKey ) {
	if ( ( e->state() & ShiftButton ) == ShiftButton &&
	     selMode != NoSelection ) {
	    setCurrentCell( tmpRow, tmpCol );
	    bool justCreated = FALSE;
	    if ( !currentSel ) {
		justCreated = TRUE;
		currentSel = new QTableSelection();
		selections.append( currentSel );
		currentSel->init( oldRow, oldCol );
	    }
	    QTableSelection oldSelection = *currentSel;
	    currentSel->expandTo( tmpRow, tmpCol );
	    repaintSelections( justCreated ? 0 : &oldSelection, currentSel );
	    emit selectionChanged();
	} else {
	    clearSelection();
	    setCurrentCell( tmpRow, tmpCol );
	}
    } else {
	setCurrentCell( tmpRow, tmpCol );
    }
}

/*! \reimp
*/

void QTable::focusInEvent( QFocusEvent* )
{
#if defined(Q_WS_WIN)
    if ( style() == WindowsStyle &&
	 ( qWinVersion() == WV_98 || qWinVersion() == WV_2000 || qWinVersion() == WV_XP ) )
	repaintSelections();
#endif
    QPoint cellPos( columnPos( curCol ) + leftMargin() - contentsX(), rowPos( curRow ) + topMargin() - contentsY() );
    QTableItem *itm = item( curRow, curCol );
    setMicroFocusHint( cellPos.x(), cellPos.y(), columnWidth( curCol ), rowHeight( curRow ), ( itm && itm->editType() != QTableItem::Never ) );
}


/*! \reimp
*/

void QTable::focusOutEvent( QFocusEvent* )
{
#if defined(Q_WS_WIN)
    if ( style() == WindowsStyle &&
	 ( qWinVersion() == WV_98 || qWinVersion() == WV_2000 || qWinVersion() == WV_XP ) )
	repaintSelections();
#endif
}

/*! \reimp
*/

QSize QTable::sizeHint() const
{
    QSize s = tableSize();
    if ( s.width() < 500 && s.height() < 500 )
	return QSize( tableSize().width() + leftMargin() + 5,
		      tableSize().height() + topMargin() + 5 );
    return QScrollView::sizeHint();
}

/*! \reimp
*/

void QTable::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    updateGeometries();
}

/*! \reimp
*/

void QTable::showEvent( QShowEvent *e )
{
    QScrollView::showEvent( e );
    QRect r( cellGeometry( numRows() - 1, numCols() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
}

static bool inUpdateCell = FALSE;

/*! Repaints the cell at \a row, \a col.

*/

void QTable::updateCell( int row, int col )
{
    if ( inUpdateCell || row == -1 || col == -1 )
	return;
    inUpdateCell = TRUE;
    QRect cg = cellGeometry( row, col );
    QRect r( contentsToViewport( QPoint( cg.x() - 2, cg.y() - 2 ) ),
	     QSize( cg.width() + 4, cg.height() + 4 ) );
    QApplication::postEvent( viewport(), new QPaintEvent( r, FALSE ) );
    inUpdateCell = FALSE;
}

void QTable::repaintCell( int row, int col )
{
    if ( row == -1 || col == -1 )
	return;
    QRect cg = cellGeometry( row, col );
    QRect r( QPoint( cg.x() - 2, cg.y() - 2 ),
	     QSize( cg.width() + 4, cg.height() + 4 ) );
    repaintContents( r, FALSE );
}

void QTable::contentsToViewport2( int x, int y, int& vx, int& vy )
{
    const QPoint v = contentsToViewport2( QPoint( x, y ) );
    vx = v.x();
    vy = v.y();
}

QPoint QTable::contentsToViewport2( const QPoint &p )
{
    return QPoint( p.x() - contentsX(),
		   p.y() - contentsY() );
}

QPoint QTable::viewportToContents2( const QPoint& vp )
{
    return QPoint( vp.x() + contentsX(),
		   vp.y() + contentsY() );
}

void QTable::viewportToContents2( int vx, int vy, int& x, int& y )
{
    const QPoint c = viewportToContents2( QPoint( vx, vy ) );
    x = c.x();
    y = c.y();
}

/*! This function should be called whenever the column width of \a col
  has been changed. It updates the geometry of any affected columns and
  repaints the table to reflect the changes it has made.
*/

void QTable::columnWidthChanged( int col )
{
    updateContents( columnPos( col ), contentsY(), contentsWidth(), visibleHeight() );
    QSize s( tableSize() );
    int w = contentsWidth();
    resizeContents( s.width(), s.height() );
    if ( contentsWidth() < w )
	repaintContents( s.width(), contentsY(),
			 w - s.width() + 1, visibleHeight(), TRUE );
    else
	repaintContents( w, contentsY(),
			 s.width() - w + 1, visibleHeight(), FALSE );

    updateGeometries();
}

/*! This function should be called whenever the row height of \a row
  has been changed. It updates the geometry of any affected rows and
  repaints the table to reflect the changes it has made.
*/

void QTable::rowHeightChanged( int row )
{
    updateContents( contentsX(), rowPos( row ), visibleWidth(), contentsHeight() );
    QSize s( tableSize() );
    int h = contentsHeight();
    resizeContents( s.width(), s.height() );
    if ( contentsHeight() < h )
	repaintContents( contentsX(), contentsHeight(),
			 visibleWidth(), h - s.height() + 1, TRUE );
    else
	repaintContents( contentsX(), h,
			 visibleWidth(), s.height() - h + 1, FALSE );

    updateGeometries();
}

/*! \internal */

void QTable::updateRowWidgets( int row )
{
    for ( int i = 0; i < numCols(); ++i ) {
	QWidget *w = cellWidget( row, i );
	if ( !w )
	    continue;
	moveChild( w, columnPos( i ), rowPos( row ) );
	w->resize( columnWidth( i ) - 1, rowHeight( row ) - 1 );
    }
}

/*! \internal */

void QTable::updateColWidgets( int col )
{
    for ( int i = 0; i < numRows(); ++i ) {
	QWidget *w = cellWidget( i, col );
	if ( !w )
	    continue;
	moveChild( w, columnPos( col ), rowPos( i ) );
	w->resize( columnWidth( col ) - 1, rowHeight( i ) - 1 );
    }
}

/*! This function is called when column order is to be changed, i.e.
  when the user moved the column header \a section
  from \a fromIndex to \a toIndex.

  If you want to change the column order programmatically, call
  swapRows() or swapColumns();

  \sa QHeader::indexChange() rowIndexChanged()
*/

void QTable::columnIndexChanged( int, int, int )
{
    repaintContents( contentsX(), contentsY(),
		     visibleWidth(), visibleHeight(), FALSE );
}

/*! This function is called when the order of the rows is to be
  changed, i.e. the user moved the row header section \a section
  from \a fromIndex to \a toIndex.

  If you want to change the order programmatically, call
  swapRows() or swapColumns();

  \sa QHeader::indexChange() columnIndexChanged()
*/

void QTable::rowIndexChanged( int, int, int )
{
    repaintContents( contentsX(), contentsY(),
		     visibleWidth(), visibleHeight(), FALSE );
}

/*! This function is called when the column \a col has been
  clicked. The default implementation sorts this column if
  sorting() is TRUE.
*/

void QTable::columnClicked( int col )
{
    if ( !sorting() )
	return;

    if ( col == lastSortCol ) {
	asc = !asc;
    } else {
	lastSortCol = col;
	asc = TRUE;
    }

    sortColumn( lastSortCol, asc );
}

/*! \property QTable::sorting
 \brief whether a click on the header of a column sorts that column

  \sa sortColumn()
*/

void QTable::setSorting( bool b )
{
    doSort = b;
}

bool QTable::sorting() const
{
    return doSort;
}

static bool inUpdateGeometries = FALSE;

/*!
    This function updates the geometries of the left and top header.
    You would not normally need to call this function.

*/

void QTable::updateGeometries()
{
    if ( inUpdateGeometries )
	return;
    inUpdateGeometries = TRUE;
    QSize ts = tableSize();
    if ( topHeader->offset() &&
	 ts.width() < topHeader->offset() + topHeader->width() )
	horizontalScrollBar()->setValue( ts.width() - topHeader->width() );
    if ( leftHeader->offset() &&
	 ts.height() < leftHeader->offset() + leftHeader->height() )
	verticalScrollBar()->setValue( ts.height() - leftHeader->height() );

    leftHeader->setGeometry( frameWidth(), topMargin() + frameWidth(),
			     leftMargin(), visibleHeight() );
    topHeader->setGeometry( leftMargin() + frameWidth(), frameWidth(),
			    visibleWidth(), topMargin() );
    horizontalScrollBar()->raise();
    verticalScrollBar()->raise();
    inUpdateGeometries = FALSE;
}

/*! Returns the width of column \a col.

  \sa setColumnWidth rowHeight()
*/

int QTable::columnWidth( int col ) const
{
    return topHeader->sectionSize( col );
}

/*! Returns the height of row \a row.

  \sa setRowHeight columnWidth()
*/

int QTable::rowHeight( int row ) const
{
    return leftHeader->sectionSize( row );
}

/*! Returns the x-coordinate of the column \a col in content
  coordinates.

  \sa columnAt() rowPos()
*/

int QTable::columnPos( int col ) const
{
    return topHeader->sectionPos( col );
}

/*! Returns the y-coordinate of the row \a row in content coordinates.

  \sa rowAt() columnPos()
*/

int QTable::rowPos( int row ) const
{
    return leftHeader->sectionPos( row );
}

/*! Returns the number of the column at \a pos. \a pos must be given in
  content coordinates.

  \sa columnPos() rowAt()
*/

int QTable::columnAt( int pos ) const
{
    return topHeader->sectionAt( pos );
}

/*! Returns the number of the row at \a pos. \a pos must be given in
  content coordinates.

  \sa rowPos() columnAt()
*/

int QTable::rowAt( int pos ) const
{
    return leftHeader->sectionAt( pos );
}

/*! Returns the bounding rectangle of the cell at \a row, \a col in content
  coordinates.
*/

QRect QTable::cellGeometry( int row, int col ) const
{
    QTableItem *itm = item( row, col );
    if ( !itm || itm->rowSpan() == 1 && itm->colSpan() == 1 )
	return QRect( columnPos( col ), rowPos( row ),
		      columnWidth( col ), rowHeight( row ) );

    while ( row != itm->row() )
	row--;
    while ( col != itm->col() )
	col--;

    QRect rect( columnPos( col ), rowPos( row ),
		columnWidth( col ), rowHeight( row ) );

    for ( int r = 1; r < itm->rowSpan(); ++r )
	    rect.setHeight( rect.height() + rowHeight( r + row ) );
    for ( int c = 1; c < itm->colSpan(); ++c )
	rect.setWidth( rect.width() + columnWidth( c + col ) );

    return rect;
}

/*! Returns the size of the table.

  This is the same as the coordinates of the bottom-right edge of
  the last table cell.
*/

QSize QTable::tableSize() const
{
    return QSize( columnPos( numCols() - 1 ) + columnWidth( numCols() - 1 ),
		  rowPos( numRows() - 1 ) + rowHeight( numRows() - 1 ) );
}

/*! \property QTable::numRows
  \brief The number of rows in the table

  \sa numCols
*/

int QTable::numRows() const
{
    return leftHeader->count();
}

/*! \property QTable::numCols
  \brief The number of columns in the table

  \sa  numRows
*/

int QTable::numCols() const
{
    return topHeader->count();
}

/*! Sets the number of rows to \a r.

  \sa numRows() setNumCols()
*/

void QTable::setNumRows( int r )
{
    if ( r < 0 )
	return;
    leftHeader->setUpdatesEnabled( FALSE );
    bool updateBefore = r < numRows();
    int w = leftMargin();
    if ( r > numRows() ) {
	clearSelection( FALSE );
	while ( numRows() < r ) {
	    leftHeader->addLabel( QString::number( numRows() + 1 ), 20 );
	    int tmpw = fontMetrics().width( QString::number( numRows() + 1 ) + "  " );
	    w = QMAX( w, tmpw );
	}
    } else {
	clearSelection( FALSE );
	while ( numRows() > r )
	    leftHeader->removeLabel( numRows() - 1 );
    }

    if ( w > leftMargin() )
	setLeftMargin( w );

    QPtrVector<QTableItem> tmp;
    tmp.resize( contents.size() );
    int i;
    for ( i = 0; i < (int)tmp.size(); ++i )
	tmp.insert( i, contents[ i ] );
    contents.setAutoDelete( FALSE );
    contents.clear();
    contents.setAutoDelete( TRUE );
    resizeData( numRows() * numCols() );

    for ( i = 0; i < (int)tmp.size(); ++i ) {
	QTableItem *it = tmp [ i ];
	int idx = it ? indexOf( it->row(), it->col() ) : 0;
	if ( it && (uint)idx < contents.size() )
	    contents.insert( idx, it );
    }

    leftHeader->setUpdatesEnabled( TRUE );
    QRect r2( cellGeometry( numRows() - 1, numCols() - 1 ) );
    resizeContents( r2.right() + 1, r2.bottom() + 1 );
    updateGeometries();
    if ( updateBefore )
	repaintContents( contentsX(), contentsY(),
			 visibleWidth(), visibleHeight(), TRUE );
    else
	repaintContents( contentsX(), contentsY(),
			 visibleWidth(), visibleHeight(), FALSE );
    leftHeader->update();
}

/*! Sets the number of columns to \a c.

  \sa numCols() setNumRows()
*/

void QTable::setNumCols( int c )
{
    if ( c < 0 )
	return;
    topHeader->setUpdatesEnabled( FALSE );
    bool updateBefore = c < numCols();
    if ( c > numCols() ) {
	clearSelection( FALSE );
	while ( numCols() < c )
	    topHeader->addLabel( QString::number( numCols() + 1 ), 100 );
    } else {
	clearSelection( FALSE );
	while ( numCols() > c )
	    topHeader->removeLabel( numCols() - 1 );
    }

    QPtrVector<QTableItem> tmp;
    tmp.resize( contents.size() );
    int i;
    for ( i = 0; i < (int)tmp.size(); ++i )
	tmp.insert( i, contents[ i ] );
    int nc = numCols();
    contents.setAutoDelete( FALSE );
    contents.clear();
    contents.setAutoDelete( TRUE );
    resizeData( numRows() * numCols() );

    for ( i = 0; i < (int)tmp.size(); ++i ) {
	QTableItem *it = tmp[ i ];
	int idx = it ? it->row() * nc + it->col() : 0;
	if ( it && (uint)idx < contents.size() && ( !it || it->col() < numCols() ) )
	    contents.insert( idx, it );
    }

    topHeader->setUpdatesEnabled( TRUE );
    QRect r( cellGeometry( numRows() - 1, numCols() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
    if ( updateBefore )
	repaintContents( contentsX(), contentsY(),
			 visibleWidth(), visibleHeight(), TRUE );
    else
	repaintContents( contentsX(), contentsY(),
			 visibleWidth(), visibleHeight(), FALSE );
    topHeader->update();
}

/*! This function returns the widget which should be used as an editor for
  the contents of the cell at \a row, \a col.

  If \a initFromCell is TRUE, the editor is used to edit the current
  contents of the cell (so the editor widget should be initialized with
  this content). If \a initFromCell is FALSE, the content of the cell is
  replaced with the new content which the user entered into the widget
  created by this function.

    The default functionality is as follows: if \a initFromCell is TRUE
    or the cell has a QTableItem and the table item's
    QTableItem::isReplaceable() is FALSE then the cell is asked to
    create an appropriate editor (using QTableItem::createEditor()).
    Otherwise a QLineEdit is used as the editor.

  If you want to create your own editor for certain cells,
  implement a custom QTableItem subclass and reimplement
  QTableItem::createEditor().

    If you are not using QTableItems and you don't want to use a
    QLineEdit as the default editor, subclass QTable and reimplement
    this function with code like this:
  \code
    QTableItem *i = item( row, col );
    if ( initFromCell || ( i && !i->isReplaceable() ) )
	// If we had a QTableItem ask the base class to create the editor
	return QTable::createEditor( row, col, initFromCell );
    else
	return ...(create your editor)
  \endcode
  Ownership of the editor widget is transferred to the caller.

  If you reimplement this function return 0 for read-only cells.
  You will need to reimplement setCellContentFromEditor() to retrieve
  the data the user entered.

  \sa QTableItem::createEditor()
*/

QWidget *QTable::createEditor( int row, int col, bool initFromCell ) const
{
    if ( isReadOnly() || isRowReadOnly( row ) || isColumnReadOnly( col ) )
	return 0;

    QWidget *e = 0;

    // the current item in the cell should be edited if possible
    QTableItem *i = item( row, col );
    if ( initFromCell || i && !i->isReplaceable() ) {
	if ( i ) {
	    e = i->createEditor();
	    if ( !e || i->editType() == QTableItem::Never )
		return 0;
	}
    }

    // no contents in the cell yet, so open the default editor
    if ( !e ) {
	e = new QLineEdit( viewport() );
	( (QLineEdit*)e )->setFrame( FALSE );
    }

    return e;
}

/*! This function is called to start in-place editing of the cell at \a
  row, \a col. Editing is achieved by creating an editor (createEditor()
  is called) and setting the cell's editor with setCellWidget() to the
  newly created editor. (After editing is complete endEdit() will be
  called to replace the cell's content with the editor's content.) If \a
  replace is TRUE the editor will be initialized with the cell's content
  (if any), i.e. the user will be modifying the original cell content;
  otherwise the user will be entering new data.

  \sa endEdit()
*/

QWidget *QTable::beginEdit( int row, int col, bool replace )
{
    if ( isReadOnly() || isRowReadOnly( row ) || isColumnReadOnly( col ) )
	return 0;
    QTableItem *itm = item( row, col );
    if ( itm && !itm->isEnabled() )
	return 0;
    if ( itm && cellWidget( itm->row(), itm->col() ) )
	return 0;
    ensureCellVisible( curRow, curCol );
    QWidget *e = createEditor( row, col, !replace );
    if ( !e )
	return 0;
    setCellWidget( row, col, e );
    e->setActiveWindow();
    e->setFocus();
    updateCell( row, col );
    return e;
}

/*! This function is called when in-place editing of the cell at \a row,
  \a col is requested to stop.

  If the cell is not being edited or \a accept is FALSE the function
  returns and the cell's contents are left unchanged.

  If \a accept is TRUE the content of the editor must be transferred to
  the relevant cell. If \a replace is TRUE the current content of this
  cell should be replaced by the content of the editor (this means
  removing the current QTableItem of the cell and creating a new one for
  the cell). Otherwise (if possible) the content of the editor should
  just be set to the existing QTableItem of this cell.

  If the cell contents should be replaced or if no QTableItem exists for
  the cell, setCellContentFromEditor() is called. Otherwise
  QTableItem::setContentFromEditor() is called on the QTableItem of the
  cell.

  Finally clearCellWidget() is called to remove the editor widget.

  \sa setCellContentFromEditor(), beginEdit()
*/

void QTable::endEdit( int row, int col, bool accept, bool replace )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;

    if ( !accept ) {
	if ( row == editRow && col == editCol )
	    setEditMode( NotEditing, -1, -1 );
	clearCellWidget( row, col );
	updateCell( row, col );
	viewport()->setFocus();
	updateCell( row, col );
	return;
    }

    QTableItem *i = item( row, col );
    if ( replace && i ) {
	clearCell( row, col );
	i = 0;
    }

    if ( !i )
	setCellContentFromEditor( row, col );
    else
	i->setContentFromEditor( editor );

    if ( row == editRow && col == editCol )
	setEditMode( NotEditing, -1, -1 );

    viewport()->setFocus();
    updateCell( row, col );

    clearCellWidget( row, col );
    emit valueChanged( row, col );
}

/*! This function is called to replace the contents of the cell at \a row,
  \a col with the contents of the cell's editor. If a QTableItem already
  exists for this cell, it is removed first (see clearCell()).

  If for example, you want to create different QTableItems depending on
  the contents of the editor, you might reimplement this function.

  If you want to work without QTableItems, you will need to reimplement
  this function to save the data the user entered into your data
  structure. (See the notes on <a href="#bigtables">large tables</a>.)

  \sa QTableItem::setContentFromEditor() createEditor()
*/

void QTable::setCellContentFromEditor( int row, int col )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;
    clearCell( row, col );
    if ( editor->inherits( "QLineEdit" ) )
	setText( row, col, ( (QLineEdit*)editor )->text() );
}

/*!
    Returns TRUE if the EditMode is \c Editing or \c Replacing.
    Returns FALSE if the EditMode is \c NotEditing.

  \sa QTable::EditMode
*/

bool QTable::isEditing() const
{
    return edMode != NotEditing;
}

/*!
    Returns a single integer which identifies a particular \a row and \a
    col by mapping the 2D table to a 1D array.

    This is useful, for example, if you have a sparse table and want to
    use a QIntDict to map integers to the cells that are used.
*/

int QTable::indexOf( int row, int col ) const
{
    return ( row * numCols() ) + col; // mapping from 2D table to 1D array
}

/*! \internal
*/

void QTable::repaintSelections( QTableSelection *oldSelection,
				QTableSelection *newSelection,
				bool updateVertical, bool updateHorizontal )
{
    if ( oldSelection && *oldSelection == *newSelection )
	return;
    if ( oldSelection && !oldSelection->isActive() )
	oldSelection = 0;

    bool optimize1 = FALSE;
    bool optimize2 = FALSE;

    QRect old;
    if ( oldSelection )
	old = rangeGeometry( oldSelection->topRow(),
			     oldSelection->leftCol(),
			     oldSelection->bottomRow(),
			     oldSelection->rightCol(),
			     optimize1 );
    else
	old = QRect( 0, 0, 0, 0 );

    QRect cur = rangeGeometry( newSelection->topRow(),
			       newSelection->leftCol(),
			       newSelection->bottomRow(),
			       newSelection->rightCol(),
			       optimize2 );
    int i;

    if ( !optimize1 || !optimize2 ||
	 old.width() > SHRT_MAX || old.height() > SHRT_MAX ||
	 cur.width() > SHRT_MAX || cur.height() > SHRT_MAX ) {
	QRect rr = cur.unite( old );
	repaintContents( rr, FALSE );
    } else {
	old = QRect( contentsToViewport2( old.topLeft() ), old.size() );
	cur = QRect( contentsToViewport2( cur.topLeft() ), cur.size() );
	QRegion r1( old );
	QRegion r2( cur );
	QRegion r3 = r1.subtract( r2 );
	QRegion r4 = r2.subtract( r1 );

	for ( i = 0; i < (int)r3.rects().count(); ++i ) {
	    QRect r( r3.rects()[ i ] );
	    r = QRect( viewportToContents2( r.topLeft() ), r.size() );
	    repaintContents( r, FALSE );
	}
	for ( i = 0; i < (int)r4.rects().count(); ++i ) {
	    QRect r( r4.rects()[ i ] );
	    r = QRect( viewportToContents2( r.topLeft() ), r.size() );
	    repaintContents( r, FALSE );
	}
    }

    int top, left, bottom, right;
    top = QMIN( oldSelection ? oldSelection->topRow() : newSelection->topRow(), newSelection->topRow() );
    left = QMIN( oldSelection ? oldSelection->leftCol() : newSelection->leftCol(), newSelection->leftCol() );
    bottom = QMAX( oldSelection ? oldSelection->bottomRow() : newSelection->bottomRow(), newSelection->bottomRow() );
    right = QMAX( oldSelection ? oldSelection->rightCol() : newSelection->rightCol(), newSelection->rightCol() );

    if ( updateHorizontal && left >= 0 ) {
	register int *s = &topHeader->states.data()[left];
	for ( i = left; i <= right; ++i ) {
	    if ( !isColumnSelected( i ) )
		*s = QTableHeader::Normal;
	    else if ( isColumnSelected( i, TRUE ) )
		*s = QTableHeader::Selected;
	    else
		*s = QTableHeader::Bold;
	    ++s;
	}
	topHeader->repaint( FALSE );
    }

    if ( updateVertical && top >= 0 ) {
	register int *s = &leftHeader->states.data()[top];
	for ( i = top; i <= bottom; ++i ) {
	    if ( !isRowSelected( i ) )
		*s = QTableHeader::Normal;
	    else if ( isRowSelected( i, TRUE ) )
		*s = QTableHeader::Selected;
	    else
		*s = QTableHeader::Bold;
	    ++s;
	}
	leftHeader->repaint( FALSE );
    }
}

/*! Repaints all selections */

void QTable::repaintSelections()
{
    if ( selections.isEmpty() )
	return;

    QRect r;
    for ( QTableSelection *s = selections.first(); s; s = selections.next() ) {
	bool b;
	r = r.unite( rangeGeometry( s->topRow(),
				    s->leftCol(),
				    s->bottomRow(),
				    s->rightCol(), b ) );
    }

    repaintContents( r, FALSE );
}

/*! Clears all selections and repaints the appropriate
  regions if \a repaint is TRUE.

  \sa removeSelection()
*/

void QTable::clearSelection( bool repaint )
{
    if ( selections.isEmpty() )
	return;
    bool needRepaint = !selections.isEmpty();

    QRect r;
    for ( QTableSelection *s = selections.first(); s; s = selections.next() ) {
	bool b;
	r = r.unite( rangeGeometry( s->topRow(),
				    s->leftCol(),
				    s->bottomRow(),
				    s->rightCol(), b ) );
    }

    currentSel = 0;
    selections.clear();
    if ( needRepaint && repaint )
	repaintContents( r, FALSE );

    leftHeader->setSectionStateToAll( QTableHeader::Normal );
    leftHeader->repaint( FALSE );
    topHeader->setSectionStateToAll( QTableHeader::Normal );
    topHeader->repaint( FALSE );
    topHeader->setSectionState( curCol, QTableHeader::Bold );
    leftHeader->setSectionState( curRow, QTableHeader::Bold );
    emit selectionChanged();
}

/*! \internal
*/

QRect QTable::rangeGeometry( int topRow, int leftCol,
			     int bottomRow, int rightCol, bool &optimize )
{
    topRow = QMAX( topRow, rowAt( contentsY() ) );
    leftCol = QMAX( leftCol, columnAt( contentsX() ) );
    int ra = rowAt( contentsY() + visibleHeight() );
    if ( ra != -1 )
	bottomRow = QMIN( bottomRow, ra );
    int ca = columnAt( contentsX() + visibleWidth() );
    if ( ca != -1 )
	rightCol = QMIN( rightCol, ca );
    optimize = TRUE;
    QRect rect;
    for ( int r = topRow; r <= bottomRow; ++r ) {
	for ( int c = leftCol; c <= rightCol; ++c ) {
	    rect = rect.unite( cellGeometry( r, c ) );
	    QTableItem *i = item( r, c );
	    if ( i && ( i->rowSpan() > 1 || i->colSpan() > 1 ) )
		optimize = FALSE;
	}
    }
    return rect;
}

/*! This function is called to activate the next cell if in-place editing was
  finished by pressing the Return key.

  The default behaviour is to move from top to bottom, i.e. move to the
  cell beneath the cell being edited. Reimplement this function if you
  want different behaviour, e.g. moving from left to right.
*/

void QTable::activateNextCell()
{
    if ( !currentSel || !currentSel->isActive() ) {
	if ( curRow < numRows() - 1 )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < numCols() - 1 )
	    setCurrentCell( 0, curCol + 1 );
	else
	    setCurrentCell( 0, 0 );
    } else {
	if ( curRow < currentSel->bottomRow() )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < currentSel->rightCol() )
	    setCurrentCell( currentSel->topRow(), curCol + 1 );
	else
	    setCurrentCell( currentSel->topRow(), currentSel->leftCol() );
    }

}

/*! \internal
*/

void QTable::fixRow( int &row, int y )
{
    if ( row == -1 ) {
	if ( y < 0 )
	    row = 0;
	else
	    row = numRows() - 1;
    }
}

/*! \internal
*/

void QTable::fixCol( int &col, int x )
{
    if ( col == -1 ) {
	if ( x < 0 )
	    col = 0;
	else
	    col = numCols() - 1;
    }
}

struct SortableTableItem
{
    QTableItem *item;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static int cmpTableItems( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    SortableTableItem *i1 = (SortableTableItem *)n1;
    SortableTableItem *i2 = (SortableTableItem *)n2;

    return i1->item->key().localeAwareCompare( i2->item->key() );
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*! Sorts column \a col. If \a ascending is
  TRUE the sort is in ascending order, otherwise the sort is in
  descending order.

  If \a wholeRows is TRUE, entire rows are sorted using swapRows();
  otherwise only cells in the column are sorted using swapCells().

  Note that if you are not using QTableItems you will need to
  reimplement swapRows() and swapCells(). (See the notes on <a
  href="#bigtables">large tables</a>.)

  \sa swapRows()
*/

void QTable::sortColumn( int col, bool ascending, bool wholeRows )
{
    int filledRows = 0, i;
    for ( i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( itm )
	    filledRows++;
    }

    if ( !filledRows )
	return;

    SortableTableItem *items = new SortableTableItem[ filledRows ];
    int j = 0;
    for ( i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( !itm )
	    continue;
	items[ j++ ].item = itm;
    }

    qsort( items, filledRows, sizeof( SortableTableItem ), cmpTableItems );

    setUpdatesEnabled( FALSE );
    viewport()->setUpdatesEnabled( FALSE );
    for ( i = 0; i < numRows(); ++i ) {
	if ( i < filledRows ) {
	    if ( ascending ) {
		if ( items[ i ].item->row() == i )
		    continue;
		if ( wholeRows )
		    swapRows( items[ i ].item->row(), i );
		else
		    swapCells( items[ i ].item->row(), col, i, col );
	    } else {
		if ( items[ i ].item->row() == filledRows - i - 1 )
		    continue;
		if ( wholeRows )
		    swapRows( items[ i ].item->row(), filledRows - i - 1 );
		else
		    swapCells( items[ i ].item->row(), col,
			       filledRows - i - 1, col );
	    }
	}
    }
    setUpdatesEnabled( TRUE );
    viewport()->setUpdatesEnabled( TRUE );

    if ( !wholeRows )
	repaintContents( columnPos( col ), contentsY(),
			 columnWidth( col ), visibleHeight(), FALSE );
    else
	repaintContents( contentsX(), contentsY(),
			 visibleWidth(), visibleHeight(), FALSE );

    delete [] items;
}

/*! Hides row \a row.

  \sa showRow() hideColumn()
*/

void QTable::hideRow( int row )
{
    leftHeader->resizeSection( row, 0 );
    leftHeader->setResizeEnabled( FALSE, row );
    rowHeightChanged( row );
}

/*! Hides column \a col.

  \sa showColumn() hideRow()
*/

void QTable::hideColumn( int col )
{
    topHeader->resizeSection( col, 0 );
    topHeader->setResizeEnabled( FALSE, col );
    columnWidthChanged( col );
}

/*! Show row \a row.

  \sa hideRow() showColumn()
*/

void QTable::showRow( int row )
{
    leftHeader->resizeSection( row, 100 );
    leftHeader->setResizeEnabled( TRUE, row );
    rowHeightChanged( row );
}

/*! Show column \a col.

  \sa hideColumn() showRow()
*/

void QTable::showColumn( int col )
{
    topHeader->resizeSection( col, 100 );
    topHeader->setResizeEnabled( TRUE, col );
    columnWidthChanged( col );
}

/*! Resizes column \a col to be \a w pixels wide.

  \sa columnWidth() setRowHeight()
*/

void QTable::setColumnWidth( int col, int w )
{
    topHeader->resizeSection( col, w );
    columnWidthChanged( col );
}

/*! Resizes row \a row to be \a h pixels high.

  \sa rowHeight() setColumnWidth()
*/

void QTable::setRowHeight( int row, int h )
{
    leftHeader->resizeSection( row, h );
    rowHeightChanged( row );
}

/*! Resizes column \a col so that the column width is wide enough to display
 the widest item the column contains.

  \sa adjustRow()
*/

void QTable::adjustColumn( int col )
{
    int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
    w = QMAX( w, 20 );
    for ( int i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( !itm )
	    continue;
	w = QMAX( w, itm->sizeHint().width() );
    }
    setColumnWidth( col, w );
}

/*! Resizes row \a row so that the row height is tall enough to display
 the tallest item the row contains.

  \sa adjustColumn()
*/

void QTable::adjustRow( int row )
{
    int h = 20;
    h = QMAX( h, leftHeader->fontMetrics().height() );
    for ( int i = 0; i < numCols(); ++i ) {
	QTableItem *itm = item( row, i );
	if ( !itm )
	    continue;
	h = QMAX( h, itm->sizeHint().height() );
    }
    setRowHeight( row, h );
}

/*!
    If \a stretch is TRUE, column \a col is set to be stretchable;
    otherwise column \a col is set to be unstretchable.

    If the table widget's width decreases or increases stretchable
    columns will grow narrower or wider to fit the space available as
    completely as possible. The user cannot manually resize stretchable
    columns.

  \sa isColumnStretchable() setRowStretchable() adjustColumn()
*/

void QTable::setColumnStretchable( int col, bool stretch )
{
    topHeader->setSectionStretchable( col, stretch );
}

/*! If \a stretch is TRUE, row \a row is set to be stretchable;
    otherwise row \a row is set to be unstretchable.

    If the table widget's height decreases or increases stretchable
    rows will grow shorter or taller to fit the space available as
    completely as possible. The user cannot manually resize stretchable
    rows.

  \sa isRowStretchable() setColumnStretchable()
*/

void QTable::setRowStretchable( int row, bool stretch )
{
    leftHeader->setSectionStretchable( row, stretch );
}

/*! Returns TRUE if column \a col is stretchable; otherwise returns
 FALSE.

  \sa setColumnStretchable() isRowStretchable()
*/

bool QTable::isColumnStretchable( int col ) const
{
    return topHeader->isSectionStretchable( col );
}

/*! Returns TRUE if row \a row is stretchable; otherwise returns FALSE.

  \sa setRowStretchable() isColumnStretchable()
*/

bool QTable::isRowStretchable( int row ) const
{
    return leftHeader->isSectionStretchable( row );
}

/*!
    Takes the table item \a i out of the table. This function does \e
    not delete the table item. You must either delete the table item yourself or put
    it into a table (using setItem()) which will then take ownership of
    it.

    Use this function if you want to move an item from one cell in a
    table to another, or to move an item from one table to another,
    reinserting the item with setItem().

    If you want to exchange two cells use swapCells().
*/

void QTable::takeItem( QTableItem *i )
{
    QRect rect = cellGeometry( i->row(), i->col() );
    if ( !i )
	return;
    contents.setAutoDelete( FALSE );
    for ( int r = 0; r < i->rowSpan(); ++r ) {
	for ( int c = 0; c < i->colSpan(); ++c )
	    clearCell( i->row() + r, i->col() + c );
    }
    contents.setAutoDelete( TRUE );
    repaintContents( rect, FALSE );
    int orow = i->row();
    int ocol = i->col();
    i->setRow( -1 );
    i->setCol( -1 );
    i->updateEditor( orow, ocol );
}

/*!
    Sets the widget \a e to the cell at \a row, \a col and takes care of
    placing and resizing the widget when the cell geometry changes.

    By default widgets are inserted into a vector with numRows() *
    numCols() elements. In very large tables you will probably want to
    store the widgets in a data structure that consumes less memory (see
    the notes on <a href="#bigtables">large tables</a>). To support the
    use of your own data structure this function calls insertWidget() to
    add the widget to the internal data structure. To use your own data
    structure reimplement insertWidget(), cellWidget() and
    clearCellWidget().

*/

void QTable::setCellWidget( int row, int col, QWidget *e )
{
    if ( !e )
	return;

    QWidget *w = cellWidget( row, col );
    if ( w && row == editRow && col == editCol )
	endEdit( editRow, editCol, FALSE, edMode != Editing );

    e->installEventFilter( this );
    clearCellWidget( row, col );
    if ( e->parent() != viewport() )
	e->reparent( viewport(), QPoint( 0,0 ) );
    insertWidget( row, col, e );
    QRect cr = cellGeometry( row, col );
    e->resize( cr.size() );
    moveChild( e, cr.x(), cr.y() );
    e->show();
    viewport()->setFocus();
}

/*! Inserts widget \a w at \a row, \a col
  into the internal datastructure. See the
  documentation of setCellWidget() for further details.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.
*/

void QTable::insertWidget( int row, int col, QWidget *w )
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return;

    if ( (int)widgets.size() != numRows() * numCols() )
	widgets.resize( numRows() * numCols() );

    widgets.insert( indexOf( row, col ), w );
}

/*! Returns the widget that has been set for the cell at \a row, \a col,
  or 0 if no widget has been set.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.

  \sa clearCellWidget() setCellWidget()
*/

QWidget *QTable::cellWidget( int row, int col ) const
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return 0;

    if ( (int)widgets.size() != numRows() * numCols() )
	( (QTable*)this )->widgets.resize( numRows() * numCols() );

    return widgets[ indexOf( row, col ) ];
}

/*! Removes the widget (if there is one) set for the cell at \a row, \a col.

    If you don't use QTableItems you may need to reimplement this function:
    see the notes on <a href="#bigtables">large tables</a>.

  \sa cellWidget() setCellWidget()
*/

void QTable::clearCellWidget( int row, int col )
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return;

    if ( (int)widgets.size() != numRows() * numCols() )
	widgets.resize( numRows() * numCols() );

    QWidget *w = cellWidget( row, col );
    if ( w )
	w->removeEventFilter( this );
    widgets.remove( indexOf( row, col ) );
}

/*! \fn void QTable::dropped ( QDropEvent * e )

  This signal is emitted when a drop event occurred on the table.

  \a e contains information about the drop.
*/

/*! If \a b is TRUE, the table starts a drag (see dragObject())
  when the user presses and moves the mouse on a selected cell.
*/

void QTable::setDragEnabled( bool b )
{
    dEnabled = b;
}

/*! If this function returns TRUE, the table supports dragging.

  \sa setDragEnabled();
*/

bool QTable::dragEnabled() const
{
    return dEnabled;
}

/*! Inserts \a count empty rows at row \a row.

  \sa insertColumns() removeRow()
*/

void QTable::insertRows( int row, int count )
{
    if ( row < 0 || count <= 0 )
	return;

    row--;
    if ( row >= numRows() )
	return;

    setNumRows( numRows() + count );

    for ( int i = numRows() - count - 1; i > row; --i )
	( (QTableHeader*)verticalHeader() )->swapSections( i, i + count );

    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
}

/*! Inserts \a count empty columns at column \a col.

  \sa insertRows() removeColumn()
*/

void QTable::insertColumns( int col, int count )
{
    if ( col < 0 || count <= 0 )
	return;

    col--;
    if ( col >= numCols() )
	return;

    setNumCols( numCols() + count );

    for ( int i = numCols() - count - 1; i > col; --i )
	( (QTableHeader*)horizontalHeader() )->swapSections( i, i + count );

    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
}

/*!
    Removes row \a row, and deletes all its cells including any table
    items and widgets the cells may contain.

  \sa hideRow() insertRows() removeColumn() removeRows()
*/

void QTable::removeRow( int row )
{
    if ( row < 0 || row >= numRows() )
	return;
    if ( row < numRows() - 1 ) {
	for ( int i = row; i < numRows() - 1; ++i )
	    ( (QTableHeader*)verticalHeader() )->swapSections( i, i + 1 );
    }
    setNumRows( numRows() - 1 );
}

/*!
    Removes the rows listed in the array \a rows, and deletes all their
    cells including any table items and widgets the cells may contain.

   \sa removeRow() insertRows() removeColumns()
*/

void QTable::removeRows( const QMemArray<int> &rows )
{
    if ( rows.count() == 0 )
	return;
    int i;
    for ( i = 0; i < (int)rows.count() - 1; ++i ) {
	for ( int j = rows[i] - i; j < rows[i + 1] - i - 1; j++ ) {
	    ( (QTableHeader*)verticalHeader() )->swapSections( j, j + i + 1 );
	}
    }

    for ( int j = rows[i] - i; j < numRows() - (int)rows.size(); j++)
	( (QTableHeader*)verticalHeader() )->swapSections( j, j + rows.count() );

    setNumRows( numRows() - rows.count() );
}

/*!
    Removes column \a col, and deletes all its cells including any table
    items and widgets the cells may contain.

  \sa removeColumns() hideColumn() insertColumns() removeRow()
*/

void QTable::removeColumn( int col )
{
    if ( col < 0 || col >= numCols() )
	return;
    if ( col < numCols() - 1 ) {
	for ( int i = col; i < numCols() - 1; ++i )
	    ( (QTableHeader*)horizontalHeader() )->swapSections( i, i + 1 );
    }
    setNumCols( numCols() - 1 );
}

/*!
    Removes the columns listed in the array \a cols, and deletes all
    their cells including any table items and widgets the cells may
    contain.

   \sa removeColumn() insertColumns() removeRows()
*/

void QTable::removeColumns( const QMemArray<int> &cols )
{
    if ( cols.count() == 0 )
	return;
    int i;
    for ( i = 0; i < (int)cols.count() - 1; ++i ) {
	for ( int j = cols[i] - i; j < cols[i + 1] - i - 1; j++ ) {
	    ( (QTableHeader*)horizontalHeader() )->swapSections( j, j + i + 1 );
	}
    }

    for ( int j = cols[i] - i; j < numCols() - (int)cols.size(); j++)
	( (QTableHeader*)horizontalHeader() )->swapSections( j, j + cols.count() );

    setNumCols( numCols() - cols.count() );
}

/*!  Starts editing the cell at \a row, \a col.

    If \a replace is TRUE the content of this cell will be replaced by
    the content of the editor when editing is finished, i.e. the user
    will be entering new data; otherwise the current content of the cell
    (if any) will be modified in the editor.

    \sa beginEdit()
*/

void QTable::editCell( int row, int col, bool replace )
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return;

    if ( beginEdit( row, col, replace ) ) {
	edMode = Editing;
	editRow = row;
	editCol = col;
    }
}

#ifndef QT_NO_DRAGANDDROP

/*! This event handler is called whenever a QTable object receives a
  \l QDragEnterEvent \a e, i.e. when the user pressed the mouse
  button to drag something.

  The focus is moved to the cell where the QDragEnterEvent occurred.
*/

void QTable::contentsDragEnterEvent( QDragEnterEvent *e )
{
    oldCurrentRow = curRow;
    oldCurrentCol = curCol;
    int tmpRow = rowAt( e->pos().y() );
    int tmpCol = columnAt( e->pos().x() );
    fixRow( tmpRow, e->pos().y() );
    fixCol( tmpCol, e->pos().x() );
    setCurrentCell( tmpRow, tmpCol );
    e->accept();
}

/*! This event handler is called whenever a QTable object receives a
  \l QDragMoveEvent \a e, i.e. when the user actually drags the mouse.

  The focus is moved to the cell where the QDragMoveEvent occurred.
*/

void QTable::contentsDragMoveEvent( QDragMoveEvent *e )
{
    int tmpRow = rowAt( e->pos().y() );
    int tmpCol = columnAt( e->pos().x() );
    fixRow( tmpRow, e->pos().y() );
    fixCol( tmpCol, e->pos().x() );
    setCurrentCell( tmpRow, tmpCol );
    e->accept();
}

/*! This event handler is called when a drag activity leaves \e this
  QTable object.
*/

void QTable::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    setCurrentCell( oldCurrentRow, oldCurrentCol );
}

/*! This event handler is called when the user ends a drag and drop
  by dropping something onto \e this QTable and thus
  triggers the drop event, \a e.
*/

void QTable::contentsDropEvent( QDropEvent *e )
{
    setCurrentCell( oldCurrentRow, oldCurrentCol );
    emit dropped( e );
}

/*! If the user presses the mouse on a selected cell, starts moving
 (i.e. dragging), and dragEnabled() is TRUE, this function is called to
 obtain a drag object. A drag using this object begins immediately
 unless dragObject() returns 0.

  By default this function returns 0. You might reimplement it and
  create a QDragObject depending on the selected items.

  \sa dropped()
*/

QDragObject *QTable::dragObject()
{
    return 0;
}

/*! Starts a drag.

  Usually you don't need to call or reimplement this function yourself.

  \sa dragObject();
 */

void QTable::startDrag()
{
    if ( startDragRow == -1 || startDragCol == -1 )
	return;

    startDragRow = startDragCol = -1;
    mousePressed = FALSE;

    QDragObject *drag = dragObject();
    if ( !drag )
	return;

    drag->drag();
}

#endif

/*! \reimp */
void QTable::windowActivationChange( bool )
{
    if ( !isVisible() )
	return;

    const QColorGroup acg = palette().active();
    const QColorGroup icg = palette().inactive();

    if ( acg != icg )
	viewport()->update();
}

/*! \reimp */
void QTable::setEnabled( bool b )
{
    if ( !b ) {
	// editor will lose focus, causing a crash deep in setEnabled(),
	// so we'll end the edit early.
	endEdit( editRow, editCol, TRUE, edMode != Editing );
    }
    QScrollView::setEnabled(b);
}


/* \class QTableHeader qtable.h
  module table

  \brief The QTableHeader class allows for creation and manipulation of
  table headers.

   QTable uses this subclass of QHeader for its headers. QTable has a
   horizontalHeader() for displaying column labels, and a
   verticalHeader() for displaying row labels.

*/

/* \enum QTableHeader::SectionState

  This enum type denotes the state of the header's text

  \value Normal the default
  \value Bold
  \value Selected  typically represented by showing the section "sunken"
  or "pressed in"
*/

/*!
    Creates a new table header called \a name with \a i sections. It is a
    child of widget \a parent and attached to table \a t.
*/

QTableHeader::QTableHeader( int i, QTable *t,
			    QWidget *parent, const char *name )
    : QHeader( i, parent, name ),
      table( t ), caching( FALSE ), numStretches( 0 )
{
    d = 0;
    states.resize( i );
    stretchable.resize( i );
    states.fill( Normal, -1 );
    stretchable.fill( FALSE, -1 );
    mousePressed = FALSE;
    autoScrollTimer = new QTimer( this );
    connect( autoScrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    line1 = new QWidget( table->viewport() );
    line1->hide();
    line1->setBackgroundMode( PaletteText );
    table->addChild( line1 );
    line2 = new QWidget( table->viewport() );
    line2->hide();
    line2->setBackgroundMode( PaletteText );
    table->addChild( line2 );

    connect( this, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( sectionWidthChanged( int, int, int ) ) );
    connect( this, SIGNAL( indexChange( int, int, int ) ),
	     this, SLOT( indexChanged( int, int, int ) ) );

    stretchTimer = new QTimer( this );
    widgetStretchTimer = new QTimer( this );
    connect( stretchTimer, SIGNAL( timeout() ),
	     this, SLOT( updateStretches() ) );
    connect( widgetStretchTimer, SIGNAL( timeout() ),
	     this, SLOT( updateWidgetStretches() ) );
}

/*!
    Adds a new section, \a size pixels wide (or high for vertical
    headers) with the label \a s. If \a size is negative the section's
    size is calculated based on the width (or height) of the label's
    text.
*/

void QTableHeader::addLabel( const QString &s , int size )
{
    states.resize( states.count() + 1 );
    states[ (int)states.count() - 1 ] = Normal;
    stretchable.resize( stretchable.count() + 1 );
    stretchable[ (int)stretchable.count() - 1 ] = FALSE;
    QHeader::addLabel( s , size );
}

/*!
    Sets the SectionState of section \a s to \a astate.

  \sa sectionState()
*/

void QTableHeader::setSectionState( int s, SectionState astate )
{
    if ( s < 0 || s >= (int)states.count() )
	return;
    if ( states.data()[ s ] == astate )
	return;

    states.data()[ s ] = astate;
    if ( isUpdatesEnabled() ) {
	if ( orientation() == Horizontal )
	    repaint( sectionPos( s ) - offset(), 0, sectionSize( s ), height(), FALSE );
	else
	    repaint( 0, sectionPos( s ) - offset(), width(), sectionSize( s ), FALSE );
    }
}

void QTableHeader::setSectionStateToAll( SectionState state )
{
    register int *d = (int *) states.data();
    int n = count();

    while (n >= 4) {
	d[0] = state;
	d[1] = state;
	d[2] = state;
	d[3] = state;
	d += 4;
	n -= 4;
    }

    if (n > 0) {
	d[0] = state;
	if (n > 1) {
	    d[1] = state;
	    if (n > 2) {
		d[2] = state;
	    }
	}
    }
}

/*! Returns the SectionState of section \a s.

  \sa setSectionState()
*/

QTableHeader::SectionState QTableHeader::sectionState( int s ) const
{
    return (QTableHeader::SectionState)states[ s ];
}

/*! \reimp
*/

void QTableHeader::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.setPen( colorGroup().buttonText() );
    int pos = orientation() == Horizontal
		     ? e->rect().left()
		     : e->rect().top();
    int id = mapToIndex( sectionAt( pos + offset() ) );
    if ( id < 0 )
	if ( pos > 0 )
	    return;
	else
	    id = 0;
    for ( int i = id; i < count(); i++ ) {
	QRect r = sRect( i );
	p.save();
	if ( sectionState( i ) == Bold || sectionState( i ) == Selected ) {
	    QFont f( font() );
	    f.setBold( TRUE );
	    p.setFont( f );
	}
	paintSection( &p, i, r );
	p.restore();
	if ( orientation() == Horizontal && r. right() >= e->rect().right() ||
	     orientation() == Vertical && r. bottom() >= e->rect().bottom() )
	    return;
    }
}

/*! \reimp

  Paints the header section with index \a index into the
  rectangular region \a fr on the painter \a p.
*/

void QTableHeader::paintSection( QPainter *p, int index, const QRect& fr )
{
    int section = mapToSection( index );
    if ( section < 0 )
	return;

    if ( sectionState( index ) != Selected ) {
	QHeader::paintSection( p, index, fr );
    } else {
	style().drawComplexControl( QStyle::CC_Header, p, this, QRect(fr.x(), fr.y(), fr.width(), fr.height()),
				    colorGroup(), QStyle::CStyle_Selected );
	paintSectionLabel( p, index, fr );
    }
}

static int real_pos( const QPoint &p, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	return p.x();
    return p.y();
}

/*! \reimp
*/

void QTableHeader::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    QHeader::mousePressEvent( e );
    mousePressed = TRUE;
    pressPos = real_pos( e->pos(), orientation() );
    startPos = -1;
    setCaching( TRUE );
    resizedSection = -1;
#ifdef QT_NO_CURSOR
    isResizing = FALSE;
#else
    isResizing = cursor().shape() != ArrowCursor;
    if ( !isResizing && sectionAt( pressPos ) != -1 )
	doSelection( e );
#endif
}

/*! \reimp
*/

void QTableHeader::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed
#ifndef QT_NO_CURSOR
	|| cursor().shape() != ArrowCursor
#endif
	    || ( ( e->state() & ControlButton ) == ControlButton &&
	   ( orientation() == Horizontal
	     ? table->columnMovingEnabled() : table->rowMovingEnabled() ) ) ) {
	QHeader::mouseMoveEvent( e );
	return;
    }

    if ( !doSelection( e ) )
	QHeader::mouseMoveEvent( e );
}

bool QTableHeader::doSelection( QMouseEvent *e )
{
    int p = real_pos( e->pos(), orientation() ) + offset();
    if ( startPos == -1 ) {
	startPos = p;
	int secAt = sectionAt( p );
	if ( ( e->state() & ControlButton ) != ControlButton
	     || table->selectionMode() == QTable::Single ) {
	    bool upd = TRUE;
	    if ( ( orientation() == Horizontal &&
		   table->isColumnSelected( secAt, TRUE ) ||
		   orientation() == Vertical &&
		   table->isRowSelected( secAt, TRUE ) ) &&
		 table->numSelections() == 1 )
		upd = FALSE;
	    table->viewport()->setUpdatesEnabled( upd );
	    table->clearSelection();
	    table->viewport()->setUpdatesEnabled( TRUE );
	}
	saveStates();
	if ( table->selectionMode() != QTable::NoSelection ) {
	    table->currentSel = new QTableSelection();
	    table->selections.append( table->currentSel );
	    if ( orientation() == Vertical ) {
		table->currentSel->init( secAt, 0 );
	    } else {
		table->currentSel->init( 0, secAt );
	    }
	}
    }
    if ( sectionAt( p ) != -1 )
	endPos = p;
    if ( startPos != -1 ) {
	updateSelections();
	p -= offset();
	if ( orientation() == Horizontal && ( p < 0 || p > width() ) ) {
	    doAutoScroll();
	    autoScrollTimer->start( 100, TRUE );
	} else if ( orientation() == Vertical && ( p < 0 || p > height() ) ) {
	    doAutoScroll();
	    autoScrollTimer->start( 100, TRUE );
	}
	return TRUE;
    }
    return FALSE;
}

/*! \reimp
*/

void QTableHeader::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    autoScrollTimer->stop();
    mousePressed = FALSE;
    bool hasCached = resizedSection != -1;
    setCaching( FALSE );
    QHeader::mouseReleaseEvent( e );
    line1->hide();
    line2->hide();
    if ( hasCached ) {
	emit sectionSizeChanged( resizedSection );
	updateStretches();
    }
}

/*! \reimp
*/

void QTableHeader::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( isResizing ) {
	int p = real_pos( e->pos(), orientation() ) + offset();
	int section = sectionAt( p );
	if ( section == -1 )
	    return;
	section--;
	if ( p >= sectionPos( count() - 1 ) + sectionSize( count() - 1 ) )
	    ++section;
	if ( orientation() == Horizontal ) {
	    table->adjustColumn( section );
	    for ( int i = 0; i < table->numCols(); ++i ) {
		if ( table->isColumnSelected( i ) )
		    table->adjustColumn( i );
	    }
	} else {
	    table->adjustRow( section );
	    for ( int i = 0; i < table->numRows(); ++i ) {
		if ( table->isRowSelected( i ) )
		    table->adjustRow( i );
	    }
	}
    }
}

/*! \reimp
*/

void QTableHeader::resizeEvent( QResizeEvent *e )
{
    stretchTimer->stop();
    widgetStretchTimer->stop();
    QHeader::resizeEvent( e );
    if ( numStretches == 0 )
	return;
    stretchTimer->start( 0, TRUE );
}

void QTableHeader::updateStretches()
{
    if ( numStretches == 0 )
	return;
    if ( orientation() == Horizontal ) {
	if ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) ==
	     width() )
	    return;
	int i;
	int pw = width() -
		 ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) ) -
		 1;
	bool block = signalsBlocked();
	blockSignals( TRUE );
	for ( i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    pw += sectionSize( i );
	}
	pw /= numStretches;
	for ( i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    if ( i == (int)stretchable.count() - 1 &&
		 sectionPos( i ) + pw < width() )
		pw = width() - sectionPos( i );
	    resizeSection( i, QMAX( 20, pw ) );
	}
	blockSignals( block );
	table->viewport()->repaint( FALSE );
	widgetStretchTimer->start( 100, TRUE );
    } else {
	if ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) == height() )
	    return;
	int i;
	int ph = height() - ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) ) - 1;
	bool block = signalsBlocked();
	blockSignals( TRUE );
	for ( i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    ph += sectionSize( i );
	}
	ph /= numStretches;
	for ( i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    if ( i == (int)stretchable.count() - 1 && sectionPos( i ) + ph < height() )
		ph = height() - sectionPos( i );
	    resizeSection( i, QMAX( 20, ph ) );
	}
	blockSignals( block );
	table->viewport()->repaint( FALSE );
	widgetStretchTimer->start( 100, TRUE );
    }
}

void QTableHeader::updateWidgetStretches()
{
    QSize s = table->tableSize();
    table->resizeContents( s.width(), s.height() );
    for ( int i = 0; i < table->numCols(); ++i )
	table->updateColWidgets( i );
}

void QTableHeader::updateSelections()
{
    if ( table->selectionMode() == QTable::NoSelection )
	return;
    int a = sectionAt( startPos );
    int b = sectionAt( endPos );
    int start = QMIN( a, b );
    int end = QMAX( a, b );
    register int *s = states.data();
    for ( int i = 0; i < count(); ++i ) {
	if ( i < start || i > end )
	    *s = oldStates.data()[ i ];
	else
	    *s = Selected;
	++s;
    }
    repaint( FALSE );

    QTableSelection oldSelection = *table->currentSel;
    if ( orientation() == Vertical )
	table->currentSel->expandTo( b, table->horizontalHeader()->count() - 1 );
    else
	table->currentSel->expandTo( table->verticalHeader()->count() - 1, b );
    table->repaintSelections( &oldSelection, table->currentSel,
			      orientation() == Horizontal,
			      orientation() == Vertical );
    emit table->selectionChanged();
}

void QTableHeader::saveStates()
{
    oldStates.resize( count() );
    register int *s = states.data();
    register int *s2 = oldStates.data();
    for ( int i = 0; i < count(); ++i ) {
	*s2 = *s;
	++s2;
	++s;
    }
}

void QTableHeader::doAutoScroll()
{
    QPoint pos = mapFromGlobal( QCursor::pos() );
    int p = real_pos( pos, orientation() ) + offset();
    if ( sectionAt( p ) != -1 )
	endPos = p;
    if ( orientation() == Horizontal )
	table->ensureVisible( endPos, table->contentsY() );
    else
	table->ensureVisible( table->contentsX(), endPos );
    updateSelections();
    autoScrollTimer->start( 100, TRUE );
}

void QTableHeader::sectionWidthChanged( int col, int, int )
{
    resizedSection = col;
    if ( orientation() == Horizontal ) {
	table->moveChild( line1, QHeader::sectionPos( col ) - 1,
			  table->contentsY() );
	line1->resize( 1, table->visibleHeight() );
	line1->show();
	line1->raise();
	table->moveChild( line2,
			  QHeader::sectionPos( col ) + QHeader::sectionSize( col ) - 1,
			  table->contentsY() );
	line2->resize( 1, table->visibleHeight() );
	line2->show();
	line2->raise();
    } else {
	table->moveChild( line1, table->contentsX(),
			  QHeader::sectionPos( col ) - 1 );
	line1->resize( table->visibleWidth(), 1 );
	line1->show();
	line1->raise();
	table->moveChild( line2, table->contentsX(),
			  QHeader::sectionPos( col ) + QHeader::sectionSize( col ) - 1 );
	line2->resize( table->visibleWidth(), 1 );
	line2->show();
	line2->raise();
    }
}

/*! \reimp

  Returns the size of section \a section in pixels or -1 if \a section
  is out of range.
*/

int QTableHeader::sectionSize( int section ) const
{
    if ( count() <= 0 || section < 0 )
	return -1;
    if ( caching )
	return sectionSizes[ section ];
    return QHeader::sectionSize( section );
}

/*! \reimp

  Returns the start position of section \a section in pixels or -1 if \a
  section is out of range.

  \sa sectionAt()
*/

int QTableHeader::sectionPos( int section ) const
{
    if ( count() <= 0 || section < 0 )
	return -1;
    if ( caching )
	return sectionPoses[ section ];
    return QHeader::sectionPos( section );
}

/*! \reimp

  Returns the number of the section at index position \a pos or -1 if
  there is no section at the position given.

  \sa sectionPos()
*/

int QTableHeader::sectionAt( int pos ) const
{
    if ( !caching )
	return QHeader::sectionAt( pos );
    if ( count() <= 0 || pos > sectionPoses[ count() - 1 ] + sectionSizes[ count() - 1 ] )
	return -1;
    int l = 0;
    int r = count() - 1;
    int i = ( (l+r+1) / 2 );
    while ( r - l ) {
	if ( sectionPoses[i] > pos )
	    r = i -1;
	else
	    l = i;
	i = ( (l+r+1) / 2 );
    }
    if ( sectionPoses[i] <= pos &&
	 pos <= sectionPoses[i] + sectionSizes[ mapToSection( i ) ] )
	return mapToSection( i );
    return -1;
}

void QTableHeader::setCaching( bool b )
{
    if ( caching == b )
	return;
    caching = b;
    sectionPoses.resize( count() );
    sectionSizes.resize( count() );
    if ( b ) {
	for ( int i = 0; i < count(); ++i ) {
	    sectionSizes[ i ] = QHeader::sectionSize( i );
	    sectionPoses[ i ] = QHeader::sectionPos( i );
	}
    }
}

/*!
    If \a b is TRUE, section \a s is stretchable; otherwise the section
    is not stretchable.

  \sa isSectionStretchable()
*/

void QTableHeader::setSectionStretchable( int s, bool b )
{
    if ( stretchable[ s ] == b )
	return;
    stretchable[ s ] = b;
    if ( b )
	numStretches++;
    else
	numStretches--;
}

/*!
    Returns TRUE if section \a s is stretcheable; otherwise returns
    FALSE.

  \sa setSectionStretchable()
*/

bool QTableHeader::isSectionStretchable( int s ) const
{
    return stretchable[ s ];
}

void QTableHeader::swapSections( int oldIdx, int newIdx )
{
    QIconSet is;
    bool his = FALSE;
    if ( iconSet( oldIdx ) ) {
	his = TRUE;
	is = *iconSet( oldIdx );
    }
    QString l = label( oldIdx );
    if ( iconSet( newIdx ) )
	setLabel( oldIdx, *iconSet( newIdx ), label( newIdx ) );
    else
	setLabel( oldIdx, label( newIdx ) );

    if ( his )
	setLabel( newIdx, is, l );
    else
	setLabel( newIdx, l );

    int w1 = sectionSize( oldIdx );
    int w2 = sectionSize( newIdx );
    resizeSection( oldIdx, w2 );
    resizeSection( newIdx, w1 );

    if ( orientation() == Horizontal )
	table->swapColumns( oldIdx, newIdx );
    else
	table->swapRows( oldIdx, newIdx );
}

void QTableHeader::indexChanged( int sec, int oldIdx, int newIdx )
{
    newIdx = mapToIndex( sec );
    if ( oldIdx > newIdx )
	moveSection( sec, oldIdx + 1 );
    else
	moveSection( sec, oldIdx );

    if ( oldIdx < newIdx ) {
	while ( oldIdx < newIdx ) {
	    swapSections( oldIdx, oldIdx + 1 );
	    oldIdx++;
	}
    } else {
	while ( oldIdx > newIdx ) {
	    swapSections( oldIdx - 1, oldIdx );
	    oldIdx--;
	}
    }

    table->repaintContents( table->contentsX(), table->contentsY(),
			    table->visibleWidth(), table->visibleHeight() );
}

#include "qtable.moc"

#endif // QT_NO_TABLE

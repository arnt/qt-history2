/****************************************************************************
**
** Implementation of QDataTable class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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

#include "qdatatable.h"

#ifndef QT_NO_SQL

#include "qsqldriver.h"
#include "qsqleditorfactory.h"
#include "qsqlpropertymap.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qpopupmenu.h"
#include "qvaluelist.h"
#include "qmessagebox.h"
#include "qsqlmanager_p.h"

//#define QT_DEBUG_DATATABLE

class QDataTablePrivate
{
public:
    QDataTablePrivate()
	: haveAllRows( FALSE ),
	  continuousEdit( FALSE ),
	  editorFactory( 0 ),
	  propertyMap( 0 ),
	  editRow( -1 ),
	  editCol( -1 ),
	  insertRowLast( -1 ),
	  insertPreRows( -1 ),
	  editBuffer( 0 ),
	  cancelMode( FALSE )
    {}
    ~QDataTablePrivate() { if ( propertyMap ) delete propertyMap; }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;
    bool         continuousEdit;
    QSqlEditorFactory* editorFactory;
    QSqlPropertyMap* propertyMap;
    QString trueTxt;
    QString falseTxt;
    int editRow;
    int editCol;
    int insertRowLast;
    QString insertHeaderLabelLast;
    int insertPreRows;
    QSqlRecord* editBuffer;
    bool cancelMode;
    int lastAt;
    QString ftr;
    QStringList srt;
    QStringList fld;
    QStringList fldLabel;
    QValueList< QIconSet > fldIcon;
    QSqlCursorManager cur;
    QDataManager dat;
};

#ifdef QT_DEBUG_DATATABLE
void qt_debug_buffer( const QString& msg, QSqlRecord* cursor )
{
    qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    qDebug(msg);
    for ( uint j = 0; j < cursor->count(); ++j ) {
	qDebug(cursor->field(j)->name() + " type:" + QString(cursor->field(j)->value().typeName()) + " value:" + cursor->field(j)->value().toString() );
    }
}
#endif


/*! \enum QDataTable::Confirm

  This enum type describes edit confirmations.

  The currently defined values are:

  <ul>
  <li> \c Yes
  <li> \c No
  <li> \c Cancel
  </ul>
*/

/*! \enum QDataTable::Refresh

  This enum type describes edit confirmations.

  The currently defined values are:

  <ul>
  <li> \c Data
  <li> \c Column
  <li> \c All
  </ul>
*/


/*!
  \class QDataTable qdatatable.h
  \module sql

  \brief A flexible SQL table widget that supports browsing and editing.

  QDataTable supports various functions for presenting and editing SQL
  data from a \l QSqlCursor.

  When displaying data, QDataTable only retrieves data for visible rows.
  If the driver supports the 'query size' property the QDataTable will
  have the correct number of rows and the vertical scrollbar will
  accurately reflect the number of rows displayed in proportion to the
  number of rows in the dataset. If the driver does not support the
  'query size' property rows are dynamically fetched from the database
  on an as-needed basis with the scrollbar becoming more accurate as the
  user scrolls down through the records.  This allows extremely large
  queries to be displayed as quickly as possible, with minimum memory
  usage.

  QDataTable inherits QTable's API and extends it with functions to
  sort and filter the data and sort columns. See setCursor(),
  setFilter(), setSort(), setSorting(), sortColumn() and refresh().

  When displaying editable cursors, cell editing will be enabled (for
  more information on editable cursors, see \l QSqlCursor).  QDataTable
  can be used to modify existing data and to enter new records.  When
  a user makes changes to a field in the table, the cursor's edit
  buffer is used.  The table will not send changes in the edit buffer
  to the database until the user moves to a different record in the
  grid.  If there is a problem updating data, errors will be handled
  automatically (see handleError() to change this behavior). QDataTable
  creates editors using the default \l QSqlEditorFactory. Different
  editor factories can be used by calling installEditorFactory(). Cell
  editing is initiated by pressing F2 and cancelled by pressing Esc.

  Columns in the table can be created automatically based on the
  cursor (see setCursor()), or manually (see addColumn() and
  removeColumn()).

  The table automatically uses many of the properties of the cursor to
  format the display of data within cells (alignment, visibility,
  etc.).  However, the filter and sort defined within the table (see
  setFilter() and setSort()) are used instead of the filter and sort
  set on the cursor.  You can change the appearance of cells by
  reimplementing paintField().

*/

/*!  Constructs a table.

*/

QDataTable::QDataTable ( QWidget * parent, const char * name )
    : QTable( parent, name )
{
    init();
}

/*!  Constructs a table using the cursor \a cursor.  If \a
  autoPopulate is TRUE (the default is FALSE), columns are
  automatically created based upon the fields in the \a cursor record.
  Note that \a autoPopulate only governs the creation of columns; to
  load the cursor's data into the table use refresh(). If the \a
  cursor is read only, the table becomes read only.  In addition, the
  table adopts the cursor's driver's definition for representing NULL
  values as strings.
*/

QDataTable::QDataTable ( QSqlCursor* cursor, bool autoPopulate, QWidget * parent, const char * name )
    : QTable( parent, name )
{
    init();
    setCursor( cursor, autoPopulate );
}

/*! \internal
*/


void QDataTable::init()
{
    d = new QDataTablePrivate();
    setFocusProxy( viewport() );
    viewport()->setFocusPolicy( StrongFocus );
    setAutoEdit( FALSE );
    setSelectionMode( NoSelection );
    d->trueTxt = tr( "True" );
    d->falseTxt = tr( "False" );
    reset();
    connect( this, SIGNAL( currentChanged( int, int ) ),
	     SLOT( setCurrentSelection( int, int )));
}

/*! Destroys the object and frees any allocated resources.

*/

QDataTable::~QDataTable()
{
    delete d;
}


/*!  Adds \a fieldName as the next column to be diplayed.  If \a label
  is specified, it is used as the column header label, otherwise the
  field's display label is used when setCursor() is called.

  \sa setCursor() refresh()

*/

void QDataTable::addColumn( const QString& fieldName, const QString& label, const QIconSet& iconset )
{
    d->fld += fieldName;
    d->fldLabel += label;
    d->fldIcon += iconset;
}

/*!  Sets column \a col to display field \a field.  If \a label is
  specified, it is used as the column header label, otherwise the
  field's display label is used when setCursor() is called.

  \sa setCursor() refresh()

*/

void QDataTable::setColumn( uint col, const QString& fieldName, const QString& label, const QIconSet& iconset )
{
    d->fld[col]= fieldName;
    d->fldLabel[col] = label;
    d->fldIcon[col] = iconset;
}

/*!  Removes column \a col from the list of columns to be diplayed.
  If \a col does not exist, nothing happens.

  \sa QSqlField

*/

void QDataTable::removeColumn( uint col )
{
    if ( d->fld.at( col ) != d->fld.end() ) {
	d->fld.remove( d->fld.at( col ) );
	d->fldLabel.remove( d->fldLabel.at( col ) );
	d->fldIcon.remove( d->fldIcon.at( col ) );
    }
}

/*! Returns the current filter used on the displayed data.  If there
  is no current cursor, QString::null is returned.

  \sa setFilter() setCursor()

 */

QString QDataTable::filter() const
{
    return d->cur.filter();
}

/*! Sets the filter to be used on the displayed data to \a filter.  To
  display the filtered data, call refresh(). The text of a filter is the
  SQL for a WHERE clause but without the leading "WHERE" or trailing
  semicolon, e.g. "surname LIKE 'A%'".

  \sa refresh() filter()
*/

void QDataTable::setFilter( const QString& filter )
{
    d->cur.setFilter( filter );
}

/*! Sets the sort to be used on the displayed data to \a sort.  If
  there is no current cursor, nothing happens. To display the
  sorted data, use refresh(). The strings in the sort string list are
  the names of the fields to be sorted; these names are used in the
  ORDER BY clause, e.g.

  \code
    QStringList fields = QStringList() << "duedate" << "amountdue";
    thisTable->setSort( fields );
  \endcode

  will produce an ORDER BY clause like this:

  <tt>ORDER BY cursorname.duedate ASC, cursorname.amountdue ASC</tt>

  If you require DESCending order use the overloaded setSort() that
  takes a QSqlIndex parameter.

  \sa sort()
*/

void QDataTable::setSort( const QStringList& sort )
{
    d->cur.setSort( sort );
}

/*! Sets the sort to be used on the displayed data to \a sort.  If
  there is no current cursor, nothing happens. A QSqlIndex contains
  field names and their ordering (ASC or DESC); these are used to
  compose the ORDER BY clause.

  \sa sort()
*/

void QDataTable::setSort( const QSqlIndex& sort )
{
    d->cur.setSort( sort );
}


/*! Returns the current sort used on the displayed data as a list of
  strings.  Each field is in the form:

  "\a cursorname.\a fieldname ASC" (for ascending sort) or
  "\a cursorname.\a fieldname DESC" (for descending sort)

  If there is no current cursor, an empty string list is returned.

  \sa setSort()

 */

QStringList QDataTable::sort() const
{
    return d->cur.sort();
}

QSqlCursor* QDataTable::sqlCursor() const
{
    return d->cur.cursor();
}

/*! If \a confirm is TRUE, all edit operations (inserts, updates and
  deletes) will be confirmed by the user.  If \a confirm is FALSE (the
  default), all edits are posted to the database immediately.

*/
void QDataTable::setConfirmEdits( bool confirm )
{
    d->dat.setConfirmEdits( confirm );
}

/*! If \a confirm is TRUE, all inserts will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataTable::setConfirmInsert( bool confirm )
{
    d->dat.setConfirmInsert( confirm );
}

/*! If \a confirm is TRUE, all updates will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataTable::setConfirmUpdate( bool confirm )
{
    d->dat.setConfirmUpdate( confirm );
}

/*! If \a confirm is TRUE, all deletes will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataTable::setConfirmDelete( bool confirm )
{
    d->dat.setConfirmDelete( confirm );
}

/*! Returns TRUE if the table confirms all edit operations (inserts,
  updates and deletes), otherwise returns FALSE.
*/

bool QDataTable::confirmEdits() const
{
    return ( d->dat.confirmEdits() );
}

/*! Returns TRUE if the table confirms inserts, otherwise returns
  FALSE.
*/

bool QDataTable::confirmInsert() const
{
    return ( d->dat.confirmInsert() );
}

/*! Returns TRUE if the table confirms updates, otherwise returns
  FALSE.
*/

bool QDataTable::confirmUpdate() const
{
    return ( d->dat.confirmUpdate() );
}

/*! Returns TRUE if the table confirms deletes, otherwise returns
  FALSE.
*/

bool QDataTable::confirmDelete() const
{
    return ( d->dat.confirmDelete() );
}

/*! If \a confirm is TRUE, all cancels will be confirmed by the user
  through a message box.  If \a confirm is FALSE (the default), all
  cancels occur immediately.
*/

void QDataTable::setConfirmCancels( bool confirm )
{
    d->dat.setConfirmCancels( confirm );
}

/*! Returns TRUE if the table confirms cancels, otherwise returns FALSE.
*/

bool QDataTable::confirmCancels() const
{
    return d->dat.confirmCancels();
}

/*!  \reimp

  For an editable table, creates an editor suitable for the field in
  column \a col.  The editor is created using the default editor
  factory, unless a different editor factory was installed using
  installEditorFactory().  The editor is primed with the value of the
  field in \a col using a property map. The property map used is the
  default property map, unless a new property map was installed using
  installPropertMap(). If \a initFromCell is TRUE then the editor is
  primed with the value in the QDataTable cell.

*/

QWidget * QDataTable::createEditor( int , int col, bool initFromCell ) const
{
    if ( d->dat.mode() == QSql::None )
	return 0;

    QSqlEditorFactory * f = (d->editorFactory == 0) ?
		     QSqlEditorFactory::defaultFactory() : d->editorFactory;

    QSqlPropertyMap * m = (d->propertyMap == 0) ?
			  QSqlPropertyMap::defaultMap() : d->propertyMap;

    QWidget * w = 0;
    if( initFromCell && d->editBuffer ){
	w = f->createEditor( viewport(), d->editBuffer->field( indexOf( col ) ) );
	if ( w )
	    m->setProperty( w, d->editBuffer->value( indexOf( col ) ) );
    }
    return w;
}

/*! \reimp
*/

bool QDataTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QTable::eventFilter( o, e );

    if ( d->cancelMode )
	return TRUE;

    int r = currentRow();
    int c = currentColumn();

    if ( d->dat.mode() != QSql::None ) {
	r = d->editRow;
	c = d->editCol;
    }

    bool insertCancelled = FALSE;
    QWidget *editorWidget = cellWidget( r, c );
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	int conf = QSql::Yes;
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Escape && d->dat.mode() == QSql::Insert ){
	    if ( confirmCancels() && !d->cancelMode ) {
		d->cancelMode = TRUE;
		conf = confirmCancel( QSql::Insert );
		d->cancelMode = FALSE;
	    }
	    if ( conf == QSql::Yes ) {
		insertCancelled = TRUE;
		endInsert();
	    } else {
		editorWidget->setActiveWindow();
		editorWidget->setFocus();
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Escape && d->dat.mode() == QSql::Update ) {
	    if ( confirmCancels() && !d->cancelMode ) {
		d->cancelMode = TRUE;
		conf = confirmCancel( QSql::Update );
		d->cancelMode = FALSE;
	    }
	    if ( conf == QSql::Yes ){
		endUpdate();
	    } else {
		editorWidget->setActiveWindow();
		editorWidget->setFocus();
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Insert && d->dat.mode() == QSql::None ) {
	    beginInsert();
	    return TRUE;
	}
	if ( ke->key() == Key_Delete && d->dat.mode() == QSql::None ) {
	    deleteCurrent();
	    return TRUE;
	}
	if ( d->dat.mode() != QSql::None ) {
	    if ( ( ke->key() == Key_Tab ) && ( c < numCols() - 1 ) ) {
		d->continuousEdit = TRUE;
	    } else if ( ( ke->key() == Key_BackTab ) && ( c > 0 ) ) {
		d->continuousEdit = TRUE;
	    } else {
		d->continuousEdit = FALSE;
	    }
	}
	break;
    }
    case QEvent::FocusOut:
	repaintCell( currentRow(), currentColumn() );
	if ( !d->cancelMode && editorWidget && o == editorWidget &&
	     ( d->dat.mode() == QSql::Insert) && !d->continuousEdit) {
	    setCurrentCell( r, c );
	    endEdit( r, c, autoEdit(), FALSE );
	    return TRUE;
	}
	break;
    case QEvent::FocusIn:
	repaintCell( currentRow(), currentColumn() );
	break;
    default:
	break;
    }
    bool b = QTable::eventFilter( o, e );
    if ( insertCancelled ) {
	setNumRows( d->insertPreRows );
	d->insertPreRows = -1;
	viewport()->setFocus();
    }
    return b;
}

/*!  \reimp
*/

void QDataTable::resizeEvent ( QResizeEvent * e )
{

    if ( sqlCursor() && !sqlCursor()->driver()->hasQuerySizeSupport() )
	loadNextPage();
    QTable::resizeEvent( e );
}

/*!  \reimp
*/

void QDataTable::contentsMousePressEvent( QMouseEvent* e )
{
    if ( d->dat.mode() != QSql::None )
	endEdit( d->editRow, d->editCol, autoEdit(), FALSE );
    if ( !sqlCursor() ) {
	QTable::contentsMousePressEvent( e );
	return;
    }
    if ( e->button() == RightButton && d->dat.mode() == QSql::None ) {
	if ( isReadOnly() )
	    return;
	enum {
	    IdInsert,
	    IdUpdate,
	    IdDelete
	};
	QPopupMenu *popup = new QPopupMenu( this );
	int id[ 3 ];
	id[ IdInsert ] = popup->insertItem( tr( "Insert" ) );
	id[ IdUpdate ] = popup->insertItem( tr( "Update" ) );
	id[ IdDelete ] = popup->insertItem( tr( "Delete" ) );
	bool enableInsert = sqlCursor()->canInsert();
	popup->setItemEnabled( id[ IdInsert ], enableInsert );
	bool enableUpdate = currentRow() > -1 && sqlCursor()->canUpdate();
	popup->setItemEnabled( id[ IdUpdate ], enableUpdate );
	bool enableDelete = currentRow() > -1 && sqlCursor()->canDelete();
	popup->setItemEnabled( id[ IdDelete ], enableDelete );
	int r = popup->exec( e->globalPos() );
	delete popup;
	if ( r == id[ IdInsert ] )
	    beginInsert();
	else if ( r == id[ IdUpdate ] ) {
	    if ( beginEdit( currentRow(), currentColumn(), FALSE ) )
		setEditMode( Editing, currentRow(), currentColumn() );
	    else
		endUpdate();
	}
	else if ( r == id[ IdDelete ] )
	    deleteCurrent();
	return;
    }
    if ( d->dat.mode() == QSql::None )
	QTable::contentsMousePressEvent( e );

}

/*!  \reimp
*/

QWidget* QDataTable::beginEdit ( int row, int col, bool replace )
{
    d->editRow = -1;
    d->editCol = -1;
    if ( !sqlCursor() )
	return 0;
    if ( d->dat.mode() == QSql::Insert && !sqlCursor()->canInsert() )
	return 0;
    if ( d->dat.mode() == QSql::Update && !sqlCursor()->canUpdate() )
	return 0;
    d->editRow = row;
    d->editCol = col;
    if ( d->continuousEdit ) {
	QWidget* w = QTable::beginEdit( row, col, replace );
	return w;
    }
    if ( d->dat.mode() == QSql::None && sqlCursor()->canUpdate() && sqlCursor()->primaryIndex().count() > 0 )
	return beginUpdate( row, col, replace );
    return 0;
}

/*! \reimp
*/

void QDataTable::endEdit( int row, int col, bool accept, bool )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;
    if ( d->cancelMode )
	return;
    if ( !accept ) {
	setEditMode( NotEditing, -1, -1 );
	clearCellWidget( row, col );
	updateCell( row, col );
	return;
    }
    if ( d->dat.mode() != QSql::None && d->editBuffer ) {
	QSqlPropertyMap * m = (d->propertyMap == 0) ?
			      QSqlPropertyMap::defaultMap() : d->propertyMap;
	d->editBuffer->setValue( indexOf( col ),  m->property( editor ) );
	clearCellWidget( row, col );
	if ( !d->continuousEdit ) {
	    switch ( d->dat.mode() ) {
	    case QSql::Insert:
		insertCurrent();
		break;
	    case QSql::Update:
		updateCurrent();
		break;
	    default:
		break;
	    }
	}
    } else {
	setEditMode( NotEditing, -1, -1 );
    }
    if ( d->dat.mode() == QSql::None ) {
	viewport()->setFocus();
    }
    updateCell( row, col );
    emit valueChanged( row, col );
}

/*! \reimp
*/
void QDataTable::activateNextCell()
{
//     if ( d->dat.mode() == QSql::None )
//	QTable::activateNextCell();
}

/*! \internal
*/

void QDataTable::endInsert()
{
    d->dat.setMode( QSql::None );
    int i;
    d->editBuffer = 0;
    for ( i = d->editRow; i <= d->insertRowLast; ++i )
	updateRow( i );
    for ( i = d->editRow; i < d->insertRowLast; ++i )
	verticalHeader()->setLabel( i, verticalHeader()->label( i+1 ) );
    verticalHeader()->setLabel( d->insertRowLast, d->insertHeaderLabelLast );
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
}

/*! \internal
*/

void QDataTable::endUpdate()
{
    d->dat.setMode( QSql::None );
    d->editBuffer = 0;
    updateRow( d->editRow );
    d->editRow = -1;
    d->editCol = -1;
}

/*! Protected virtual function called when editing is about to begin on
   a new record.  If the table is read-only, or the cursor does not
   allow inserts, nothing happens.

   Editing takes place using the cursor's edit buffer (see
   QSqlCursor::editBuffer()).

   When editing begins, a new row is created in the table marked with
   a '*' in the row's vertical header column.

*/

bool QDataTable::beginInsert()
{
    if ( !sqlCursor() || isReadOnly() || !numCols() )
	return FALSE;
    if ( !sqlCursor()->canInsert() )
	return FALSE;
    int i = 0;
    int row = currentRow();
    d->insertPreRows = numRows();
    if ( row < 0 || numRows() < 1 )
	row = 0;
    setNumRows( d->insertPreRows + 1 );
    setCurrentCell( row, 0 );
    d->editBuffer = sqlCursor()->primeInsert();
    emit primeInsert( d->editBuffer );
    d->dat.setMode( QSql::Insert );
    int lastRow = row;
    int lastY = contentsY() + visibleHeight();
    for ( i = row; i < numRows() ; ++i ) {
	QRect cg = cellGeometry( i, 0 );
	if ( (cg.y()+cg.height()) > lastY ) {
	    lastRow = i;
	    break;
	}
    }
    if ( lastRow == row && ( numRows()-1 > row ) )
	lastRow = numRows() - 1;
    d->insertRowLast = lastRow;
    d->insertHeaderLabelLast = verticalHeader()->label( d->insertRowLast );
    for ( i = lastRow; i > row; --i )
	verticalHeader()->setLabel( i, verticalHeader()->label( i-1 ) );
    verticalHeader()->setLabel( row, "*" );
    d->editRow = row;
    for ( i = row; i <= d->insertRowLast; ++i )
	updateRow( i );
    if ( QTable::beginEdit( row, 0, FALSE ) )
	setEditMode( Editing, row, 0 );
    return TRUE;
}

/*! Protected virtual function called when editing is about to begin on
   an existing row.  If the table is read-only, nothing happens.

   Editing takes place using the cursor's edit buffer (see
   QSqlCursor::editBuffer()).

   \a row and \a col refer to the row and column in the QDataTable.

   (\a replace is provided for reimplementors and reflects the API of
   QTable::beginEdit().)

*/

QWidget* QDataTable::beginUpdate ( int row, int col, bool replace )
{
    if ( !sqlCursor() || isReadOnly() )
	return 0;
    setCurrentCell( row, col );
    d->dat.setMode( QSql::Update );
    if ( sqlCursor()->seek( row ) ) {
	d->editBuffer = sqlCursor()->primeUpdate();
	emit primeUpdate( d->editBuffer );
	return QTable::beginEdit( row, col, replace );
    }
    return 0;
}

/*!  For an editable table, issues an insert on the current cursor
  using the values of the cursor's edit buffer. If there is no current
  cursor or there is no current "insert" row, nothing happens.  If
  confirmEdits() or confirmInsert() is TRUE, confirmEdit() is called
  to confirm the insert. Returns TRUE if the insert succeeded,
  otherwise returns FALSE.

*/

void QDataTable::insertCurrent()
{
    if ( d->dat.mode() != QSql::Insert || ! numCols() )
	return;
    if ( !sqlCursor()->canInsert() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::insertCurrent: insert not allowed for " + sqlCursor()->name() );
#endif
	endInsert();
	return;
    }
    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmInsert() )
	conf = confirmEdit( QSql::Insert );
    switch ( conf ) {
    case QSql::Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeInsert( d->editBuffer );
	b = sqlCursor()->insert();
	QApplication::restoreOverrideCursor();
	if ( !b || !sqlCursor()->isActive() ) {
	    handleError( sqlCursor()->lastError() );
	    refresh();
	    if ( QTable::beginEdit( currentRow(), currentColumn(), FALSE ) )
		setEditMode( Editing, currentRow(), currentColumn() );
	} else {
	    QSqlIndex idx = sqlCursor()->primaryIndex( TRUE );
	    refresh();
	    findBuffer( idx, d->lastAt );
	    emit cursorChanged( QSql::Insert );
	    endInsert();
	    setEditMode( NotEditing, -1, -1 );
	    setCurrentCell( currentRow(), currentColumn() );
	}
	break;
    }
    case QSql::No:
	endInsert();
	setEditMode( NotEditing, -1, -1 );
	break;
    case QSql::Cancel:
	if ( QTable::beginEdit( currentRow(), currentColumn(), FALSE ) )
	    setEditMode( Editing, currentRow(), currentColumn() );
	break;
    }
    return;
}

/*! \internal
*/

void QDataTable::updateRow( int row )
{
    for ( int i = 0; i < numCols(); ++i )
	updateCell( row, i );
}

/*!  For an editable table, issues an update using the cursor's edit
  buffer.  If there is no current cursor or there is no current
  selection, nothing happens.  If confirmEdits() or confirmUpdate() is
  TRUE, confirmEdit() is called to confirm the update. Returns TRUE if
  the update succeeded, otherwise returns FALSE.

  The underlying cursor must have a valid primary index to ensure that a
  unique record is updated within the database otherwise the database
  may be changed to an inconsistent state.

*/

void QDataTable::updateCurrent()
{
    if ( d->dat.mode() != QSql::Update )
	return;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::updateCurrent: no primary index for " + sqlCursor()->name() );
#endif
	endUpdate();
	return;
    }
    if ( !sqlCursor()->canUpdate() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::updateCurrent: updates not allowed for " + sqlCursor()->name() );
#endif
	endUpdate();
	return;
    }
    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmUpdate() )
	conf = confirmEdit( QSql::Update );
    switch ( conf ) {
    case QSql::Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeUpdate( d->editBuffer );
	b = sqlCursor()->update();
	QApplication::restoreOverrideCursor();
	if ( !b || !sqlCursor()->isActive() ) {
	    handleError( sqlCursor()->lastError() );
	    refresh();
	    setCurrentCell( d->editRow, d->editCol );
	    if ( QTable::beginEdit( d->editRow, d->editCol, FALSE ) )
		setEditMode( Editing, d->editRow, d->editCol );
	} else {
	    emit cursorChanged( QSql::Update );
	    QSqlIndex idx = sqlCursor()->primaryIndex( TRUE );
	    refresh();
	    findBuffer( idx, d->lastAt );
	    endUpdate();
	}
	break;
    }
    case QSql::No:
	endUpdate();
	setEditMode( NotEditing, -1, -1 );
	break;
    case QSql::Cancel:
	setCurrentCell( d->editRow, d->editCol );
	if ( QTable::beginEdit( d->editRow, d->editCol, FALSE ) )
	    setEditMode( Editing, d->editRow, d->editCol );
	break;
    }
    return;
}

/*!  For an editable table, issues a delete on the current cursor's
  primary index using the values of the currently selected row.  If
  there is no current cursor or there is no current selection, nothing
  happens. If confirmEdits() or confirmDelete() is TRUE, confirmEdit()
  is called to confirm the delete. Returns TRUE if the delete
  succeeded, otherwise FALSE.

  The underlying cursor must have a valid primary index to ensure that a
  unique record is deleted within the database otherwise the database
  may be changed to an inconsistent state.

*/

void QDataTable::deleteCurrent()
{
    if ( !sqlCursor() || isReadOnly() )
	return;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::deleteCurrent: no primary index " + sqlCursor()->name() );
#endif
	return;
    }
    if ( !sqlCursor()->canDelete() )
	return;

    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmDelete() )
	conf = confirmEdit( QSql::Delete );

    // Have to have this here - the confirmEdit() might pop up a
    // dialog that causes a repaint which the cursor to the
    // record it has to repaint.
    if ( !sqlCursor()->seek( currentRow() ) )
	return;
    switch ( conf ) {
	case QSql::Yes:{
	QApplication::setOverrideCursor( Qt::waitCursor );
	sqlCursor()->primeDelete();
	emit primeDelete( sqlCursor()->editBuffer() );
	emit beforeDelete( sqlCursor()->editBuffer() );
	b = sqlCursor()->del();
	QApplication::restoreOverrideCursor();
	if ( !b )
	    handleError( sqlCursor()->lastError() );
	refresh();
	emit cursorChanged( QSql::Delete );
	setCurrentCell( currentRow(), currentColumn() );
	updateRow( currentRow() );
	}
	break;
    case QSql::No:
	setEditMode( NotEditing, -1, -1 );
	break;
    }
    return;
}

/*!  Protected virtual function which returns a confirmation for an
  edit of mode \a m.  Derived classes can reimplement this function
  and provide their own confirmation dialog.  The default
  implementation uses a message box which prompts the user to confirm
  the edit action.

*/

QSql::Confirm QDataTable::confirmEdit( QSql::Op m )
{
    return d->dat.confirmEdit( this, m );
}

/*!  Protected virtual function which returns a confirmation for
   cancelling an edit mode \a m.  Derived classes can reimplement this
   function and provide their own confirmation dialog.  The default
   implementation uses a message box which prompts the user to confirm
   the edit action.

*/

QSql::Confirm  QDataTable::confirmCancel( QSql::Op m )
{
    return d->dat.confirmCancel( this, m );
}


/*! Searches the current cursor for a cell containing the string \a str
    starting at the current cell and working forwards (or backwards if
    \a backwards is TRUE). If the string is found, the cell containing
    the string is set as the current cell. If \a caseSensitive is FALSE
    the case of \a str will be ignored.

    The search will wrap, i.e. if the first (or if backwards is TRUE,
    last) cell is reached without finding \a str the search will
    continue until it reaches the starting cell. If \a str is not found
    the search will fail and the current cell will remain unchanged.
*/
void QDataTable::find( const QString & str, bool caseSensitive, bool backwards )
{
    if ( !sqlCursor() )
	return;

    QSqlCursor * r = sqlCursor();
    QString tmp, text;
    uint  row = currentRow(), startRow = row,
	  col = backwards ? currentColumn() - 1 : currentColumn() + 1;
    bool  wrap = TRUE, found = FALSE;

    if( str.isEmpty() || str.isNull() )
	return;

    if( !caseSensitive )
	tmp = str.lower();
    else
	tmp = str;

    QApplication::setOverrideCursor( Qt::waitCursor );
    while( wrap ){
	while( !found && r->seek( row ) ){
	    for( int i = col; backwards ? (i >= 0) : (i < (int) numCols());
		 backwards ? i-- : i++ )
	    {
		text = r->value( indexOf( i ) ).toString();
		if( !caseSensitive ){
		    text = text.lower();
		}
		if( text.contains( tmp ) ){
		    setCurrentCell( row, i );
		    col = i;
		    found = TRUE;
		}
	    }
	    if( !backwards ){
		col = 0;
		row++;
	    } else {
		col = numCols() - 1;
		row--;
	    }
	}
	if( !backwards ){
	    if( startRow != 0 ){
		startRow = 0;
	    } else {
		wrap = FALSE;
	    }
	    r->first();
	    row = 0;
	} else {
	    if( startRow != (uint) (numRows() - 1) ){
		startRow = numRows() - 1;
	    } else {
		wrap = FALSE;
	    }
	    r->last();
	    row = numRows() - 1;
	}
    }
    QApplication::restoreOverrideCursor();
}


/*!  Resets the table so that it displays no data.

  \sa setCursor()

*/

void QDataTable::reset()
{
    clearCellWidget( currentRow(), currentColumn() );
    switch ( d->dat.mode() ) {
    case QSql::Insert:
	endInsert();
	break;
    case QSql::Update:
	endUpdate();
	break;
    default:
	break;
    }
    ensureVisible( 0, 0 );
    verticalScrollBar()->setValue(0);
    setNumRows(0);

    d->haveAllRows = FALSE;
    d->continuousEdit = FALSE;
    d->dat.setMode( QSql::None );
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    d->cancelMode = FALSE;
    d->lastAt = -1;
    d->fld.clear();
    d->fldLabel.clear();
    d->fldIcon.clear();
    if ( sorting() )
	horizontalHeader()->setSortIndicator( -1 );
}

/*!  Returns the index of the field within the current SQL query that is
  displayed in column \a i.

*/

int QDataTable::indexOf( uint i ) const
{
    QDataTablePrivate::ColIndex::ConstIterator it = d->colIndex.at( i );
    if ( it != d->colIndex.end() )
	return *it;
    return -1;
}

/*! Returns TRUE if the table will automatically delete the cursor
  specified by setCursor() otherwise returns FALSE.
*/

bool QDataTable::autoDelete() const
{
    return d->cur.autoDelete();
}

/*! Sets the auto-delete flag to \a enable.  If \a enable is TRUE, the
  table will automatically delete the cursor specified by setCursor().
  Otherwise, (the default) the cursor will not be deleted.
*/

void QDataTable::setAutoDelete( bool enable )
{
    d->cur.setAutoDelete( enable );
}

/*!  Sets the auto-edit property of the table to \a auto. The
  default is FALSE. ### more info

*/

void QDataTable::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}


/*! Returns TRUE if the auto-edit property is on, otherwise FALSE is
  returned.

*/

bool QDataTable::autoEdit() const
{
    return d->dat.autoEdit();
}

/*!  Sets the text to be displayed when a NULL value is encountered in
  the data to \a nullText.  The default value is specified by the
  cursor's driver.

*/

void QDataTable::setNullText( const QString& nullText )
{
    d->nullTxt = nullText;
}

/*!  Returns the text to be displayed when a NULL value is encountered
  in the data.

*/

QString QDataTable::nullText() const
{
    return d->nullTxt;
}

/*!  Sets the text to be displayed when a TRUE bool value is
  encountered in the data to \a trueText.  The default is 'True'.

*/

void QDataTable::setTrueText( const QString& trueText )
{
    d->trueTxt = trueText;
}

/*!  Returns the text to be displayed when a TRUE bool value is
  encountered in the data.

*/

QString QDataTable::trueText() const
{
    return d->trueTxt;
}

/*!  Sets the text to be displayed when a FALSE bool value is
  encountered in the data to \a falseText.  The default is 'False'.

*/

void QDataTable::setFalseText( const QString& falseText )
{
    d->falseTxt = falseText;
}


/*!  Returns the text to be displayed when a FALSE bool value is
  encountered in the data.

*/

QString QDataTable::falseText() const
{
    return d->falseTxt;
}

/*!  \reimp
*/

int QDataTable::numRows() const
{
    return QTable::numRows();
}

/*!  \reimp

  The number of rows in the table will be determined by the cursor
  (see setCursor()), so normally this function should never be called.
  It is included for completeness.
*/

void QDataTable::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

/*!  \reimp

  The number of columns in the table will be determined automatically
  (see addColumn()), so normally this function should never be called.
  It is included for completeness.
*/

void QDataTable::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

/*!  \reimp
*/

int QDataTable::numCols() const
{
    return QTable::numCols();
}

/*!  Returns the text in cell \a row, \a col, or an empty string if
  the cell is empty. If the cell's value is NULL then it's nullText()
  will be returned. If the cell does not exist then a null QString is
  returned.

*/

QString QDataTable::text ( int row, int col ) const
{
    if ( !sqlCursor() )
	return QString::null;
    if ( sqlCursor()->seek( row ) )
	return sqlCursor()->value( indexOf( col ) ).toString();
    return QString::null;
}

/*!  Returns the value in cell \a row, \a col, or an invalid value if
   the cell does not exist or has no value.

*/

QVariant QDataTable::value ( int row, int col ) const
{
    if ( !sqlCursor() )
	return QVariant();
    if ( sqlCursor()->seek( row ) )
	return sqlCursor()->value( indexOf( col ) );
    return QVariant();
}

/*!  \internal
  Used to update the table when there is no way of knowing the size of
  the result set - divide the result set into pages and load the pages
  as the user moves around in the table.
*/
void QDataTable::loadNextPage()
{
    if ( d->haveAllRows )
	return;
    if ( !sqlCursor() )
	return;
    int pageSize = 0;
    int lookAhead = 0;
    if ( height() ) {
	pageSize = (int)( height() * 2 / 20 );
	lookAhead = pageSize / 2;
    }
    int startIdx = verticalScrollBar()->value() / 20;
    int endIdx = startIdx + pageSize + lookAhead;
    if ( endIdx < numRows() || endIdx < 0 )
	return;

    while ( endIdx > 0 && !sqlCursor()->seek( endIdx ) )
	endIdx--;
    if ( endIdx != ( startIdx + pageSize + lookAhead ) )
	d->haveAllRows = TRUE;
    setNumRows( endIdx + 1 );
}

/*! \internal
*/

void QDataTable::loadLine( int )
{
    loadNextPage();
}

/*!  Sorts the column \a col in ascending order if \a ascending is
  TRUE, otherwise in descending order. The \a wholeRows parameter is
  ignored for SQL tables.

*/

void QDataTable::sortColumn ( int col, bool ascending,
			      bool  )
{
    if ( sorting() ) {
	if ( !sqlCursor() )
	    return;
	QSqlIndex lastSort = sqlCursor()->sort();
	QSqlIndex newSort( lastSort.cursorName(), "newSort" );
	newSort.append( *sqlCursor()->field( indexOf( col ) ) );
	newSort.setDescending( 0, !ascending );
	horizontalHeader()->setSortIndicator( col, ascending );
	setSort( newSort );
	refresh();
    }
}

/*!  \reimp
*/

void QDataTable::columnClicked ( int col )
{
    if ( sorting() ) {
	if ( !sqlCursor() )
	    return;
	QSqlIndex lastSort = sqlCursor()->sort();
	bool asc = TRUE;
	if ( lastSort.count() && lastSort.field( 0 )->name() == sqlCursor()->field( indexOf( col ) )->name() )
	    asc = lastSort.isDescending( 0 );
	sortColumn( col, asc );
    }
}

/*!  \reimp
*/
void QDataTable::repaintCell( int row, int col )
{
    QRect cg = cellGeometry( row, col );
    QRect re( QPoint( cg.x() - 2, cg.y() - 2 ),
	      QSize( cg.width() + 4, cg.height() + 4 ) );
    repaintContents( re, FALSE );
}

/*! \reimp

  This function renders the cell at \a row, \a col with the value of
  the corresponding cursor field on the painter \a p.  Depending on the
  current edit mode of the table, paintField() is called for the
  appropriate cursor field. \a cr describes the cell coordinates in the
  content coordinate system. If \a selected is TRUE the cell has been
  selected and would normally be rendered differently than an unselected
  cell.

  \sa QSql::isNull()
*/

void QDataTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QTable::paintCell(p,row,col,cr, false);  // empty cell

    if( hasFocus() && (row == currentRow()) && (col == currentColumn()) ){
	p->fillRect( 1, 1, cr.width() - 3, cr.height() - 3,
		     colorGroup().brush( QColorGroup::Highlight ) );
	p->setPen( colorGroup().highlightedText() );
	p->drawRect( 1,1, cr.width()-3,cr.height()-3 );
    } else {
	p->setPen( colorGroup().text() );
    }

    if ( !sqlCursor() )
	return;

    if ( d->dat.mode() != QSql::None ) {
	if ( row == d->editRow && d->editBuffer ) {
	    paintField( p, d->editBuffer->field( indexOf( col ) ), cr,
			selected );
	} else if ( row > d->editRow && d->dat.mode() == QSql::Insert ) {
	    if ( sqlCursor()->seek( row - 1 ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	} else {
	    if ( sqlCursor()->seek( row ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	}
    } else {
	if ( sqlCursor()->seek( row ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr, selected );

    }
}


/*! Paints the \a field on the painter \a p. The painter has already
   been translated to the appropriate cell's origin where the \a field
   is to be rendered. \a cr describes the cell coordinates in the content
   coordinate system.

   If you want to draw custom field content you have to reimplement
   paintField() to do the custom drawing.  The default implementation
   renders the \a field value as text.  If the field is NULL,
   nullText() is displayed in the cell.  If the field is Boolean,
   trueText() or falseText() is displayed as appropriate.

*/

void QDataTable::paintField( QPainter * p, const QSqlField* field,
			    const QRect & cr, bool )
{
    // ###

    if ( !field )
	return;
    QString text;
    if ( field->isNull() ) {
	text = nullText();
    } else {
	const QVariant val = field->value();
	text = val.toString();
	if ( val.type() == QVariant::Bool )
	    text = val.toBool() ? d->trueTxt : d->falseTxt;
    }
    p->drawText( 2,2, cr.width()-4, cr.height()-4, fieldAlignment( field ), text );
}

/*! Returns the alignment for \a field.
*/

int QDataTable::fieldAlignment( const QSqlField* /*field*/ )
{
    return Qt::AlignLeft | Qt::AlignVCenter; //## Reggie: add alignment to QTable
}


/*!  If the \a sql driver supports query sizes, the number of rows in
  the table is set to the size of the query.  Otherwise, the table
  dynamically resizes itself as it is scrolled.  If \q sql is not
  active, it is made active by issuing a select() on the cursor using
  the current filter and current sort of \a sql.

*/

void QDataTable::setSize( QSqlCursor* sql )
{
    // ##is this required anymore?
//     if ( !sql->isActive() ) {
//	sql->select( sql->filter(), sql->sort() );
//     }
    // ### what are the connect/disconnect calls doing here!? move to refresh()
    if ( sql->driver()->hasQuerySizeSupport() ) {
	setVScrollBarMode( Auto );
	disconnect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		 this, SLOT( loadLine(int) ) );
	setNumRows( sql->size() );
    } else {
	setVScrollBarMode( AlwaysOn );
	connect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		 this, SLOT( loadLine(int) ) );
	verticalScrollBar()->setValue( 0 );
	setNumRows( 0 );
	loadNextPage();
    }
}

/*!  Sets \a cursor as the data source for the table.  To force the
  display of the data from \a cursor, use refresh(). If \a
  autoPopulate is TRUE, columns are automatically created based upon
  the fields in the \a cursor record.  If \a autoDelete is TRUE (the
  default is FALSE), the table will take ownership of the \a cursor
  and delete it when appropriate.  If the \a cursor is read only, the
  table becomes read only.  The table adopts the cursor's driver's
  definition for representing NULL values as strings.

  \sa refresh() setReadOnly() setAutoDelete() QSqlDriver::nullText()

*/

void QDataTable::setCursor( QSqlCursor* cursor, bool autoPopulate, bool autoDelete )
{
    setUpdatesEnabled( FALSE );
    d->cur.setCursor( 0 );
    if ( cursor ) {
	d->cur.setCursor( cursor, autoDelete );
	if ( autoPopulate ) {
	    d->fld.clear();
	    d->fldLabel.clear();
	    d->fldIcon.clear();
	    for ( uint i = 0; i < sqlCursor()->count(); ++i )
		addColumn( sqlCursor()->field( i )->name(), sqlCursor()->field( i )->name() ); //## algorithm for betten display label
	}
	if ( sqlCursor()->isReadOnly() )
	    setReadOnly( TRUE );
	setNullText(sqlCursor()->driver()->nullText() );
	setAutoDelete( autoDelete );
    }
    setUpdatesEnabled( TRUE );
}


/*!  Protected virtual function which is called when an error has
  occurred on the current cursor().  The default implementation
  displays a warning message to the user with information about the
  error.

*/
void QDataTable::handleError( const QSqlError& e )
{
    QMessageBox::warning ( this, "Warning", e.driverText() + "\n" + e.databaseText(), 0, 0 );
}

/*!  \reimp
*/

void QDataTable::resizeData ( int )
{

}

/*!  \reimp
*/

QTableItem * QDataTable::item ( int, int ) const
{
    return 0;
}

/*!  \reimp
*/

void QDataTable::setItem ( int , int , QTableItem * )
{

}

/*!  \reimp
*/

void QDataTable::clearCell ( int , int )
{

}

/*!  \reimp
*/

void QDataTable::setPixmap ( int , int , const QPixmap &  )
{

}

/*!  \reimp
*/

void QDataTable::takeItem ( QTableItem * )
{

}

/*!  Installs a new SQL editor factory. This enables the user to
  create and instantiate their own editors for use in cell editing.
  Note that QDataTable takes ownership of this pointer, and will delete
  it when it is no longer needed or when installEditorFactory() is
  called again.

  \sa QSqlEditorFactory
*/

void QDataTable::installEditorFactory( QSqlEditorFactory * f )
{
    if( f ) {
	delete d->editorFactory;
	d->editorFactory = f;
    }
}

/*!  Installs a new property map. This enables the user to create and
  instantiate their own property maps for use in cell editing.  Note
  that QDataTable takes ownership of this pointer, and will delete it
  when it is no longer needed or when installPropertMap() is called
  again.

  \sa QSqlPropertyMap

*/

void QDataTable::installPropertyMap( QSqlPropertyMap* m )
{
    if ( m ) {
	delete d->propertyMap;
	d->propertyMap = m;
    }
}

/*!  \internal
*/

void QDataTable::setCurrentSelection( int row, int )
{
    if ( !sqlCursor() )
	return;
    if ( row == d->lastAt )
	return;
    if ( !sqlCursor()->seek( row ) )
	return;
    d->lastAt = row;
    emit currentChanged( sqlCursor() );
}

/*!  Returns a pointer to the currently selected record, or an 0 if
  there is no current selection.  The table own the pointer, so do not
  delete it.

*/

QSqlRecord* QDataTable::currentRecord() const
{
    if ( !sqlCursor() || currentRow() < 0 )
	return 0;
    return sqlCursor();
}

/*! Sorts column \a col in ascending order.

  \sa setSorting()
*/

void QDataTable::sortAscending( int col )
{
    sortColumn( col, TRUE );
}

/*! Sorts column \a col in descending order.

  \sa setSorting()
*/

void QDataTable::sortDescending( int col )
{
    sortColumn( col, FALSE );
}

/*! Refreshes the table.  If there is no currently defined cursor (see
  setCursor()), nothing happens. The \mode paramaeter describes the
  type of refresh that will take place.

  \sa Refresh setCursor() addColumn()
*/

void QDataTable::refresh( QDataTable::Refresh mode )
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return;
    bool refreshData = ( (mode & RefreshData) == RefreshData );
    bool refreshCol = ( (mode & RefreshColumns) == RefreshColumns );
    if ( ( (mode & RefreshAll) == RefreshAll ) ) {
	refreshData = TRUE;
	refreshCol = TRUE;
    }
    viewport()->setUpdatesEnabled( FALSE );
    d->haveAllRows = FALSE;
    if ( refreshData )
	d->cur.refresh();
    if ( refreshCol ) {
	setNumCols( 0 );
	d->colIndex.clear();
	if ( d->fld.count() ) {
	    QSqlField* field = 0;
	    for ( uint i = 0; i < d->fld.count(); ++i ) {
		field = cur->field( d->fld[ i ] );
		if ( field && ( cur->isGenerated( field->name() ) ||
				cur->isCalculated( field->name() ) ) &&
		     !cur->primaryIndex().contains( field->name() ) )
		    {
			setNumCols( numCols() + 1 );
			d->colIndex.append( cur->position( field->name() ) );
			setColumnReadOnly( numCols()-1, field->isReadOnly() );
			QHeader* h = horizontalHeader();
			QString label = d->fldLabel[ i ];
			QIconSet icons = d->fldIcon[ i ];
			h->setLabel( numCols()-1, icons, label );
		    }
	    }
	}
    }
    viewport()->setUpdatesEnabled( TRUE );
    horizontalHeader()->repaint();
    setSize( cur );
    // keep others aware
    if ( d->lastAt != currentRow() ) {
	setCurrentSelection( currentRow(), currentColumn() );
    } else {
	if ( cur->seek( d->lastAt ) ) {
	    emit currentChanged( cur );
	}
    }
}

/*! \overload

  Refreshes the table.  The cursor is refreshed using the current
  filter, the current sort, and the currently defined columns.
  Equivalent to calling refresh( QDataTable::RefreshAll ).
*/

void QDataTable::refresh()
{
    refresh( RefreshData );
}

/*!  \reimp

*/

bool QDataTable::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return FALSE;
    bool found = d->cur.findBuffer( idx, atHint );
    if ( found )
	setCurrentCell( cur->at(), currentColumn() );
    return found;
}

/*! \fn void QDataTable::currentChanged( QSqlRecord* record )
  This signal is emitted whenever a new row is selected in the table.
  The \a record parameter points to the contents of the newly selected
  record.
*/

/*! \fn void QDataTable::primeInsert( QSqlRecord* buf )
  This signal is emitted after the cursor is primed for insert by the
  table, when an insert action is beginning on the table.  The \a buf
  parameter points to the edit buffer being inserted.  Connect to
  this signal in order to, for example, prime the record buffer with
  default data values.
*/

/*! \fn void QDataTable::primeUpdate( QSqlRecord* buf ) This signal is
  emitted after the cursor is primed for update by the table, when an
  update action is beginning on the table.  The \a buf parameter
  points to the edit buffer being updated.  Connect to this signal
  to ###?
*/

/*! \fn void QDataTable::primeDelete( QSqlRecord* buf ) This signal is
  emitted after the cursor is primed for delete by the table, when a
  delete action is beginning on the table.  The \a buf parameter
  points to the edit buffer being deleted.  Connect to this signal
  to ###?
*/

/*! \fn void QDataTable::beforeInsert( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is inserted into the database.
  The \a buf parameter points to the edit buffer being inserted.
*/

/*! \fn void QDataTable::beforeUpdate( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is updated in the database.
  The \a buf parameter points to the edit buffer being updated.
*/

/*! \fn void QDataTable::beforeDelete( QSqlRecord* buf )
  This signal is emitted just before the currently selected record is deleted from the database.
  The \a buf parameter points to the edit buffer being deleted.
*/

/*! \fn void QDataTable::cursorChanged( QSql::Op mode )
  This signal is emitted whenever the cursor record was changed due to an edit.
  The \a mode parameter is the edit that just took place.
*/

#endif

/****************************************************************************
**
** Implementation of QSqlTable class
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

#include "qsqltable.h"
#include "qsqldriver.h"
#include "qsqleditorfactory.h"
#include "qsqlpropertymap.h"
#include "qsqlnavigator.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qpopupmenu.h"
#include "qmessagebox.h"
#include "qbitarray.h"

#ifndef QT_NO_SQL

// void qt_debug_buffer( const QString& msg, QSqlRecord* cursor )
// {
//     qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
//     qDebug(msg);
//     for ( uint j = 0; j < cursor->count(); ++j ) {
//	qDebug(cursor->field(j)->name() + " type:" + QString(cursor->field(j)->value().typeName()) + " value:" + cursor->field(j)->value().toString() );
//     }
// }

class QSqlTablePrivate
{
public:
    QSqlTablePrivate()
	: haveAllRows( FALSE ),
	  continuousEdit( FALSE ),
	  editorFactory( 0 ),
	  propertyMap( 0 ),
	  mode( QSqlTable::None ),
	  editRow( -1 ),
	  editCol( -1 ),
	  insertRowLast( -1 ),
	  insertPreRows( -1 ),
	  editBuffer( 0 ),
	  confEdits( 3 ),
	  confCancs( FALSE ),
	  cancelMode( FALSE ),
	  autoDelete( FALSE )
    {}
    ~QSqlTablePrivate() { if ( propertyMap ) delete propertyMap; }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;
    bool         continuousEdit;
    QSqlEditorFactory* editorFactory;
    QSqlPropertyMap* propertyMap;
    QString trueTxt;
    QString falseTxt;
    QSqlTable::Mode mode;
    int editRow;
    int editCol;
    int insertRowLast;
    QString insertHeaderLabelLast;
    int insertPreRows;
    QSqlRecord* editBuffer;
    QBitArray confEdits;
    bool confCancs;
    bool cancelMode;
    bool autoDelete;
    int lastAt;
    QString ftr;
    QStringList srt;
    QStringList fld;
    QStringList fldLabel;
};

/*! \enum QSqlTable::Confirm

  This enum type describes edit confirmations.

  The currently defined values are:

  <ul>
  <li> \c Yes
  <li> \c No
  <li> \c Cancel
  </ul>
*/

/*! \enum QSqlTable::Mode

  This enum type describes table editing modes.

  The currently defined values are:

  <ul>
  <li> \c None
  <li> \c Insert
  <li> \c Update
  <li> \c Delete
  </ul>
*/

/*!
  \class QSqlTable qsqltable.h
  \module sql

  \brief A flexible SQL table widget that supports browsing and editing.

  QSqlTable supports various functions for presenting and editing SQL
  data from a \l QSqlCursor.

  When displaying data, QSqlTable only retrieves data for visible rows.
  If the driver supports the 'query size' property the QSqlTable will
  have the correct number of rows and the vertical scrollbar will
  accurately reflect the number of rows displayed in proportion to the
  number of rows in the dataset. If the driver does not support the
  'query size' property rows are dynamically fetched from the database
  on an as-needed basis with the scrollbar becoming more accurate as the
  user scrolls down through the records.  This allows extremely large
  queries to be displayed as quickly as possible, with minimum memory
  usage.

  QSqlTable inherits QTable's API and extends it with functions to
  sort and filter the data and sort columns. See setCursor(),
  setFilter(), setSort(), setSorting(), sortColumn() and refresh().

  When displaying editable cursors, cell editing will be enabled (for
  more information on editable cursors, see \l QSqlCursor).  QSqlTable
  can be used to modify existing data and to enter new records.  When
  a user makes changes to a field in the table, the cursor's edit
  buffer is used.  The table will not send changes in the edit buffer
  to the database until the user moves to a different record in the
  grid.  If there is a problem updating data, errors will be handled
  automatically (see handleError() to change this behavior). QSqlTable
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

QSqlTable::QSqlTable ( QWidget * parent, const char * name )
    : QTable( parent, name ), QSqlCursorNavigator()
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

QSqlTable::QSqlTable ( QSqlCursor* cursor, bool autoPopulate, QWidget * parent, const char * name )
    : QTable( parent, name ), QSqlCursorNavigator()
{
    init();
    setCursor( cursor, autoPopulate );
}

/*! \internal
*/


void QSqlTable::init()
{
    setFocusProxy( viewport() );
    viewport()->setFocusPolicy( StrongFocus );

    d = new QSqlTablePrivate();
    setSelectionMode( NoSelection );
    d->trueTxt = tr( "True" );
    d->falseTxt = tr( "False" );
    reset();
    connect( this, SIGNAL( currentChanged( int, int ) ),
	     SLOT( setCurrentSelection( int, int )));
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlTable::~QSqlTable()
{
    if ( d->autoDelete )
	delete sqlCursor();
    delete d;
}


/*!  Adds \a fieldName as the next column to be diplayed.  If \a label
  is specified, it is used as the column header label, otherwise the
  field's display label is used when setCursor() is called.

  \sa setCursor() refresh()

*/

void QSqlTable::addColumn( const QString& fieldName, const QString& label )
{
    d->fld += fieldName;
    d->fldLabel += label;
}

/*!  Sets column \a col to display field \a field.  If \a label is
  specified, it is used as the column header label, otherwise the
  field's display label is used when setCursor() is called.

  \sa setCursor() refresh()

*/

void QSqlTable::setColumn( uint col, const QString& fieldName, const QString& label )
{
    d->fld[col]= fieldName;
    d->fldLabel[col] = label;
}

/*!  Removes column \a col from the list of columns to be diplayed.
  If \a col does not exist, nothing happens.

  \sa QSqlField

*/

void QSqlTable::removeColumn( uint col )
{
    if ( d->fld.at( col ) != d->fld.end() ) {
	d->fld.remove( d->fld.at( col ) );
	d->fldLabel.remove( d->fldLabel.at( col ) );
    }
}

/*! Returns the current filter used on the displayed data.  If there
  is no current cursor, QString::null is returned.

  \sa setFilter() setCursor()

 */

QString QSqlTable::filter() const
{
    return QSqlCursorNavigator::filter();
}

/*! Sets the filter to be used on the displayed data to \a filter.  To
  display the filtered data, call refresh(). The text of a filter is the
  SQL for a WHERE clause but without the leading "WHERE" or trailing
  semicolon, e.g. "surname LIKE 'A%'".

  \sa refresh() filter()
*/

void QSqlTable::setFilter( const QString& filter )
{
    QSqlCursorNavigator::setFilter( filter );
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

void QSqlTable::setSort( const QStringList& sort )
{
    QSqlCursorNavigator::setSort( sort );
}

/*! Sets the sort to be used on the displayed data to \a sort.  If
  there is no current cursor, nothing happens. A QSqlIndex contains
  field names and their ordering (ASC or DESC); these are used to
  compose the ORDER BY clause.

  \sa sort()
*/

void QSqlTable::setSort( const QSqlIndex& sort )
{
    QSqlCursorNavigator::setSort( sort );
}


/*! Returns the current sort used on the displayed data as a list of
  strings.  Each field is in the form:

  "\a cursorname.\a fieldname ASC" (for ascending sort) or
  "\a cursorname.\a fieldname DESC" (for descending sort)

  If there is no current cursor, an empty string list is returned.

  \sa setSort()

 */

QStringList QSqlTable::sort() const
{
    return QSqlCursorNavigator::sort();
}

/*! If \a confirm is TRUE, all edit operations (inserts, updates and
  deletes) will be confirmed by the user.  If \a confirm is FALSE (the
  default), all edits are posted to the database immediately.

*/
void QSqlTable::setConfirmEdits( bool confirm )
{
    d->confEdits.fill( confirm );
}

/*! If \a confirm is TRUE, all inserts will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QSqlTable::setConfirmInsert( bool confirm )
{
    d->confEdits[ Insert ] = confirm;
}

/*! If \a confirm is TRUE, all updates will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QSqlTable::setConfirmUpdate( bool confirm )
{
    d->confEdits[ Update ] = confirm;
}

/*! If \a confirm is TRUE, all deletes will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QSqlTable::setConfirmDelete( bool confirm )
{
    d->confEdits[ Delete ] = confirm;
}

/*! Returns TRUE if the table confirms all edit operations (inserts,
  updates and deletes), otherwise returns FALSE.
*/

bool QSqlTable::confirmEdits() const
{
    return ( confirmInsert() && confirmUpdate() && confirmDelete() );
}

/*! Returns TRUE if the table confirms inserts, otherwise returns
  FALSE.
*/

bool QSqlTable::confirmInsert() const
{
    return ( d->confEdits[ Insert ] );
}

/*! Returns TRUE if the table confirms updates, otherwise returns
  FALSE.
*/

bool QSqlTable::confirmUpdate() const
{
    return ( d->confEdits[ Update ] );
}

/*! Returns TRUE if the table confirms deletes, otherwise returns
  FALSE.
*/

bool QSqlTable::confirmDelete() const
{
    return ( d->confEdits[ Delete ] );
}

/*! If \a confirm is TRUE, all cancels will be confirmed by the user
  through a message box.  If \a confirm is FALSE (the default), all
  cancels occur immediately.
*/

void QSqlTable::setConfirmCancels( bool confirm )
{
    d->confCancs = confirm;
}

/*! Returns TRUE if the table confirms cancels, otherwise returns FALSE.
*/

bool QSqlTable::confirmCancels() const
{
    return d->confCancs;
}

/*!  \reimp

  For an editable table, creates an editor suitable for the field in
  column \a col.  The editor is created using the default editor
  factory, unless a different editor factory was installed using
  installEditorFactory().  The editor is primed with the value of the
  field in \a col using a property map. The property map used is the
  default property map, unless a new property map was installed using
  installPropertMap(). If \a initFromCell is TRUE then the editor is
  primed with the value in the QSqlTable cell.

*/

QWidget * QSqlTable::createEditor( int , int col, bool initFromCell ) const
{
    if ( d->mode == QSqlTable::None )
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

bool QSqlTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QTable::eventFilter( o, e );

    if ( d->cancelMode )
	return TRUE;

    int r = currentRow();
    int c = currentColumn();

    if ( d->mode != QSqlTable::None ) {
	r = d->editRow;
	c = d->editCol;
    }

    bool insertCancelled = FALSE;
    QWidget *editorWidget = cellWidget( r, c );
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	int conf = Yes;
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Escape && d->mode == QSqlTable::Insert ){
	    if ( confirmCancels() && !d->cancelMode )
		conf = confirmCancel( QSqlTable::Insert );
	    if ( conf == Yes ) {
		insertCancelled = TRUE;
		endInsert(); // ### What happens to the keyboard focus??
	    } else {
		editorWidget->setActiveWindow();
		editorWidget->setFocus();
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Escape && d->mode == QSqlTable::Update ) {
	    if ( confirmCancels() && !d->cancelMode )
		conf = confirmCancel( QSqlTable::Update );
	    if ( conf == Yes ){
		endUpdate();
	    } else {
		editorWidget->setActiveWindow();
		editorWidget->setFocus();
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Insert && d->mode == QSqlTable::None ) {
	    beginInsert();
	    return TRUE;
	}
	if ( ke->key() == Key_Delete && d->mode == QSqlTable::None ) {
	    deleteCurrent();
	    return TRUE;
	}
	if ( d->mode != QSqlTable::None ) {
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
	if ( !d->cancelMode && editorWidget && o == editorWidget && (d->mode == QSqlTable::Insert) && !d->continuousEdit) {
	    setCurrentCell( r, c );
	    endEdit( r, c, TRUE, FALSE );
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

void QSqlTable::resizeEvent ( QResizeEvent * e )
{

    if ( sqlCursor() && !sqlCursor()->driver()->hasQuerySizeSupport() )
	loadNextPage();
    QTable::resizeEvent( e );
}

/*!  \reimp
*/

void QSqlTable::contentsMousePressEvent( QMouseEvent* e )
{
    if ( d->mode != QSqlTable::None ) {
	endEdit( d->editRow, d->editCol, TRUE, FALSE );
    }
    if ( !sqlCursor() ) {
	QTable::contentsMousePressEvent( e );
	return;
    }
    if ( e->button() == RightButton && d->mode == QSqlTable::None ) {
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
    if ( d->mode == QSqlTable::None )
	QTable::contentsMousePressEvent( e );

}

/*!  \reimp
*/

QWidget* QSqlTable::beginEdit ( int row, int col, bool replace )
{
    d->editRow = -1;
    d->editCol = -1;
    if ( !sqlCursor() )
	return 0;
    if ( d->mode == QSqlTable::Insert && !sqlCursor()->canInsert() )
	return 0;
    if ( d->mode == QSqlTable::Update && !sqlCursor()->canUpdate() )
	return 0;
    d->editRow = row;
    d->editCol = col;
    if ( d->continuousEdit ) {
	QWidget* w = QTable::beginEdit( row, col, replace );
	return w;
    }
    if ( d->mode == QSqlTable::None && sqlCursor()->canUpdate() && sqlCursor()->primaryIndex().count() > 0 )
	return beginUpdate( row, col, replace );
    return 0;
}

/*! \reimp
*/

void QSqlTable::endEdit( int row, int col, bool accept, bool )
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
    if ( d->mode != QSqlTable::None && d->editBuffer ) {
	QSqlPropertyMap * m = (d->propertyMap == 0) ?
			      QSqlPropertyMap::defaultMap() : d->propertyMap;
	d->editBuffer->setValue( indexOf( col ),  m->property( editor ) );
	clearCellWidget( row, col );
	if ( !d->continuousEdit ) {
	    switch ( d->mode ) {
	    case QSqlTable::Insert:
		insertCurrent();
		break;
	    case QSqlTable::Update:
		updateCurrent();
		break;
	    default:
		break;
	    }
	}
    } else {
	setEditMode( NotEditing, -1, -1 );
    }
    if ( d->mode == QSqlTable::None ) {
	viewport()->setFocus();
    }
    updateCell( row, col );
    emit valueChanged( row, col );
}

/*! \reimp
*/
void QSqlTable::activateNextCell()
{
    if ( d->mode == QSqlTable::None )
	QTable::activateNextCell();
}

/*! \internal
*/

void QSqlTable::endInsert()
{
    d->mode = QSqlTable::None;
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

void QSqlTable::endUpdate()
{
    d->mode = QSqlTable::None;
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

bool QSqlTable::beginInsert()
{
    if ( !sqlCursor() || isReadOnly() || ! numCols() )
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
    emit beginInsert( d->editBuffer );
    d->mode = QSqlTable::Insert;
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

   \a row and \a col refer to the row and column in the QSqlTable.

*/

QWidget* QSqlTable::beginUpdate ( int row, int col, bool replace )
{
    if ( !sqlCursor() || isReadOnly() )
	return 0;
    setCurrentCell( row, col );
    d->mode = QSqlTable::Update;
    if ( sqlCursor()->seek( row ) ) {
	d->editBuffer = sqlCursor()->primeUpdate();
	emit beginUpdate( d->editBuffer );
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

void QSqlTable::insertCurrent()
{
    if ( d->mode != QSqlTable::Insert || ! numCols() )
	return;
    if ( !sqlCursor()->canInsert() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::insertCurrent: insert not allowed for " + sqlCursor()->name() );
#endif
	endInsert();
	return;
    }
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() || confirmInsert() )
	conf = confirmEdit( QSqlTable::Insert );
    switch ( conf ) {
    case Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeInsert( d->editBuffer );
	b = sqlCursor()->insert();
	QApplication::restoreOverrideCursor();
	if ( !b || !sqlCursor()->isActive() )
	    handleError( sqlCursor()->lastError() );
	QSqlIndex idx = sqlCursor()->primaryIndex( TRUE );
	endInsert();
	setEditMode( NotEditing, -1, -1 );
	refresh();
	findBuffer( idx, d->lastAt );
	emit cursorChanged( QSqlCursor::Insert );
	setCurrentCell( currentRow(), currentColumn() );
	break;
    }
    case No:
	endInsert();
	setEditMode( NotEditing, -1, -1 );
	break;
    case Cancel:
	if ( QTable::beginEdit( currentRow(), currentColumn(), FALSE ) )
	    setEditMode( Editing, currentRow(), currentColumn() );
	break;
    }
    return;
}

/*! \internal
*/

void QSqlTable::updateRow( int row )
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

void QSqlTable::updateCurrent()
{
    if ( d->mode != QSqlTable::Update )
	return;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: no primary index for " + sqlCursor()->name() );
#endif
	endUpdate();
	return;
    }
    if ( !sqlCursor()->canUpdate() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: updates not allowed for " + sqlCursor()->name() );
#endif
	endUpdate();
	return;
    }
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() || confirmUpdate() )
	conf = confirmEdit( QSqlTable::Update );
    switch ( conf ) {
    case Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeUpdate( d->editBuffer );
	b = sqlCursor()->update();
	QApplication::restoreOverrideCursor();
	if ( !b || !sqlCursor()->isActive() )
	    handleError( sqlCursor()->lastError() );
	QSqlIndex idx = sqlCursor()->primaryIndex( TRUE );
	endUpdate();
	refresh();
	findBuffer( idx, d->lastAt );
	emit cursorChanged( QSqlCursor::Update );
	setCurrentCell( currentRow(), currentColumn() );
	break;
    }
    case No:
	endUpdate();
	setEditMode( NotEditing, -1, -1 );
	break;
    case Cancel:
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

void QSqlTable::deleteCurrent()
{
    if ( !sqlCursor() || isReadOnly() )
	return;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::deleteCurrent: no primary index " + sqlCursor()->name() );
#endif
	return;
    }
    if ( !sqlCursor()->canDelete() )
	return;
    if ( !sqlCursor()->seek( currentRow() ) )
	return;
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() || confirmDelete() )
	conf = confirmEdit( QSqlTable::Delete );
    switch ( conf ) {
    case Yes:
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeDelete( d->editBuffer );
	b = sqlCursor()->del();
	QApplication::restoreOverrideCursor();
	if ( !b )
	    handleError( sqlCursor()->lastError() );
	refresh();
	emit cursorChanged( QSqlCursor::Delete );
	setCurrentCell( currentRow(), currentColumn() );
	updateRow( currentRow() );
	break;
    case No:
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

QSqlTable::Confirm QSqlTable::confirmEdit( QSqlTable::Mode m )
{
    QString cap;
    switch ( m ) {
    case None:
	return QSqlTable::Cancel;
    case Insert:
	cap = "Insert";
	break;
    case Update:
	cap = "Update";
	break;
    case Delete:
	cap = "Delete";
	break;
    }
    QSqlTable::Confirm conf;
    if ( m == Delete )
	conf = (QSqlTable::Confirm)QMessageBox::information ( this, tr( cap ),
		tr("Delete this record?"),
		tr( "Yes" ),
		tr( "No" ),
		QString::null, 0, 1 );
    else
	conf = (QSqlTable::Confirm)QMessageBox::information ( this, tr( cap ),
		tr( "Save edits?" ),
		tr( "Yes" ),
		tr( "No" ),
		tr( "Cancel" ), 0, 2 );
    return conf;
}

/*!  Protected virtual function which returns a confirmation for
   cancelling an edit mode \a m.  Derived classes can reimplement this
   function and provide their own confirmation dialog.  The default
   implementation uses a message box which prompts the user to confirm
   the edit action.

*/

QSqlTable::Confirm  QSqlTable::confirmCancel( QSqlTable::Mode )
{
    d->cancelMode = TRUE;
    QSqlTable::Confirm conf =  (QSqlTable::Confirm)QMessageBox::information ( this, tr( "Confirm" ),
					      tr( "Cancel your edits?" ),
					      tr( "Yes" ),
					      tr( "No" ), QString::null, 0, 1 );
    d->cancelMode = FALSE;
    return conf;
}


/*! Searches the current cursor for a cell containing the string \a str
    starting at the current cell and working forwards. If the string is
    found, the cell containing the string is set as the current cell.

    If \a caseSensitive is FALSE the case of \a str will be ignored. If
    \a backwards is TRUE the search will start at the current cell and
    work backwards.

    The search will wrap, i.e. if the first (or if backwards is TRUE,
    last) cell is reached without finding \a str the search will
    continue until it reaches the starting cell. If \a str is not found
    the search will fail and the current cell will remain unchanged.
*/
void QSqlTable::find( const QString & str, bool caseSensitive, bool backwards )
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


/*!  Resets the table so that it displays no data.  This is called
  internally before displaying new data.

  \sa setCursor()

*/

void QSqlTable::reset()
{
    clearCellWidget( currentRow(), currentColumn() );
    switch ( d->mode ) {
    case Insert:
	endInsert();
	break;
    case Update:
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
    d->mode =  QSqlTable::None;
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    d->cancelMode = FALSE;
    d->lastAt = -1;
    d->fld.clear();
    d->fldLabel.clear();
    if ( sorting() )
	horizontalHeader()->setSortIndicator( -1 );
}

/*!  Returns the index of the field within the current SQL query that is
  displayed in column \a i.

*/

int QSqlTable::indexOf( uint i ) const
{
    QSqlTablePrivate::ColIndex::ConstIterator it = d->colIndex.at( i );
    if ( it != d->colIndex.end() )
	return *it;
    return -1;
}

/*! Returns TRUE if the table will automatically delete the cursor
  specified by setCursor() otherwise returns FALSE.
*/

bool QSqlTable::autoDelete() const
{
    return d->autoDelete;
}

/*! Sets the auto-delete flag to \a enable.  If \a enable is TRUE, the
  table will automatically delete the cursor specified by setCursor().
  Otherwise, (the default) the cursor will not be deleted.
*/

void QSqlTable::setAutoDelete( bool enable )
{
    d->autoDelete = enable;
}

/*!  Sets the text to be displayed when a NULL value is encountered in
  the data to \a nullText.  The default value is specified by the
  cursor's driver.

*/

void QSqlTable::setNullText( const QString& nullText )
{
    d->nullTxt = nullText;
}

/*!  Returns the text to be displayed when a NULL value is encountered
  in the data.

*/

QString QSqlTable::nullText() const
{
    return d->nullTxt;
}

/*!  Sets the text to be displayed when a TRUE bool value is
  encountered in the data to \a trueText.  The default is 'True'.

*/

void QSqlTable::setTrueText( const QString& trueText )
{
    d->trueTxt = trueText;
}

/*!  Returns the text to be displayed when a TRUE bool value is
  encountered in the data.

*/

QString QSqlTable::trueText() const
{
    return d->trueTxt;
}

/*!  Sets the text to be displayed when a FALSE bool value is
  encountered in the data to \a falseText.  The default is 'False'.

*/

void QSqlTable::setFalseText( const QString& falseText )
{
    d->falseTxt = falseText;
}


/*!  Returns the text to be displayed when a FALSE bool value is
  encountered in the data.

*/

QString QSqlTable::falseText() const
{
    return d->falseTxt;
}

/*!  \reimp
*/

int QSqlTable::numRows() const
{
    return QTable::numRows();
}

/*!  \reimp

  The number of rows in the table will be determined by the cursor
  (see setCursor()), so normally this function should never be called.
  It is included for completeness.
*/

void QSqlTable::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

/*!  \reimp

  The number of columns in the table will be determined automatically
  (see addColumn()), so normally this function should never be called.
  It is included for completeness.
*/

void QSqlTable::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

/*!  \reimp
*/

int QSqlTable::numCols() const
{
    return QTable::numCols();
}

/*!  Returns the text in cell \a row, \a col, or an empty string if
  the cell is empty. If the cell's value is NULL then it's nullText()
  will be returned. If the cell does not exist then a null QString is
  returned.

*/

QString QSqlTable::text ( int row, int col ) const
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

QVariant QSqlTable::value ( int row, int col ) const
{
    if ( !sqlCursor() )
	return QVariant();
    if ( sqlCursor()->seek( row ) )
	return sqlCursor()->value( indexOf( col ) );
    return QVariant();
}

/*!  \internal
*/

void QSqlTable::loadNextPage()
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

void QSqlTable::loadLine( int )
{
    loadNextPage();
}

/*!  Sorts the column \a col in ascending order if \a ascending is
  TRUE, otherwise in descending order. The \a wholeRows parameter is
  ignored for SQL tables.

*/

void QSqlTable::sortColumn ( int col, bool ascending,
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
	QApplication::setOverrideCursor( Qt::waitCursor );
	sqlCursor()->select( sqlCursor()->filter(), newSort );
	QApplication::restoreOverrideCursor();
	viewport()->repaint( FALSE );
    }
}

/*!  \reimp
*/

void QSqlTable::columnClicked ( int col )
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

// void QSqlTable::paintFocus( QPainter * p, const QRect & cr )
// {
// //    QSqlTable::paintFocus( p, cr );
// }

/*!  \reimp
*/
void QSqlTable::repaintCell( int row, int col )
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

void QSqlTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
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
    if ( d->mode != QSqlTable::None ) {
	if ( row == d->editRow && d->editBuffer ) {
	    paintField( p, d->editBuffer->field( indexOf( col ) ), cr,
			selected );
	} else if ( row > d->editRow && d->mode == QSqlTable::Insert ) {
	    if ( sqlCursor()->seek( row - 1 ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	} else {
	    if ( sqlCursor()->seek( row ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	}
    }
    else {
	if ( sqlCursor()->seek( row ) ) {
	    paintField( p, sqlCursor()->field( indexOf( col ) ), cr, selected );
	}
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

void QSqlTable::paintField( QPainter * p, const QSqlField* field,
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

int QSqlTable::fieldAlignment( const QSqlField* field )
{
    if ( !sqlCursor() )
	return Qt::AlignLeft | Qt::AlignVCenter;
    return sqlCursor()->alignment( field->name() ) | Qt::AlignVCenter;
}


/*!  If the \a sql driver supports query sizes, the number of rows in
  the table is set to the size of the query.  Otherwise, the table
  dynamically resizes itself as it is scrolled.  If \q sql is not
  active, it is made active by issuing a select() on the cursor using
  the current filter and current sort of \a sql.

*/

void QSqlTable::setSize( QSqlCursor* sql )
{
    if ( !sql->isActive() ) {
	sql->select( sql->filter(), sql->sort() );
    }
    if ( sql->driver()->hasQuerySizeSupport() ) {
	setVScrollBarMode( Auto );
	disconnect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		 this, SLOT( loadLine(int) ) );
	setNumRows( sql->size() );
    } else {
	setVScrollBarMode( AlwaysOn );
	connect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		 this, SLOT( loadLine(int) ) );
	loadNextPage();
    }
}

/*!  Sets \a cursor as the data source for the table.  To force the
  display of the data from \a cursor, use refresh(). If \q autoPopulate
  is TRUE (the default is FALSE), columns are automatically created
  based upon the fields in the \a cursor record.  If \a autoDelete is
  TRUE (the default is FALSE), the table will take ownership of the \a
  cursor and delete it when appropriate.  If the \a cursor is read
  only, the table becomes read only.  The table adopts the cursor's
  driver's definition for representing NULL values as strings.

  \sa refresh() setReadOnly() setAutoDelete() QSqlDriver::nullText()

*/

void QSqlTable::setCursor( QSqlCursor* cursor, bool autoPopulate, bool autoDelete )
{
    setUpdatesEnabled( FALSE );
    if ( d->autoDelete )
	delete sqlCursor();
    if ( cursor ) {
	//	reset(); ## needed here?
	setSqlCursor( cursor );
	if ( autoPopulate ) {
	    for ( uint i = 0; i < sqlCursor()->count(); ++i )
		addColumn( sqlCursor()->field( i )->name(), sqlCursor()->displayLabel( sqlCursor()->field( i )->name() ) );
	}
	setReadOnly( sqlCursor()->isReadOnly() ); // ## do this by default?
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
void QSqlTable::handleError( const QSqlError& e )
{
    QMessageBox::warning ( this, "Warning", e.driverText() + "\n" + e.databaseText(), 0, 0 );
}

/*!  \reimp
*/

void QSqlTable::resizeData ( int )
{

}

/*!  \reimp
*/

QTableItem * QSqlTable::item ( int, int ) const
{
    return 0;
}

/*!  \reimp
*/

void QSqlTable::setItem ( int , int , QTableItem * )
{

}

/*!  \reimp
*/

void QSqlTable::clearCell ( int , int )
{

}

/*!  \reimp
*/

void QSqlTable::setPixmap ( int , int , const QPixmap &  )
{

}

/*!  \reimp
*/

void QSqlTable::takeItem ( QTableItem * )
{

}

/*!  Installs a new SQL editor factory. This enables the user to
  create and instantiate their own editors for use in cell editing.
  Note that QSqlTable takes ownership of this pointer, and will delete
  it when it is no longer needed or when installEditorFactory() is
  called again.

  \sa QSqlEditorFactory
*/

void QSqlTable::installEditorFactory( QSqlEditorFactory * f )
{
    if( f ) {
	delete d->editorFactory;
	d->editorFactory = f;
    }
}

/*!  Installs a new property map. This enables the user to create and
  instantiate their own property maps for use in cell editing.  Note
  that QSqlTable takes ownership of this pointer, and will delete it
  when it is no longer needed or when installPropertMap() is called
  again.

  \sa QSqlPropertyMap

*/

void QSqlTable::installPropertyMap( QSqlPropertyMap* m )
{
    if ( m ) {
	delete d->propertyMap;
	d->propertyMap = m;
    }
}

/*!  \internal
*/

void QSqlTable::setCurrentSelection( int row, int )
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

/*!  Returns the currently selected record, or an empty record if
  there is no current selection.

*/

QSqlRecord QSqlTable::currentFieldSelection() const
{
    QSqlRecord fil;
    if ( !sqlCursor() || currentRow() < 0 )
	return fil;
    int row = currentRow();
    if ( !sqlCursor()->seek( row ) )
	return fil;
    fil = *sqlCursor();
    return fil;
}

/*! Sorts column \a col in ascending order.

  \sa setSorting()
*/

void QSqlTable::sortAscending( int col )
{
    sortColumn( col, TRUE );
}

/*! Sorts column \a col in descending order.

  \sa setSorting()
*/

void QSqlTable::sortDescending( int col )
{
    sortColumn( col, FALSE );
}

/*! Refreshes the table using the current cursor
*/

void QSqlTable::refresh()
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return;
    QSqlCursorNavigator::refresh();
    setNumCols(0);
    d->colIndex.clear();
    if ( d->fld.count() ) {
	QSqlField* field = 0;
	for ( uint i = 0; i < d->fld.count(); ++i ) {
	    field = cur->field( d->fld[ i ] );
	    if ( field &&
		 cur->isVisible( field->name() ) &&
		 !cur->primaryIndex().contains( field->name() ) ) {
		setNumCols( numCols() + 1 );
		d->colIndex.append( cur->position( field->name() ) );
		setColumnReadOnly( numCols()-1, field->isReadOnly() );
		QHeader* h = horizontalHeader();
		QString label = d->fldLabel[ i ];
		if ( label == QString::null )
		    label = cur->displayLabel( field->name() );
		h->setLabel( numCols()-1, label );
	    }
	}
    }
    setSize( cur );
}

/*!  \reimp

*/

bool QSqlTable::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return FALSE;
    bool found = QSqlCursorNavigator::findBuffer( idx, atHint );
    if ( found )
	setCurrentCell( cur->at(), currentColumn() );
    return found;
}

/*! \fn void QSqlTable::currentChanged( const QSqlRecord* record )
  This signal is emitted whenever a new row is selected in the table.
  The \a record parameter points to the contents of the newly selected
  record.
*/

/*! \fn void QSqlTable::beginInsert( QSqlRecord* buf )
  This signal is emitted when an insert is beginning on the cursor's edit buffer.
  The \a buf parameter points to the record buffer being inserted.
*/

/*! \fn void QSqlTable::beginUpdate( QSqlRecord* buf )
  This signal is emitted when an update is beginning on the cursor's edit buffer.
  The \a buf parameter points to the record buffer being updated.
*/

/*! \fn void QSqlTable::beforeInsert( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is inserted into the database.
  The \a buf parameter points to the record buffer being inserted.
*/

/*! \fn void QSqlTable::beforeUpdate( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is updated in the database.
  The \a buf parameter points to the record buffer being updated.
*/

/*! \fn void QSqlTable::beforeDelete( QSqlRecord* buf )
  This signal is emitted just before the currently selected record is deleted from the database.
  The \a buf parameter points to the record buffer being deleted.
*/

/*! \fn void QSqlTable::cursorChanged( QSqlCursor::Mode mode )
  This signal is emitted whenever the cursor record was changed due to an edit.
  The \a mode parameter is the edit that just took place.
*/

#endif

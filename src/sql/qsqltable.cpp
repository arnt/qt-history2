/****************************************************************************
**
** Implementation of QSqlTable class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qapplication.h"
#include "qsqltable.h"
#include "qsqldriver.h"
#include "qsqleditorfactory.h"
#include "qsqlform.h"
#include "qlayout.h"
#include "qpopupmenu.h"
#include "qmessagebox.h"

#ifndef QT_NO_SQL

// void qt_debug_buffer( const QString& msg, QSqlRecord* cursor )
// {
//     qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
//     qDebug(msg);
//     for ( uint j = 0; j < cursor->count(); ++j ) {
// 	qDebug(cursor->field(j)->name() + " type:" + QString(cursor->field(j)->value().typeName()) + " value:" + cursor->field(j)->value().toString() );
//     }
// }

class QSqlTablePrivate
{
public:
    QSqlTablePrivate()
	: nullTxt(),
	  haveAllRows(FALSE),
	  continuousEdit(FALSE),
	  ro(TRUE),
	  editorFactory(0),
	  propertyMap(0),
	  cursor(0),
	  mode( QSqlTable::None ),
	  editRow(-1),
	  editCol(-1),
	  insertRowLast(-1),
	  insertHeaderLabelLast(),
	  insertPreRows(-1),
	  editBuffer(0),
	  confEdits( FALSE ),
	  confCancs( FALSE ),
	  cancelMode( FALSE ),
	  autoDelete( FALSE ),
	  lastAt()
    {}
    ~QSqlTablePrivate() { if ( propertyMap ) delete propertyMap; }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    typedef      QValueList< bool > ColReadOnly;
    ColReadOnly  colReadOnly;
    bool         haveAllRows;
    bool         continuousEdit;
    bool         ro;
    QSqlEditorFactory* editorFactory;
    QSqlPropertyMap* propertyMap;
    QString trueTxt;
    QString falseTxt;
    QSqlCursor* cursor;
    QSqlTable::Mode mode;
    int editRow;
    int editCol;
    int insertRowLast;
    QString insertHeaderLabelLast;
    int insertPreRows;
    QSqlRecord* editBuffer;
    bool confEdits;
    bool confCancs;
    bool cancelMode;
    bool autoDelete;
    int lastAt;
};

/*!
  \class QSqlTable qsqltable.h
  \module sql

  \brief A flexible and editable SQL table widget.

  QSqlTable supports various methods for presenting and editing SQL data.

  When displaying data, QSqlTable only retrieves data for visible
  rows.  If drivers do not support the 'query size' property, rows are
  dynamically fetched from the database on an as-needed basis.  This
  allows extremely large queries to be displayed as quickly as
  possible, with limited memory usage.

  QSqlTable also offers an API for sorting columns. See setSorting()
  and sortColumn().

  When displaying QSqls, cell editing can be enabled with
  setCellEditing().  QSqlTable creates editors.... ### The user can
  create their own special editors by ... ###

*/

/*!
  Constructs a SQL table.

*/

QSqlTable::QSqlTable ( QWidget * parent, const char * name )
    : QTable( parent, name )
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

/*!
  Destructor.

*/

QSqlTable::~QSqlTable()
{
    if ( d->autoDelete )
	delete d->cursor;
    delete d;
}


/*!

  Adds \a field from the current cursor as the next column to be
  diplayed.  Fields which are not visible and fields which are part of
  a cursor's primary index are not displayed. If there is no current
  cursor, nothing happens.

  \sa QSqlField setCursor()

*/

void QSqlTable::addColumn( const QSqlField* field )
{
    if ( cursor() && field && cursor()->isVisible( field->name() ) && !cursor()->primaryIndex().field( field->name() ) ) {
	setNumCols( numCols() + 1 );
	d->colIndex.append( cursor()->position( field->name() ) );
	if ( field->isReadOnly() )
	    d->colReadOnly.append( TRUE );
	else
	    d->colReadOnly.append( FALSE );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, cursor()->displayLabel( field->name() ) );
    }
}

/*!

  Removes column \a col from the list of columns to be diplayed.  If
  \a col does not exist, nothing happens.

  \sa QSqlField

*/

void QSqlTable::removeColumn( uint col )
{
    if ( col >= (uint)numCols() )
	return;
    QHeader* h = horizontalHeader();
    for ( uint i = col; i < (uint)numCols()-1; ++i )
	h->setLabel( i, h->label(i+1) );
    setNumCols( numCols()-1 );
    QSqlTablePrivate::ColIndex::Iterator it = d->colIndex.at( col );
    if ( it != d->colIndex.end() )
	d->colIndex.remove( it );
    QSqlTablePrivate::ColReadOnly::Iterator it2 = d->colReadOnly.at( col );
    if ( it2 != d->colReadOnly.end() )
	d->colReadOnly.remove( it2 );
}

/*!

  Sets column \a col to display field \a field.  If \a col does not
  exist, nothing happens.

  \sa QSqlField

*/

void QSqlTable::setColumn( uint col, const QSqlField* field )
{
    if ( col >= (uint)numCols() )
	return;
    if ( !cursor() )
	return;
    if ( cursor()->isVisible( field->name() ) && !cursor()->primaryIndex().field( field->name() ) ) {
	d->colIndex[ col ] = cursor()->position( field->name() );
	if ( field->isReadOnly() )
	    d->colReadOnly[ col ] =  TRUE;
	else
	    d->colReadOnly[ col ] =  FALSE;
	QHeader* h = horizontalHeader();
	h->setLabel( col, field->name() );
    } else {
	removeColumn( col );
    }
}

/*!

  Sets the table's readonly flag to \a b.  Note that if the underlying cursor cannot
  be edited, this function will have no effect.

  \sa setCursor() isReadOnly()

*/

void QSqlTable::setReadOnly( bool b )
{
    d->ro = b;
}

/*!

  Returns TRUE if the table is readonly, otherwise FALSE is returned.

  \sa setReadOnly()

*/

bool QSqlTable::isReadOnly() const
{
    return d->ro;
}

/*!

  Sets the \a column's readonly flag to \a b.  Readonly columns cannot
  be edited. Note that if the underlying cursor cannot be edited, this
  function will have no effect.

  \sa setCursor() isColumnReadOnly()

*/

void QSqlTable::setColumnReadOnly( int col, bool b )
{
    if ( col >= numCols() )
	return;
    d->colReadOnly[ col ] = b;
}

/*!

  Returns TRUE if the \a column is readonly, otherwise FALSE is
  returned.

  \sa setColumnReadOnly()

*/

bool QSqlTable::isColumnReadOnly( int col ) const
{
    if ( col >= numCols() )
	return FALSE;
    return d->colReadOnly[ col ];
}

void QSqlTable::setConfirmEdits( bool confirm )
{
    d->confEdits = confirm;
}

bool QSqlTable::confirmEdits() const
{
    return d->confEdits;
}

void QSqlTable::setConfirmCancels( bool confirm )
{
    d->confCancs = confirm;
}

bool QSqlTable::confirmCancels() const
{
    return d->confCancs;
}

/*!

  \reimpl

  For an editable table, creates an editor suitable for the data type
  in \a row and \a col.

  \sa QSqlEditorFactory QSqlPropertyMap

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
    }
    return b;
}

/*
  \reimpl

*/

void QSqlTable::resizeEvent ( QResizeEvent * e )
{

    if ( d->cursor && !d->cursor->driver()->hasQuerySizeSupport() )
	loadNextPage();
    QTable::resizeEvent( e );
}

/*
  \reimpl

*/

void QSqlTable::contentsMousePressEvent( QMouseEvent* e )
{
    if ( d->mode != QSqlTable::None ) {
	endEdit( d->editRow, d->editCol, TRUE, FALSE );
    }
    if ( !d->cursor ) {
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
	popup->setItemEnabled( id[ IdInsert ], d->cursor->canInsert() );
	popup->setItemEnabled( id[ IdUpdate ], currentRow()>-1 && d->cursor->canUpdate() );
	popup->setItemEnabled( id[ IdDelete ], currentRow()>-1 && d->cursor->canDelete() );
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

/*
  \reimpl

*/

QWidget* QSqlTable::beginEdit ( int row, int col, bool replace )
{
    d->editRow = -1;
    d->editCol = -1;
    if ( !d->cursor )
	return 0;
    if ( d->mode == QSqlTable::Insert && !d->cursor->canInsert() )
	return 0;
    if ( d->mode == QSqlTable::Update && !d->cursor->canUpdate() )
	return 0;
    d->editRow = row;
    d->editCol = col;
    if ( d->continuousEdit ) {
	QWidget* w = QTable::beginEdit( row, col, replace );
	return w;
    }
    if ( d->mode == QSqlTable::None && d->cursor->canUpdate() && d->cursor->primaryIndex().count() > 0 )
	return beginUpdate( row, col, replace );
    return 0;
}

/*
  \reimpl

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

void QSqlTable::activateNextCell()
{
    if ( d->mode == QSqlTable::None )
	QTable::activateNextCell();
}

/*
  \internal

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

void QSqlTable::endUpdate()
{
    d->mode = QSqlTable::None;
    d->editBuffer = 0;
    updateRow( d->editRow );
    d->editRow = -1;
    d->editCol = -1;
}

bool QSqlTable::beginInsert()
{
    if ( !d->cursor || isReadOnly() || ! numCols() )
	return FALSE;
    if ( !d->cursor->canInsert() )
	return FALSE;
    int i = 0;
    int row = currentRow();
    d->insertPreRows = numRows();
    if ( row < 0 || numRows() < 1 )
	row = 0;
    setNumRows( d->insertPreRows + 1 );
    setCurrentCell( row, 0 );
    d->editBuffer = d->cursor->insertBuffer();
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

QWidget* QSqlTable::beginUpdate ( int row, int col, bool replace )
{
    if ( !d->cursor || isReadOnly() )
	return 0;
    setCurrentCell( row, col );
    d->mode = QSqlTable::Update;
    if ( d->cursor->seek( row ) ) {
	d->editBuffer = d->cursor->updateBuffer();
	emit beginUpdate( d->editBuffer );
	return QTable::beginEdit( row, col, replace );
    }
    return 0;
}

/*!  For an editable table, issues an insert on the current cursor using
  the values of the currently edited "insert" row.  If there is no
  current cursor or there is no current "insert" row, nothing happens.
  Returns TRUE if the insert succeeded, otherwise FALSE.

  \sa primeFields
*/

void QSqlTable::insertCurrent()
{
    if ( d->mode != QSqlTable::Insert || ! numCols() )
	return;
    if ( !d->cursor->canInsert() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::insertCurrent: insert not allowed for " + d->cursor->name() );
#endif
	endInsert();
	return;
    }
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() )
	conf = confirmEdit( QSqlTable::Insert );
    switch ( conf ) {
    case Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeInsert( d->editBuffer );
	b = d->cursor->insert();
	QApplication::restoreOverrideCursor();
	if ( !b || !d->cursor->isActive() )
	    handleError( d->cursor->lastError() );
	QSqlIndex idx = d->cursor->primaryIndex( TRUE );
	endInsert();
	setEditMode( NotEditing, -1, -1 );
	refresh( d->cursor, idx );
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

void QSqlTable::updateRow( int row )
{
    for ( int i = 0; i < numCols(); ++i )
	updateCell( row, i );
}

/*!  For an editable table, issues an update on the current cursor's
  primary index using the values of the edited selected row.  If
  there is no current cursor or there is no current selection, nothing
  happens.  Returns TRUE if the update succeeded, otherwise FALSE.

  For this method to succeed, the underlying cursor must have a valid
  primary index to ensure that a unique record is updated within the
  database.

*/

void QSqlTable::updateCurrent()
{
    if ( d->mode != QSqlTable::Update )
	return;
    if ( d->cursor->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: no primary index for " + d->cursor->name() );
#endif
	endUpdate();
	return;
    }
    if ( !d->cursor->canUpdate() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: updates not allowed for " + d->cursor->name() );
#endif
	endUpdate();
	return;
    }
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() )
	conf = confirmEdit( QSqlTable::Update );
    switch ( conf ) {
    case Yes: {
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeUpdate( d->editBuffer );
	b = d->cursor->update();
	QApplication::restoreOverrideCursor();
	if ( !b || !d->cursor->isActive() )
	    handleError( d->cursor->lastError() );
	QSqlIndex idx = d->cursor->primaryIndex( TRUE );
	endUpdate();
	refresh( d->cursor, idx );
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
  happens.  Returns TRUE if the delete succeeded, otherwise FALSE.

  For this method to succeed, the underlying cursor must have a valid
  primary index to ensure that a unique record is deleted within the
  database.

*/

void QSqlTable::deleteCurrent()
{
    if ( !d->cursor || isReadOnly() )
	return;
    if ( d->cursor->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlTable::deleteCurrent: no primary index " + d->cursor->name() );
#endif
	return;
    }
    if ( !d->cursor->canDelete() )
	return;
    if ( !d->cursor->seek( currentRow() ) )
	return;
    int b = 0;
    int conf = Yes;
    if ( confirmEdits() )
	conf = confirmEdit( QSqlTable::Delete );
    switch ( conf ) {
    case Yes:
	QApplication::setOverrideCursor( Qt::waitCursor );
	emit beforeDelete( d->editBuffer );
	b = d->cursor->del();
	QApplication::restoreOverrideCursor();
	if ( !b )
	    handleError( d->cursor->lastError() );
	refresh( d->cursor );
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
  edit of mode \a m.  Derived classes can reimplement in order to
  provide their own confirmation dialog.  The default implementation
  uses a message box which prompts the user to confirm the edit action.

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
    QSqlTable::Confirm conf = (QSqlTable::Confirm)QMessageBox::information ( this, tr( cap ),
					      m == Delete ? tr("Are you sure?") : tr( "Save edits?" ),
					      tr( "Yes" ),
					      tr( "No" ),
					      m == Delete ? QString::null : tr( "Cancel" ), 0, m == Delete ? 1 : 2 );
    return conf;
}

/*!  Protected virtual function which returns a confirmation for
   cancelling an edit mode \a m.  Derived classes can reimplement in
   order to provide their own confirmation dialog.  The default
   implementation uses a message box which prompts the user to confirm
   the edit action.

*/

QSqlTable::Confirm  QSqlTable::confirmCancel( QSqlTable::Mode )
{
    d->cancelMode = TRUE;
    QSqlTable::Confirm conf =  (QSqlTable::Confirm)QMessageBox::information ( this, tr( "Confirm" ),
					      tr( "Are you sure you want to cancel?" ),
					      tr( "Yes" ),
					      tr( "No" ), QString::null, 0, 1 );
    d->cancelMode = FALSE;
    return conf;
}

/*!  Refreshes the \a cursor.  A "select" is issued on the \a cursor using
  the cursor's current filter and current sort.  The table is resized to
  accomodate the cursor size, if possible.  If \a idx is specified, the
  table selects the first record matching the value of the index.

  \sa QSql
*/

void QSqlTable::refresh( QSqlCursor* cursor, QSqlIndex idx )
{
    bool seekPrimary = (idx.count() ? TRUE : FALSE );
    QSqlIndex pi;
    if ( seekPrimary )
	pi = idx;
    QApplication::setOverrideCursor( Qt::waitCursor );
    cursor->select( cursor->filter(), cursor->sort() );
    setSize( cursor );
    if ( seekPrimary ) {
	// ###
	bool indexEquals = FALSE;

	/* common case, check current page */
	int startIdx = verticalScrollBar()->value() / 20;
	int pageSize = (int)( height() * 2 / 20 );
	int endIdx = startIdx + pageSize;
	for ( int j = startIdx; j <= endIdx; ++j ) {
	    if ( cursor->seek( j ) ) {
		for ( uint i = 0; i < pi.count(); ++i ) {
		    const QString fn( pi.field(i)->name() );
		    if ( pi.field(i)->value() == cursor->value( fn ) )
			indexEquals = TRUE;
		    else {
			indexEquals = FALSE;
			break;
		    }
		}
		if ( indexEquals )
		    break;
	    }
	}

	if ( indexEquals )
	    setCurrentCell( cursor->at(), currentColumn() );
	else {
	    /* give up, use brute force */
	    cursor->first();
	    for ( ;; ) {
		indexEquals = FALSE;
		for ( uint i = 0; i < pi.count(); ++i ) {
		    const QString fn( pi.field(i)->name() );
		    if ( pi.field(i)->value() == cursor->value( fn ) )
			indexEquals = TRUE;
		    else {
			indexEquals = FALSE;
			break;
		    }
		}
		if ( indexEquals ) {
		    setCurrentCell( cursor->at(), currentColumn() );
		    break;
		}
		if ( !cursor->next() )
		    break;
	    }
	}
    }
    QApplication::restoreOverrideCursor();
}

/*!

 Search the current result set for the string \a str. If the string is
 found, it will be marked as the current cell.
 */

void QSqlTable::find( const QString & str, bool caseSensitive,
			    bool backwards )
{
    // ### Searching backwards is not implemented yet.
    Q_UNUSED( backwards );

    QSqlCursor * rset = d->cursor;
    if ( !rset )
	return;
    unsigned int  row = currentRow(), startRow = row,
		  col = currentColumn() + 1;
    bool  wrap = TRUE,
	 found = FALSE;

    if( str.isEmpty() || str.isNull() )
	return;

    QApplication::setOverrideCursor( Qt::waitCursor );
    if( rset ){
	while( wrap ){
	    while( !found && rset->seek( row ) ){
		for(unsigned int i = col; i < rset->count(); i++){
		    // ## Sort out the colIndex stuff
		    QString tmp, text = rset->value( i ).toString();
		    if( !caseSensitive ){
			text = text.lower();
			tmp  = str.lower();
		    }
		    if( text.contains( tmp ) ){
			setCurrentCell( row, i );
			col = i;
			found = TRUE;
		    }
		}
		col = 0;
		row++;
	    }

	    if( startRow != 0 ){
		startRow = 0;
	    } else {
		wrap = FALSE;
	    }
	    rset->first();
	    row = 0;
	}
    }
    QApplication::restoreOverrideCursor();
}


/*!

  Resets the table so that it displays no data.  This is called
  internally before displaying a new query.

  \sa setSql() setCursor()

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
    setNumCols(0);
    d->cursor = 0;
    d->haveAllRows = FALSE;
    d->continuousEdit = FALSE;
    d->colIndex.clear();
    d->colReadOnly.clear();
    d->mode =  QSqlTable::None;
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    d->cancelMode = FALSE;
    d->lastAt = -1;
    if ( sorting() )
	horizontalHeader()->setSortIndicator( -1 );
}

/*!

  Returns the index of the field within the current SQL query based on
  the displayed column \a i.

*/

int QSqlTable::indexOf( uint i ) const
{
    QSqlTablePrivate::ColIndex::ConstIterator it = d->colIndex.at( i );
    if ( it != d->colIndex.end() )
	return *it;
    return -1;
}


void QSqlTable::setAutoDelete( bool enable )
{
    d->autoDelete = enable;
}

/*!

  Sets the text to be displayed when a NULL value is encountered in
  the data to \a nullText.  The default value is '<null>'.

*/

void QSqlTable::setNullText( const QString& nullText )
{
    d->nullTxt = nullText;
}

/*!

  Returns the text to be displayed when a NULL value is encountered in
  the data.

*/

QString QSqlTable::nullText() const
{
    return d->nullTxt;
}

/*!

  Sets the text to be displayed when a TRUE bool value is encountered in
  the data to \a nullText.  The default value is 'True'.

*/

void QSqlTable::setTrueText( const QString& trueText )
{
    d->trueTxt = trueText;
}

/*!

  Returns the text to be displayed when a TRUE bool value is encountered in
  the data.

*/

QString QSqlTable::trueText() const
{
    return d->trueTxt;
}

/*!

  Sets the text to be displayed when a FALSE bool value is encountered in
  the data to \a nullText.  The default value is 'False'.

*/

void QSqlTable::setFalseText( const QString& falseText )
{
    d->falseTxt = falseText;
}


/*!

  Returns the text to be displayed when a FALSE bool value is encountered in
  the data.

*/

QString QSqlTable::falseText() const
{
    return d->falseTxt;
}




/*!

  \reimpl

*/

void QSqlTable::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

/*!

  \reimpl

*/

void QSqlTable::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

/*!

  Returns the text in cell \a row, \a col, or an empty string if the
  relevant item does not exist or includes no text.

*/

QString QSqlTable::text ( int row, int col ) const
{
    if ( !d->cursor )
	return QString::null;
    if ( d->cursor->seek( row ) )
	return d->cursor->value( indexOf( col ) ).toString();
    return QString::null;
}

/*!

   Returns the value in cell \a row, \a col, or an invalid value if
   the relevant item does not exist or includes no text.

*/

QVariant QSqlTable::value ( int row, int col ) const
{
    if ( !d->cursor )
	return QVariant();
    if ( d->cursor->seek( row ) )
	return d->cursor->value( indexOf( col ) );
    return QVariant();
}

/*!

  \internal

*/

void QSqlTable::loadNextPage()
{
    if ( d->haveAllRows )
	return;
    if ( !d->cursor )
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
    while ( endIdx > 0 && !d->cursor->seek( endIdx ) )
	endIdx--;
    if ( endIdx != ( startIdx + pageSize + lookAhead ) )
	d->haveAllRows = TRUE;
    setNumRows( endIdx + 1 );
}

/*!

  \internal

*/

void QSqlTable::loadLine( int )
{
    loadNextPage();
}

/*!

  Sorts the column \a col in ascending order if \a ascending is TRUE,
  else in descending order. The \a wholeRows parameter is ignored for
  SQL tables.

*/

void QSqlTable::sortColumn ( int col, bool ascending,
			      bool  )
{
    if ( sorting() ) {
	if ( !d->cursor )
	    return;
	QSqlIndex lastSort = d->cursor->sort();
	QSqlIndex newSort( lastSort.tableName() );
	newSort.append( *d->cursor->field( indexOf( col ) ) );
	newSort.setDescending( 0, !ascending );
	horizontalHeader()->setSortIndicator( col, ascending );
	QApplication::setOverrideCursor( Qt::waitCursor );
	d->cursor->select( d->cursor->filter(), newSort );
	QApplication::restoreOverrideCursor();
	viewport()->repaint( FALSE );
    }
}

/*!

  \reimpl

*/

void QSqlTable::columnClicked ( int col )
{
    if ( sorting() ) {
	if ( !d->cursor )
	    return;
	QSqlIndex lastSort = d->cursor->sort();
	bool asc = TRUE;
	if ( lastSort.count() && lastSort.field( 0 )->name() == d->cursor->field( indexOf( col ) )->name() )
	    asc = lastSort.isDescending( 0 );
	sortColumn( col, asc );
    }
}

/*!
  \reimpl
 */
// void QSqlTable::paintFocus( QPainter * p, const QRect & cr )
// {
// //    QSqlTable::paintFocus( p, cr );
// }

/*!
  \reimpl
 */
void QSqlTable::repaintCell( int row, int col )
{
    QRect cg = cellGeometry( row, col );
    QRect re( QPoint( cg.x() - 2, cg.y() - 2 ),
	      QSize( cg.width() + 4, cg.height() + 4 ) );
    repaintContents( re, FALSE );
}

/*!

  \reimpl

  This function is reimplemented to render the cell at \a row, \a col
  with the value of the corresponding cursor field.  Depending on the
  current edit mode of the table, paintField() is called for the
  appropriate cursor field.

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

    if ( !d->cursor )
	return;
    if ( d->mode != QSqlTable::None ) {
	if ( row == d->editRow && d->editBuffer ) {
	    paintField( p, d->editBuffer->field( indexOf( col ) ), cr,
			selected );
	} else if ( row > d->editRow && d->mode == QSqlTable::Insert ) {
	    if ( d->cursor->seek( row - 1 ) )
		paintField( p, d->cursor->field( indexOf( col ) ), cr,
			    selected );
	} else {
	    if ( d->cursor->seek( row ) )
		paintField( p, d->cursor->field( indexOf( col ) ), cr,
			    selected );
	}
    }
    else {
	if ( d->cursor->seek( row ) )
	    paintField( p, d->cursor->field( indexOf( col ) ), cr, selected );
    }
}


/* Paints the \a field on the painter \a p. The painter has already
   been translated to the appropriate cell's origin where the \a field
   is to be rendered. cr describes the cell coordinates in the content
   coordinate system..

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
    if ( field->isNull() )
	text = nullText();
    else {
	const QVariant val = field->value();
	text = val.toString();
	if ( val.type() == QVariant::Bool )
	    text = val.toBool() ? d->trueTxt : d->falseTxt;
    }
    p->drawText( 2,2, cr.width()-4, cr.height()-4, fieldAlignment( field ), text );
}

int QSqlTable::fieldAlignment( const QSqlField* field )
{
    if ( !cursor() )
	return Qt::AlignLeft | Qt::AlignVCenter;
    return cursor()->alignment( field->name() ) | Qt::AlignVCenter;
}


/*!  Adds the fields in \a fieldList to the column header.
*/

void QSqlTable::addColumns( const QSqlRecord& fieldList )
{
    for ( uint j = 0; j < fieldList.count(); ++j )
	addColumn( fieldList.field(j) );
}

/*!

  If the \a sql driver supports query sizes, the number of rows in the
  table is set to the size of the query.  Otherwise, the table
  dynamically resizes itself as it is scrolled.

*/

void QSqlTable::setSize( const QSqlCursor* sql )
{
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

/*!

  Displays the \a cursor in the table.  If autopopulate is TRUE,
  columns are automatically created based upon the fields in the \a
  cursor record.  If the \a cursor is read only, the table becomes
  read only.  The table adopts the cursor's driver's definition
  representing NULL values as strings.

  \sa isReadOnly() setReadOnly() QSqlDriver::nullText()

*/

void QSqlTable::setCursor( QSqlCursor* cursor, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
    if ( cursor ) {
	d->cursor = cursor;
	if ( autoPopulate )
	    addColumns( *d->cursor );
	if ( !d->cursor->isActive() )
	    d->cursor->select();
	setSize( d->cursor );
	setReadOnly( d->cursor->isReadOnly() );
	setNullText(d->cursor->driver()->nullText() );
    }
    setUpdatesEnabled( TRUE );
}


/*!

  Protected virtual function which is called when an error has
  occurred on the current cursor().  The default implementation displays
  a warning message to the user with information about the error.

*/
void QSqlTable::handleError( const QSqlError& e )
{
    QMessageBox::warning ( this, "Warning", e.driverText() + "\n" + e.databaseText(), 0, 0 );
}

/*!

  Returns a pointer to the cursor associated with the table, or 0 if
  there is no current cursor.

*/

QSqlCursor* QSqlTable::cursor() const
{
    return d->cursor;
}

/*!

  \reimpl

*/

void QSqlTable::resizeData ( int )
{

}

/*!

  \reimpl

*/

QTableItem * QSqlTable::item ( int, int ) const
{
    return 0;
}

/*!

  \reimpl

*/

void QSqlTable::setItem ( int , int , QTableItem * )
{

}

/*!

  \reimpl

*/

void QSqlTable::clearCell ( int , int )
{

}

/*!

  \reimpl

*/

void QSqlTable::setPixmap ( int , int , const QPixmap &  )
{

}

/*!

  \reimpl

*/

void QSqlTable::takeItem ( QTableItem * )
{

}

void QSqlTable::refresh( QSqlIndex idx )
{
    if ( d->cursor )
	refresh( d->cursor, idx );
}

/*!

  Installs a new SQL editor factory. This enables the user to create
  and instantiate their own editors for use in cell editing.  Note that
  QSqlTable takes ownership of this pointer, and will delete it when
  it is no longer needed or when installEditorFactory() is called again.

  \sa QSqlEditorFactory
*/

void QSqlTable::installEditorFactory( QSqlEditorFactory * f )
{
    if( f ) {
	delete d->editorFactory;
	d->editorFactory = f;
    }
}

/*!

  Installs a new property map. This enables the user to create and
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

/*!

  \internal

*/

void QSqlTable::setCurrentSelection( int row, int )
{
    if ( !d->cursor )
	return;
    if ( row == d->lastAt )
	return;
    if ( !d->cursor->seek( row ) )
	return;
    d->lastAt = row;
    emit currentChanged( d->cursor );
}

/*!

  Returns the currently selected record, or an empty record if there
  is no current selection.

*/

QSqlRecord QSqlTable::currentFieldSelection() const
{
    QSqlRecord fil;
    if ( !d->cursor || currentRow() < 0 )
	return fil;
    int row = currentRow();
    if ( !d->cursor->seek( row ) )
	return fil;
    fil = *d->cursor;
    return fil;
}

#endif



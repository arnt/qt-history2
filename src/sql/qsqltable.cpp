#include "qapplication.h"
#include "qsqltable.h"
#include "qsqldriver.h"
#include "qsqleditorfactory.h"
#include "qsqlform.h"
#include "qlayout.h"
#include "qpopupmenu.h"

#ifndef QT_NO_SQL

void qt_debug_buffer( const QString& msg, QSqlView* view )
{
    //    qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    //    qDebug(msg);
    for ( uint j = 0; j < view->count(); ++j ) {
	//	qDebug(view->field(j)->name() + " type:" + QString(view->field(j)->value().typeName()) + " value:" + view->field(j)->value().toString() );
    }
}

enum {
    IdInsert,
    IdUpdate,
    IdDelete
};


class QSqlTablePrivate
{
public:
    QSqlTablePrivate()
	: nullTxt( "NULL" ),
	  haveAllRows(FALSE),
	  continuousEdit(FALSE),
	  ro(TRUE),
	  editorFactory(0),
	  propertyMap(0),
	  view(0),
	  insertMode( FALSE ),
	  editRow(-1),
	  insertRowLast(-1),
	  insertHeaderLabelLast(),
	  updateMode( FALSE )
    {}
    ~QSqlTablePrivate() { if ( propertyMap ) delete propertyMap; }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;
    bool         continuousEdit;

    bool         ro;
    QSqlEditorFactory* editorFactory;
    QSqlPropertyMap* propertyMap;
    QString trueTxt;
    QString falseTxt;
    QSqlView* view;
    bool insertMode;
    int editRow;
    int insertRowLast;
    QString insertHeaderLabelLast;
    QSqlView editBuffer;
    bool updateMode;
};

/*!
  \class QSqlTable qsqltable.h
  \module database

  \brief A flexible and editable SQL table widget.

  QSqlTable supports various methods for presenting and editing SQL data.

  When displaying data, QSqlTable only retrieves data for visible
  rows.  If drivers do not support the 'query size' property, rows are
  dynamically fetched from the database on an as-needed basis.  This
  allows extremely large queries to be displayed as quickly as
  possible, with limited memory usage.

  QSqlTable also offers an API for sorting columns. See setSorting()
  and sortColumn().

  When displaying QSqlViews, cell editing can be enabled with
  setCellEditing().  QSqlTable creates editors.... ### The user can
  create their own special editors by ... ###

*/

/*!
  Constructs a SQL table.

*/

QSqlTable::QSqlTable ( QWidget * parent, const char * name )
    : QTable(parent,name)
{
    d = new QSqlTablePrivate();
    setSelectionMode( NoSelection );
    d->editorFactory = new QSqlEditorFactory( this, "qt_default_qsqleditorfactory");
    d->propertyMap = new QSqlPropertyMap();
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
    delete d;
}


/*!

  Adds \a field as the next column to be diplayed.  By default, fields
  which are not visible and fields which are part of a table's primary
  index are not displayed.

  \sa QSqlField

*/

void QSqlTable::addColumn( const QSqlField* field )
{
    if ( field->isVisible() && !field->isPrimaryIndex() ) {
	setNumCols( numCols() + 1 );
	d->colIndex.append( field->fieldNumber() );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, field->displayLabel() );
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
    if ( field->isVisible() && !field->isPrimaryIndex() ) {
	d->colIndex[ col ] = field->fieldNumber();
	QHeader* h = horizontalHeader();
	h->setLabel( col, field->name() );
    } else {
	removeColumn( col );
    }
}

/*!

  Sets the table's readonly flag to \a b.  Note that if the underlying view cannot
  be edited, this function will have no effect.

  \sa setView()

*/

void QSqlTable::setReadOnly( bool b )
{
    d->ro = b;
}

bool QSqlTable::isReadOnly() const
{
    return d->ro;
}

/*!

  \reimpl

  For an editable table, creates an editor suitable for the data type
  in \a row and \a col.

  \sa QSqlEditorFactory QSqlPropertyMap

*/

QWidget * QSqlTable::createEditor( int , int col, bool initFromCell ) const
{
    qDebug("QSqlTable::createEditor( int , int col, bool initFromCell ) const");
    if ( !d->insertMode && !d->updateMode )
	return 0;
    QWidget * w = 0;
    if( initFromCell ){
	w = d->editorFactory->createEditor( viewport(), d->editBuffer.value( indexOf( col ) ) );
	qDebug("editor factory returned:" + QString::number((int)w));
	d->propertyMap->setProperty( w, d->editBuffer.value( indexOf( col ) ) );
    } else {
	qDebug("not init from cell");
    }
    return w;
}

bool QSqlTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QTable::eventFilter( o, e );

    int r = currentRow();
    int c = currentColumn();
    QWidget *editorWidget = cellWidget( r, c );
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Escape && d->insertMode )
	    endInsert();
	if ( ke->key() == Key_Escape && d->updateMode )
	    endUpdate();
	if ( ke->key() == Key_Insert && !d->insertMode && !d->updateMode ) {
	    beginInsert();
	    return TRUE;
	}
	if ( ke->key() == Key_Delete && !d->insertMode && !d->updateMode ) {
	    deleteCurrent();
	    return TRUE;
	}
	if ( editorWidget && o == editorWidget ) {
	    if ( ( ke->key() == Key_Tab ) && ( c < numCols() - 1 ) )
		d->continuousEdit = TRUE;
	    else if ( ( ke->key() == Key_BackTab ) && ( c > 0 ) )
		d->continuousEdit = TRUE;
	    else
		d->continuousEdit = FALSE;
	}
	break;
    }
    default:
	break;
    }
    return QTable::eventFilter( o, e );
}

/*
  \reimpl

*/

void QSqlTable::mousePressEvent( QMouseEvent *e )
{
    if ( !d->view )
	return;
    if ( e->button() == RightButton ) {
	QPopupMenu *popup = new QPopupMenu( this );
	int id[ 3 ];
	id[ IdInsert ] = popup->insertItem( tr( "Insert" ) );
	id[ IdUpdate ] = popup->insertItem( tr( "Update" ) );
	id[ IdDelete ] = popup->insertItem( tr( "Delete" ) );
	popup->setItemEnabled( id[ IdInsert ], !isReadOnly() && d->view->canInsert() );
	popup->setItemEnabled( id[ IdUpdate ], !isReadOnly() && d->view->canUpdate() );
	popup->setItemEnabled( id[ IdDelete ], !isReadOnly() && d->view->canDelete() );
	int r = popup->exec( e->globalPos() );
	delete popup;
	if ( r == id[ IdInsert ] )
	    beginInsert();
	else if ( r == id[ IdUpdate ] ) {
	    qDebug("updating");
	    //	    viewport()->setFocus();
	    if ( beginEdit( currentRow(), currentColumn(), FALSE ) ) {
		qDebug("setting edit mode");
		setEditMode( Editing, currentRow(), currentColumn() );
	    } else {
		qDebug("could not begin edit");
		endUpdate();
	    }
	}
	else if ( r == id[ IdDelete ] )
	    deleteCurrent();
	return;
    }
}

/*
  \reimpl

*/

QWidget* QSqlTable::beginEdit ( int row, int col, bool replace )
{
    if ( !d->view )
	return 0;
    qDebug("beginEdit row:" + QString::number(row) + " col:" + QString::number(col));
    qDebug("continuousEdit:" + QString::number(d->continuousEdit));
    if ( d->continuousEdit )
	return QTable::beginEdit( row, col, replace );
    if ( !d->insertMode && !d->updateMode )
	return beginUpdate( row, col, replace );
    qDebug("NO EDIT, returning 0");
    return 0;
}

/*
  \internal

*/

void QSqlTable::endInsert()
{
    //qDebug("QSqlTable::endInsert()");
    d->insertMode = FALSE;
    d->editBuffer.clear();
    int i;
    for ( i = d->editRow; i <= d->insertRowLast; ++i )
	updateRow( i );
    for ( i = d->editRow; i < d->insertRowLast; ++i )
	verticalHeader()->setLabel( i, verticalHeader()->label( i+1 ) );
    verticalHeader()->setLabel( d->insertRowLast, d->insertHeaderLabelLast );
    d->editRow = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
}

void QSqlTable::endUpdate()
{
    //qDebug("endUpdate()");
    d->updateMode = FALSE;
    updateRow( d->editRow );
    d->editRow = -1;
}

bool QSqlTable::beginInsert()
{
    QSqlView* vw = d->view;
    if ( !vw || isReadOnly() || ! numCols() )
	return FALSE;
    int i = 0;
    int row = currentRow();
    if ( row < 0 )
	return FALSE;
    ensureCellVisible( row, 0 );
    setCurrentCell( row, 0 );
    qt_debug_buffer("beginInsert: before creating edit buffer, VIEW", d->view);
    d->editBuffer.clear();
    d->editBuffer = *vw;
    d->editBuffer.detach();
    d->editBuffer.clearValues();
    qt_debug_buffer("beginInsert: after creating edit buffer", &d->editBuffer);
    if ( !primeInsert( &d->editBuffer ) )
	return FALSE;
    d->editRow = row;
    d->insertMode = TRUE;
    int lastRow = row;
    int lastY = contentsY() + visibleHeight();
    for ( i = row; i < numRows() ; ++i ) {
	QRect cg = cellGeometry( i, 0 );
	if ( (cg.y()+cg.height()) > lastY ) {
	    lastRow = i;
	    break;
	}
    }
    d->insertRowLast = lastRow;
    d->insertHeaderLabelLast = verticalHeader()->label( d->insertRowLast );
    for ( i = lastRow; i > row; --i )
	verticalHeader()->setLabel( i, verticalHeader()->label( i-1 ) );
    verticalHeader()->setLabel( row, "*" );
    for ( i = d->editRow; i <= d->insertRowLast; ++i )
	updateRow( i );
    //qDebug("QSqlTable::beginInsert(), about to begin edit");
    if ( QTable::beginEdit( row, 0, FALSE ) )
	setEditMode( Editing, row, 0 );
    //qDebug("QSqlTable::beginInsert(), DONE");
    return TRUE;
}

QWidget* QSqlTable::beginUpdate ( int row, int col, bool replace )
{
    qDebug("QSqlTable::beginUpdate");
    ensureCellVisible( row, col );
    setCurrentSelection( row, col );
    d->updateMode = TRUE;
    d->editRow = row;
    d->editBuffer.clear();
    d->editBuffer = *d->view;
    d->editBuffer.detach();
    return QTable::beginEdit( row, col, replace );
}

/*!  Protected virtual method which is called to "prime" the fields of
  a view before an insert is performed.  This can be used, for
  example, to prime any auto-numbering or unique index fields.
  This method should return TRUE if the "prime" was successful and the
  insert should continue.  Returning FALSE will prevent the insert from
  proceeding.  The default implementation returns TRUE.

  \sa insertCurrent()
*/

bool QSqlTable::primeInsert( QSqlView* )
{
    return TRUE;
}

/*!  For an editable table, issues an insert on the current view using
  the values of the currently edited "insert" row.  If there is no
  current view or there is no current "insert" row, nothing happens.
  Returns TRUE if the insert succeeded, otherwise FALSE.

  \sa primeFields
*/

bool QSqlTable::insertCurrent()
{
    //qDebug("QSqlTable::insertCurrent()");
    if ( !d->view || isReadOnly() || ! numCols() )
	return FALSE;
    if ( !d->editBuffer.canInsert() ) {
#ifdef CHECK_RANGE
	qWarning("QSqlTable::insertCurrent: insert not allowed for " + d->editBuffer.name() );
#endif
	return FALSE;
    }
    QApplication::setOverrideCursor( Qt::waitCursor );    
    bool b = d->editBuffer.insert();
    QApplication::restoreOverrideCursor();
    endInsert();
    refresh( d->view );
    setCurrentSelection( currentRow(), currentColumn() );
    return b;
}

void QSqlTable::updateRow( int row )
{
    for ( int i = 0; i < numCols(); ++i )
	updateCell( row, i );
}

void QSqlTable::columnWidthChanged( int col )
{
    //qDebug(QString(name()) + "QSqlTable::columnWidthChanged( int col )");
    QTable::columnWidthChanged(col);
}

/*!  Protected virtual method which is called to "prime" the fields of
  a view before an update is performed.  This method should return
  TRUE if the "prime" was successful and the update should continue.
  Returning FALSE will prevent the update from proceeding.  The
  default implementation returns TRUE.

  \sa updateCurrent()
*/

bool QSqlTable::primeUpdate( QSqlView* )
{
    return TRUE;
}


/*!  For an editable table, issues an update on the current view's
  primary index using the values of the edited selected row.  If
  there is no current view or there is no current selection, nothing
  happens.  Returns TRUE if the update succeeded, otherwise FALSE.

  For this method to succeed, the underlying view must have a valid
  primary index to ensure that a unique record is updated within the
  database.

*/

bool QSqlTable::updateCurrent()
{
    if ( !d->updateMode )
	return FALSE;
    d->updateMode = FALSE;
    if ( d->editBuffer.primaryIndex().count() == 0 ) {
#ifdef CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: no primary index for " + d->editBuffer.name() );
#endif
	return FALSE;
    }
    if ( !d->editBuffer.canUpdate() ) {
#ifdef CHECK_RANGE
	qWarning("QSqlTable::updateCurrent: updates not allowed for " + d->editBuffer.name() );
#endif
	return FALSE;
    }
    if ( !primeUpdate( &d->editBuffer ) )
	return FALSE;
    qt_debug_buffer("updateCurrent: edit buffer (before update)", &d->editBuffer);
    QApplication::setOverrideCursor( Qt::waitCursor );
    bool b = d->editBuffer.update( d->editBuffer.primaryIndex() );
    QApplication::restoreOverrideCursor();    
    refresh( d->view );
    setCurrentSelection( currentRow(), currentColumn() );
    return b;
}

/*!  Protected virtual method which is called to "prime" the fields of
  a view before a delete is performed.  This method should return
  TRUE if the "prime" was successful and the delete should continue.
  Returning FALSE will prevent the delete from proceeding.  The
  default implementation returns TRUE.

  \sa deleteCurrent()
*/

bool QSqlTable::primeDelete( QSqlView* )
{
    return TRUE;
}

/*!  For an editable table, issues a delete on the current view's
  primary index using the values of the currently selected row.  If
  there is no current view or there is no current selection, nothing
  happens.  Returns TRUE if the delete succeeded, otherwise FALSE.

  For this method to succeed, the underlying view must have a valid
  primary index to ensure that a unique record is deleted within the
  database.

*/
bool QSqlTable::deleteCurrent()
{
    QSqlView* vw = d->view;
    if ( !vw || isReadOnly() )
	return FALSE;
    if ( vw->primaryIndex().count() == 0 ) {
#ifdef CHECK_RANGE
	qWarning("QSqlTable::deleteCurrent: no primary index " + vw->name() );
#endif
	return FALSE;
    }
    if ( !vw->canDelete() )
	return FALSE;
    if ( !vw->seek( currentRow() ) )
	return FALSE;
    if ( !primeDelete( vw ) )
	return FALSE;
    QApplication::setOverrideCursor( Qt::waitCursor );    
    bool b = vw->del( vw->primaryIndex() );
    QApplication::restoreOverrideCursor();
    refresh( vw );
    setCurrentSelection( currentRow(), currentColumn() );
    updateRow( currentRow() );
    return b;
}

/*!  Refreshes the \a view.  A "select" is issued on the \a view using
  the view's current filter and current sort.  The table is resized to accomodate the view size, if possible.

  \sa QSqlView
*/


void QSqlTable::refresh( QSqlView* view, bool seekPrimary )
{
    QSqlIndex pi;
    if ( seekPrimary )
	pi = view->primaryIndex( TRUE );
    QApplication::setOverrideCursor( Qt::waitCursor );
    view->select( view->filter(), view->sort() );
    QApplication::restoreOverrideCursor();
    setSize( view );
    //    viewport()->repaint();
}

/*!
  \reimpl

*/

void QSqlTable::setCellContentFromEditor( int row, int col )
{
    //    qDebug("setCellContentFromEditor( int row, int col )");
    if ( !d->insertMode && !d->updateMode )
	return;
    QWidget * editor = cellWidget( row, col );
    if ( !editor )
	return;
    //    qDebug("setting edit buffer value to :" + d->propertyMap->property( editor ).toString() );
    //    qDebug("propertyMap property type:" + QString(d->propertyMap->property( editor ).typeName()) + " val:" + d->propertyMap->property( editor ).toString() );
    d->editBuffer.setValue( indexOf( col ),  d->propertyMap->property( editor ) );
    qt_debug_buffer("setCellContentFromEditor: edit buffer", &d->editBuffer);
    if ( !d->continuousEdit ) {
	if ( d->insertMode )
	    insertCurrent();
	else
	    updateCurrent();
    }
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

    QSqlView * rset = d->view;
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
			ensureCellVisible( row, i );
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

  \sa setSql() setView()

*/

void QSqlTable::reset()
{
    clearCellWidget( currentRow(), currentColumn() );    
    if ( d->insertMode )
	endInsert();
    if ( d->updateMode )
	endUpdate();
    ensureVisible( 0, 0 );
    verticalScrollBar()->setValue(0);
    setNumRows(0);
    setNumCols(0);
    d->view = 0;
    d->haveAllRows = FALSE;
    d->continuousEdit = FALSE;
    d->colIndex.clear();
    d->insertMode =  FALSE;
    d->editRow = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    d->updateMode =  FALSE;
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
    //qDebug("QSqlTable::text ( int row, int col ) const");
    QSql* sql = d->view;
    if ( !sql )
	return QString::null;
    if ( sql->seek( row ) )
	return sql->value( indexOf( col ) ).toString();
    return QString::null;
}

/*!

   Returns the value in cell \a row, \a col, or an invalid value if
   the relevant item does not exist or includes no text.

*/

QVariant QSqlTable::value ( int row, int col ) const
{
    //qDebug("QSqlTable::value ( int row, int col ) const");
    QSql* sql = d->view;
    if ( !sql )
	return QVariant();
    if ( sql->seek( row ) )
	return sql->value( indexOf( col ) );
    return QVariant();
}


/*!

  \internal

*/

void QSqlTable::loadNextPage()
{
    if ( d->haveAllRows )
	return;
    QSql* sql = d->view;
    if ( !sql )
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
    while ( endIdx > 0 && !sql->seek( endIdx ) )
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
	QSqlView* rset = d->view;
	if ( !rset )
	    return;
	QSqlIndex lastSort = rset->sort();
	QSqlIndex newSort( lastSort.tableName() );
	newSort.append( rset->field( indexOf( col ) ) );
	newSort.setDescending( 0, !ascending );
	horizontalHeader()->setSortIndicator( col, ascending );
	QApplication::setOverrideCursor( Qt::waitCursor );
	rset->select( rset->filter(), newSort );
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
	QSqlView* rset = d->view;
	if ( !rset )
	    return;
	QSqlIndex lastSort = rset->sort();
	bool asc = TRUE;
	if ( lastSort.count() && lastSort.field( 0 )->name() == rset->field( indexOf( col ) )->name() )
	    asc = lastSort.isDescending( 0 );
	sortColumn( col, asc );
    }
}

/*!

  \reimpl

  This function is reimplemented to render the cell at \a row, \a col
  with the value of the corresponding view field.  Depending on the
  current edit mode of the table, paintField() is called for the
  appropriate view field.

  \sa QSqlView::isNull()
*/

void QSqlTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    //    //qDebug("QSqlTable::paintCell");
    QTable::paintCell(p,row,col,cr,selected);  // empty cell
    if ( !d->view )
	return;
    if ( d->insertMode || d->updateMode ) {
	if ( row == d->editRow ) {
	    //	    qDebug("painting editRow:" + QString::number(row));
	    //	    qDebug("field name:" + d->editBuffer.field( indexOf( col ) )->name() + " type:" + QString(d->editBuffer.field( indexOf( col ) )->value().typeName()));
	    paintField( p, d->editBuffer.field( indexOf( col ) ), cr, selected );
	} else if ( row > d->editRow && d->insertMode ) {
	    //	    //qDebug("trying to paint row:" + QString::number(row));
	    if ( d->view->seek( row - 1 ) )
		paintField( p, d->view->field( indexOf( col ) ), cr, selected );
	} else {
	    //	    //qDebug("seeking in update mode");
	    if ( d->view->seek( row ) )
		paintField( p, d->view->field( indexOf( col ) ), cr, selected );
	}
    }
    else {
	//	//qDebug("paint SEEKing row:" + QString::number(row));
	if ( d->view->seek( row ) )
	    paintField( p, d->view->field( indexOf( col ) ), cr, selected );
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

void QSqlTable::paintField( QPainter * p, const QSqlField* field, const QRect & cr,
			     bool  )
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
    p->drawText( 0,0, cr.width(), cr.height(), fieldAlignment( field ), text );
}

int QSqlTable::fieldAlignment( const QSqlField* field )
{
    bool num;
    bool ok1 = FALSE, ok2 = FALSE;
    QString txt = field->value().toString();
    (void)txt.toInt( &ok1 );
    if ( !ok1 )
	(void)txt.toDouble( &ok2 );
    num = ok1 || ok2;
    return ( num ? AlignRight : AlignLeft ) | AlignVCenter;
}


/*!  Adds the fields in \a fieldList to the column header.
*/

void QSqlTable::addColumns( const QSqlFieldList& fieldList )
{
    for ( uint j = 0; j < fieldList.count(); ++j )
	addColumn( fieldList.field(j) );
}


/*!

  If the \a sql driver supports query sizes, the number of rows in the
  table is set to the size of the query.  Otherwise, the table
  dynamically resizes itself as it is scrolled.

*/

void QSqlTable::setSize( const QSql* sql )
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

  Displays the \a view in the table.  If autopopulate is TRUE, columns
  are automatically created based upon the fields in the \a view.

*/

void QSqlTable::setView( QSqlView* view, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
    if ( view ) {
	d->view = view;
	if ( autoPopulate )
	    addColumns( *d->view );
	setSize( d->view );
	setReadOnly( d->view->isReadOnly() );
    }
    setUpdatesEnabled( TRUE );
}

/*!

  Returns a pointer to the view associated with the table, or 0 if
  there is no current view.

*/

QSqlView* QSqlTable::view() const
{
    return d->view;
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

  Installs a new SQL property map. This enables the user to create and
  instantiate their own property maps for use in cell editing.  Note
  that QSqlTable takes ownership of this pointer, and will delete it
  when it is no longer needed or when installPropertMap() is called
  again.

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
    QSql* sql = d->view;
    if ( !sql )
	return;
    if ( !sql->seek( row ) )
	return;
    QSqlFieldList fil = sql->fields();
    emit currentChanged( &fil );
}

/*!

  Returns a list of the currently selected fields, or an empty list if there is no current selection.

*/

QSqlFieldList QSqlTable::currentFieldSelection() const
{
    QSqlFieldList fil;
    QSql* sql = d->view;
    if ( !sql || currentRow() < 0 )
	return fil;
    int row = currentRow();
    if ( !sql->seek( row ) )
	return fil;
    fil = sql->fields();
    return fil;
}

#endif



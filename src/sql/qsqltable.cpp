#include "qapplication.h"
#include "qsqltable.h"
#include "qsqldriver.h"
#include "qsqlpropertymanager.h"
#include "qsqleditorfactory.h"

#ifndef QT_NO_SQL

class QSqlTablePrivate
{
public:
    QSqlTablePrivate() : view(0) {}
    
    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;
    
    QSqlEditorFactory* editorFactory;
    QString trueTxt;
    QString falseTxt;
    QSqlView* view;
};


/*!
  \class QSqlTable qsqltable.h
  \module database

  \brief A flexible and editable SQL table widget.

  QSqlTable supports various methods for presenting SQL data.

  When displaying data, QSqlTable only retrieves data for visible
  rows.  If drivers do not support the 'query size' property, rows are
  dynamically fetched from the database on an as-needed basis.  This
  allows extremely large queries to be displayed as quickly as
  possible, with limited memory usage.

  QSqlTable also offers an API for sorting columns. See setSorting()
  and sortColumn().  When displaying QSql data, sorting is disabled
  since it is not always possible to sort on the fields within QSql,
  or the QSql query may already contain an ORDER BY.

  When displaying QSqlView, cell editing can be enabled with
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
    d->editorFactory = new QSqlEditorFactory( this, "Default QSqlEditorFactory");
    d->trueTxt = tr( "True" );
    d->falseTxt = tr( "False" );
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

  \reimpl

*/

QWidget * QSqlTable::createEditor( int row, int col, bool initFromCell ) const
{
    QSqlView* vw = d->view;
    if ( !vw )
	return 0;
    QSqlPropertyManager m;
    QWidget * w = 0;

    m.addClass( "QSqlCustomEd", "state" );
    if( initFromCell && vw->seek( row ) ){
	w = d->editorFactory->createEditor( viewport(), vw->value( indexOf( col ) ) );
	m.setProperty( w, vw->value( indexOf( col ) ) );
    }
    return w;
}

/*!

  \reimpl

*/

void QSqlTable::setCellContentFromEditor( int row, int col )
{
    QSqlView* vw = d->view;
    if ( !vw )
	return;
    QSqlPropertyManager m;
    QWidget * editor = cellWidget( row, col );

    if ( !editor )
	return;
    vw->seek( row );
    (*vw)[ indexOf( col )] = m.property( editor );
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
    ensureVisible( 0, 0 );
    verticalScrollBar()->setValue(0);
    setNumRows(0);
    setNumCols(0);
    d->view = 0;
    d->haveAllRows = FALSE;
    d->colIndex.clear();
    if ( sorting() ) {
	horizontalHeader()->setSortIndicator( -1 );
	setSorting( FALSE );
    }
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
	rset->select( newSort );
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

*/

void QSqlTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    // ###
    QTable::paintCell(p,row,col,cr,selected);
    QSql* sql = d->view;
    if ( !sql )
	return;
    if ( sql->seek( row ) ) {
	QString text;
	if ( sql->isNull( indexOf(col) ) )
	    text = nullText();
	else {
	    QVariant val = sql->value( indexOf(col) );
	    text = val.toString();
	    if ( val.type() == QVariant::Bool )
		text = val.toBool() ? d->trueTxt : d->falseTxt;
	}
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
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

  Displays the \a view in the table.  If autopoulate is TRUE, columns
  are automatically created based upon the fields in the \a view.

*/

void QSqlTable::setView( QSqlView* view, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    setSorting( FALSE );
    reset();
    d->view = view;
    if ( autoPopulate )
	addColumns( *d->view );
    setSize( d->view );
    setUpdatesEnabled( TRUE );
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
  and instantiate their own editors for the different cells in a
  table.  Note that QSqlTable takes ownership of this pointer, and
  will delete it when it is no longer needed or when
  setEditorFactory() is called again.

*/

void QSqlTable::setEditorFactory( QSqlEditorFactory * f )
{
    if( f ) {
	delete d->editorFactory;
	d->editorFactory = f;
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
    if ( sql->seek( row ) ) {
	QSqlFieldList fil = sql->fields();
	emit currentChanged( &fil );
    }
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
    if ( sql->seek( currentRow() ) )
	fil = sql->fields();
    return fil;
}

#endif



#include "qsqltable.h"

#ifndef QT_NO_SQL

class QSqlTablePrivate
{
public:
    QSqlTablePrivate() : nullTxt("<null>"), haveAllRows(FALSE), s( 0 ) {}
    ~QSqlTablePrivate() { if ( s ) delete s; }

    enum Mode {
	Sql,
	Rowset,
	View
    };

    void resetMode( Mode m )
    {
	delete s;
	switch( m ) {
	case Sql:
	    s = new QSql();
	    break;
	case Rowset:
	    s = new QSqlRowset();
	    break;
	case View:
	    s = new QSqlView();
	    break;
	}
	mode = m;
    }

    QSql* sql()
    {
	return s;
    }

    QSqlRowset* rowset()
    {
	if ( mode == Rowset || mode == View ) {
	    return (QSqlRowset*)s;
	}
	return 0;
    }

    QSqlView* view()
    {
	if ( mode == View )
	    return (QSqlView*)s;
	return 0;
    }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;

private:
    QSql* s;
    Mode mode;
};


/*!
  \class QSqlTable qsqltable.h
  \module database

  \brief A flexible and editable SQL table widget.

  QSqlTable supports various methods for presenting SQL data.  General SQL
  queries (QSql), read-only rowsets (QSqlRowset) and updatable SQL tables or
  views (QSqlView) can all be displayed.

  When displaying data, QSqlTable only retrieves data for visible
  rows.  If drivers do not support the 'query size' property, rows are
  dynamically fetched from the database on an as-needed basis.  This
  allows extremely large queries to be displayed a s quickly as
  possible, with limited memory usage.

  QSqlTable also offers an API for sorting columns. See setSorting()
  and sortColumn().  When displaying QSql data, sorting is disabled
  since it is not always possible to sort on the fields within QSql,
  or the QSql query may already contain an ORDER BY.

  When displaying QSqlView, cell editing can be enabled with setCellEditing().
  QSqlTable creates editors.... ###  The user can create their own special editors by ... ###

*/


/*!
  Constructs a SQL table.

*/

QSqlTable::QSqlTable ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name)
{
    d = new QSqlTablePrivate();
    setSelectionMode( NoSelection );
    setVScrollBarMode( AlwaysOn );
    connect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
	     this, SLOT( loadLine(int) ) );
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

void QSqlTable::addColumn( const QSqlField& field )
{
    qDebug("adding col:" + field.name());
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	qDebug("current num cols:" + QString::number(numCols()));
	setNumCols( numCols() + 1 );
	qDebug("new num cols:" + QString::number(numCols()));
	d->colIndex.append( field.fieldNumber() );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, field.displayLabel() );
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

void QSqlTable::setColumn( uint col, const QSqlField& field )
{
    if ( col >= (uint)numCols() )
	return;
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	d->colIndex[ col ] = field.fieldNumber();
	QHeader* h = horizontalHeader();
	h->setLabel( col, field.name() );
    } else {
	removeColumn( col );
    }
}

/*!

  \reimpl

*/

QWidget * QSqlTable::createEditor( int row, int col, bool initFromCell ) const
{
    return 0;
    Q_UNUSED( row );
    Q_UNUSED( col );
    Q_UNUSED( initFromCell );
}

/*!

  Resets the table so that it displays no data.  This is called
  internally before displaying a new query.

  \sa setSql() setRowset() setView()

*/

void QSqlTable::reset()
{
    ensureVisible( 0, 0 );    
    verticalScrollBar()->setValue(0);    
    setNumRows(0);
    setNumCols(0);
    d->haveAllRows = FALSE;
    d->colIndex.clear();
}

/*!

  Returns the index of the field within the current SQL query based on
  the displayed column \a i.

*/

int QSqlTable::indexOf( uint i )
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

QString QSqlTable::nullText()
{
    return d->nullTxt;
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
    QSql* sql = d->sql();
    if ( !sql )
	return QString::null;
    if ( sql->seek( row ) )
	return sql->value( col ).toString();
    return QString::null;
}

/*!

   Returns the value in cell \a row, \a col, or an invalid value if
   the relevant item does not exist or includes no text.

*/

QVariant QSqlTable::value ( int row, int col ) const
{
    QSql* sql = d->sql();
    if ( !sql )
	return QVariant();
    if ( sql->seek( row ) )
	return sql->value( col );
    return QVariant();
}


/*!

  \internal

*/

void QSqlTable::loadNextPage()
{
    if ( d->haveAllRows )
	return;
    QSql* sql = d->sql();
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

void QSqlTable::sortColumn ( int col, bool ascending = TRUE,
			      bool  )
{
    if ( sorting() ) {
	QSqlRowset* rset = d->rowset();
	if ( !rset )
	    return;
	QSqlIndex lastSort = rset->sort();
	QSqlIndex newSort( lastSort.tableName() );
	newSort.append( rset->field( col ) );
	newSort.setDescending( 0, !ascending );
	horizontalHeader()->setSortIndicator( col, ascending );
	rset->select( newSort );
	repaintContents( 0, 0, viewport()->width(), viewport()->height() );
    }
}

/*!

  \reimpl

*/

void QSqlTable::columnClicked ( int col )
{
    if ( sorting() ) {
	QSqlRowset* rset = d->rowset();
	if ( !rset )
	    return;
	QSqlIndex lastSort = rset->sort();
	bool asc = TRUE;
	if ( lastSort.count() && lastSort.field( 0 ).name() == rset->field( col ).name() )
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
    // ### need to have smarter painting
    QTable::paintCell(p,row,col,cr,selected);
    QSql* sql = d->sql();
    if ( !sql )
	return;
    if ( sql->seek(row) ) {
	QString text = sql->value( indexOf(col) ).toString();
	if ( sql->isNull( indexOf(col) ) )
	    text = nullText();
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}

/*!

  Displays the SQL \a query in the table.  By default, SQL queries
  cannot be sorted.  If a \a databaseName is not specified, the
  default database connection is used.  If autopoulate is TRUE,
  columns are automatically created based upon the \a query.

*/

void QSqlTable::setQuery( const QString& query, const QString& databaseName, bool autoPopulate )
{
    QSql s( databaseName );
    s.setQuery( query );
    setQuery( s, autoPopulate );
}

/*

  \overload

*/

void QSqlTable::setQuery( const QSql& query, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    setSorting( FALSE );
    reset();
    d->resetMode( QSqlTablePrivate::Sql );
    QSql* sql = d->sql();
    (*sql) = query;
    if ( autoPopulate ) {
	QSqlFieldList fl = sql->fields();
	for ( uint j = 0; j < fl.count(); ++j )
	    addColumn( fl.field(j) );
    }
    loadNextPage();
    setUpdatesEnabled( TRUE );
}
/*!

  Displays the rowset \a name in the table.  By default, the rowset
  can be sorted.  If a \a databaseName is not specified, the
  default database connection is used.  If autopoulate is TRUE,
  columns are automatically created based upon the \a query.

*/

void QSqlTable::setRowset( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlRowset r( name, databaseName );
    setRowset( r, autoPopulate );
}

/*

  \overload

*/

void QSqlTable::setRowset( const QSqlRowset& rowset, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();        
    setSorting( TRUE );
    d->resetMode( QSqlTablePrivate::Rowset );
    QSqlRowset* rset = d->rowset();
    (*rset) = rowset;
    rset->select( rowset.sort() );
    if ( autoPopulate ) {
	for ( uint j = 0; j < rset->count(); ++j )
	    addColumn( rset->field(j) );
    }
    loadNextPage();
    setUpdatesEnabled( TRUE );
}

/*!

  Displays the view \a name in the table.  By default, the view
  can be sorted and edited.  If a \a databaseName is not specified, the
  default database connection is used.  If autopoulate is TRUE,
  columns are automatically created based upon the \a query.

*/

void QSqlTable::setView( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlView v( name, databaseName );
    setView( v, autoPopulate );
}

void QSqlTable::setView( const QSqlView& view, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    setSorting( TRUE );
    reset();
    d->resetMode( QSqlTablePrivate::View );
    QSqlView* vw = d->view();
    (*vw) = view;
    vw->select( view.sort() );
    if ( autoPopulate ) {
	for ( uint j = 0; j < vw->count(); ++j )
	    addColumn( vw->field(j) );
    }
    loadNextPage();
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

#endif

#include "qsqltable.h"

#ifndef QT_NO_SQL

class QSqlTablePrivate
{
public:
    QSqlTablePrivate() : s( 0 ) {}
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
    
private:
    QSql* s;
    Mode mode;
};

QSqlTable::QSqlTable ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name), nullTxt("<null>"), haveAllRows(FALSE)
{
    d = new QSqlTablePrivate();
    setSelectionMode( NoSelection );
    setVScrollBarMode( AlwaysOn );
    connect( verticalScrollBar(), SIGNAL( nextPage() ),
	     this, SLOT( loadNextPage() ) );
    connect( verticalScrollBar(), SIGNAL( nextLine() ),
	     this, SLOT( loadNextLine() ) );
    connect( verticalScrollBar(), SIGNAL( sliderMoved(int) ),
	     this, SLOT( loadLine(int) ) );
}

QSqlTable::~QSqlTable()
{
    delete d;
}

void QSqlTable::addColumn( const QSqlField& field )
{
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	setNumCols( numCols() + 1 );
	colIndex.append( field.fieldNumber() );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, field.displayLabel() );
    }
}

void QSqlTable::removeColumn( uint col )
{
    if ( col >= (uint)numCols() )
	return;
    QHeader* h = horizontalHeader();
    for ( uint i = col; i < (uint)numCols()-1; ++i )
	h->setLabel( i, h->label(i+1) );
    setNumCols( numCols()-1 );
    ColIndex::Iterator it = colIndex.at( col );
    if ( it != colIndex.end() )
	colIndex.remove( it );
}

void QSqlTable::setColumn( uint col, const QSqlField& field )
{
    if ( col >= (uint)numCols() )
	return;
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	colIndex[ col ] = field.fieldNumber();
	QHeader* h = horizontalHeader();
	h->setLabel( col, field.name() );
    } else {
	removeColumn( col );
    }
}

QWidget * QSqlTable::createEditor( int row, int col, bool initFromCell ) const
{
    return 0;
    Q_UNUSED( row );
    Q_UNUSED( col );
    Q_UNUSED( initFromCell );
}

void QSqlTable::reset()
{
    setNumRows(0);
    setNumCols(0);
    haveAllRows = FALSE;
    colIndex.clear();
}

int QSqlTable::indexOf( uint i )
{
    ColIndex::ConstIterator it = colIndex.at( i );
    if ( it != colIndex.end() )
	return *it;
    return -1;
}

void QSqlTable::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

void QSqlTable::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

void QSqlTable::loadNextPage()
{
    if ( haveAllRows )
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
	haveAllRows = TRUE;
    setNumRows( endIdx + 1 );    
}

void QSqlTable::loadNextLine()
{
    if ( haveAllRows )
	return;
    QSql* sql = d->sql();
    if ( !sql )
	return;
    int endIdx = numRows() + 1;
    if ( endIdx < numRows() || endIdx < 0 ) 
	return;
    if ( endIdx < 0 || !sql->seek( endIdx ) )
	return;
    setNumRows( endIdx );
}

void QSqlTable::loadLine( int )
{
    loadNextPage();
}

void QSqlTable::columnClicked ( int col )
{
    QSqlRowset* rset = d->rowset();
    if ( !rset )
	return;
    QSqlIndex lastSort = rset->sort();
    QSqlIndex newSort( lastSort.tableName() );
    newSort.append( rset->field( col ) );
    if ( lastSort.count() == 1 && (lastSort.field(0).name() == newSort.field(0).name()) ) 
	newSort.setDescending( 0, !lastSort.isDescending( 0 ) );
    horizontalHeader()->setSortIndicator( col, !newSort.isDescending( 0 ) );
    rset->select( newSort );
    repaintContents( 0, 0, viewport()->width(), viewport()->height() );
}

void QSqlTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
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

void QSqlTable::setQuery( const QString& query, const QString& databaseName, bool autoPopulate )
{
    QSql s( databaseName );
    s.setQuery( query );
    setQuery( s, autoPopulate );
}

void QSqlTable::setQuery( const QSql& query, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
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

void QSqlTable::setRowset( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlRowset r( name, databaseName );
    setRowset( r, autoPopulate );
}

void QSqlTable::setRowset( const QSqlRowset& rowset, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
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


void QSqlTable::setView( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlView v( name, databaseName );
    setView( v, autoPopulate );
}

void QSqlTable::setView( const QSqlView& view, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
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

#endif

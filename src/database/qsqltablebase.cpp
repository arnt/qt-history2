#include "qsqltablebase.h"

#ifndef QT_NO_SQL

QSqlTableBase::QSqlTableBase ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name), nullTxt("<null>"), haveAllRows(FALSE)
{
    setSelectionMode( NoSelection );
    setVScrollBarMode( AlwaysOn );
    connect( verticalScrollBar(), SIGNAL( nextPage() ),
	     this, SLOT( loadNextPage() ) );
    connect( verticalScrollBar(), SIGNAL( nextLine() ),
	     this, SLOT( loadNextLine() ) );
    connect( verticalScrollBar(), SIGNAL( sliderMoved(int) ),
	     this, SLOT( loadLine(int) ) );
}

QSqlTableBase::~QSqlTableBase()
{
}

void QSqlTableBase::addColumn( const QSqlField& field )
{
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	setNumCols( numCols() + 1 );
	colIndex.append( field.fieldNumber() );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, field.displayLabel() );
    }
}

void QSqlTableBase::removeColumn( uint col )
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

void QSqlTableBase::setColumn( uint col, const QSqlField& field )
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

QWidget * QSqlTableBase::createEditor( int row, int col, bool initFromCell ) const
{
    return 0;
    Q_UNUSED( row );
    Q_UNUSED( col );
    Q_UNUSED( initFromCell );
}

void QSqlTableBase::reset()
{
    setNumRows(0);
    setNumCols(0);
    haveAllRows = FALSE;
    colIndex.clear();
}

int QSqlTableBase::indexOf( uint i )
{
    ColIndex::ConstIterator it = colIndex.at( i );
    if ( it != colIndex.end() )
	return *it;
    return -1;
}

void QSqlTableBase::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

void QSqlTableBase::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

void QSqlTableBase::loadNextPage()
{
    if ( haveAllRows )
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
    while ( endIdx > 0 && !rowExists( endIdx ) )
	endIdx--;
    if ( endIdx != ( startIdx + pageSize + lookAhead ) )
	haveAllRows = TRUE;
    setNumRows( endIdx + 1 );    
}

void QSqlTableBase::loadNextLine()
{
    if ( haveAllRows )
	return;
    int endIdx = numRows() + 1;
    if ( endIdx < numRows() || endIdx < 0 ) 
	return;
    if ( endIdx < 0 || !rowExists( endIdx ) )
	return;
    setNumRows( endIdx );
}

void QSqlTableBase::loadLine( int )
{
    loadNextPage();
}

// void QSqlTableBase::setCurrentCell( int row, int col )
// {
//     loadNextPage();
//     QSqlTableBase::setCurrentCell( row, col );
// }

//////////////

QSqlSortedTable::QSqlSortedTable ( QWidget * parent, const char * name )
    : QSqlTableBase( parent, name )
{
}

QSqlSortedTable::~QSqlSortedTable()
{
    
}

void QSqlSortedTable::columnClicked ( int col )
{
    QSqlIndex lastSort = currentSort();
    QSqlIndex newSort( lastSort.tableName() );
    newSort.append( field( col ) );
    if ( lastSort.count() == 1 && (lastSort.field(0).name() == newSort.field(0).name()) ) 
	newSort.setDescending( 0, !lastSort.isDescending( 0 ) );
    horizontalHeader()->setSortIndicator( col, !newSort.isDescending( 0 ) );
    setSort( newSort );
    repaintContents( 0, 0, viewport()->width(), viewport()->height() );
}

void QSqlSortedTable::setSort( QSqlIndex )
{
    
}

#endif

#include "gridopt.h"
#include "formeditor.h"

#include <qmap.h>
#include <qtl.h>

void DRange::setGeometry( int _left, int _width )
{
  if ( _width >= 0 )
  {
    m_left = _left;
    m_right = _left + _width - 1;
  }
  else
  {
    m_left = _left - _width + 1;
    m_right = _left;
  }
}

bool DRange::intersects( const DRange& _r ) const
{
  if ( !isValid() || !_r.isValid() )
    return FALSE;

  return ( contains( _r ) || _r.contains( *this ) || contains( _r.left() ) || contains( _r.right() ) );
}

bool DRange::contains( const DRange& _r ) const
{
  if ( !isValid() || !_r.isValid() )
    return FALSE;

  return( m_left <= _r.m_left && m_right >= _r.m_right );
}

bool DRange::contains( int point ) const
{
  if ( !isValid() )
    return FALSE;

  return ( m_left <= point && m_right >= point );
}

bool DRange::isLeftEdge( const DRange& _r ) const
{
  return ( _r.m_left == m_left && _r.m_right < m_right );
}

bool DRange::isRightEdge( const DRange& _r ) const
{
  return ( _r.m_left < m_left && m_right + 1 == _r.m_right );
}

bool DRange::isLeftEdge( int point ) const
{
  return ( m_left == point );
}

bool DRange::isRightEdge( int point ) const
{
  return ( m_right + 1 == point );
}

void DRange::swap()
{
  if ( m_left > m_right )
  {
    int tmp = m_left;
    m_left = m_right;
    m_right = tmp;
  }
}

void DRange::alignLeft( int point )
{
  ASSERT( point <= m_right );
  m_left = point;
}

void DRange::alignRight( int point )
{
  m_right = point - 1;
  ASSERT( m_right >= m_left );
}

/*****************************************
 *
 *****************************************/

QValueList<int> dGuessGrid( QValueList<DRange>& _ranges )
{
  QMap<int,bool> known;
  QValueList<int> edges;

  QValueList<DRange>::Iterator it = _ranges.begin();
  for( ; it != _ranges.end(); ++it )
  {
    if ( !known.contains( it->left() ) )
    {
      known[ it->left() ] = TRUE;
      edges.append( it->left() );
    }
    if ( !known.contains( it->left() + it->width() ) )
    {
      known[ it->left() + it->width() ] = TRUE;
      edges.append( it->left() + it->width() );
    }
  }
  qBubbleSort( edges );
  debug("#edges=%i", edges.count() );

  // Debug
  QTextStream out( stdout, IO_WriteOnly );
  QStreamIterator<int> iter( out, "\n" );
  qCopy( edges.begin(), edges.end(), iter );

  // /Debug

  // Iterate over all edges
  QValueList<int>::Iterator it2 = edges.begin();
  while( it2 != edges.end() )
  {
    int l = *it2;
    QValueList<int>::Iterator it3 = it2;
    ++it3;
    if ( it3 != edges.end() )
    {
      int r = *it3;
      // Now we have the range between this egde and the next one
      DRange range( l, r );
	
      // Does this range contain some widgets ?
      bool contains = FALSE;
      QValueList<DRange>::Iterator rit = _ranges.begin();
      for( ; rit != _ranges.end() && !contains; ++rit )
      {
	debug("Testing range %i %i and %i %i", l, r, rit->left(), rit->right() );
	if ( range.contains( *rit ) )
	  contains = TRUE;
      }
      // Can we erase this range ?
      if ( !contains )
      {
	// Make all left edges disappear
	rit = _ranges.begin();
	for( ; rit != _ranges.end() && !contains; ++rit )
	{
	  if ( rit->isLeftEdge( range ) )
	  {
	    debug("Removing left edge");
	    rit->setCoords( range.right(), rit->right() );
	  }
	  else if ( rit->isRightEdge( range.left() ) )
	  {
	    debug("Extending right edge" );
	    rit->setCoords( rit->left(), range.right() - 1 );
	  }
	}
	// Remove the left edge
	debug("-Delete range %i %i", *it2, *it3 );
	it2 = edges.remove( it2 );
      }
      // Dont erase this range since it contains a complete widget
      else
      {
	debug("-skipping range %i %i", *it2, *it3 );
	++it2;
      }
    }
    else
      it2 = it3;
  }

  return edges;
}

DGridLayout* dGuessGrid( QWidget* _parent, QList<QWidget>& _widgets )
{
  QValueList<DRange> ranges;

  // Horizontal
  QListIterator<QWidget> it( _widgets );
  for( ; it.current(); ++it )
  {
    ranges.append( DRange( it.current()->x(), it.current()->x() + it.current()->width() - 1 ) );
  }
  QValueList<int> hedges = dGuessGrid( ranges );

  QValueList<DRange>::Iterator it2 = ranges.begin();
  it.toFirst();
  for( ; it2 != ranges.end(); ++it2, ++it )
    it.current()->setGeometry( it2->left(), it.current()->y(),
			       it2->width(), it.current()->height() );

  debug("#hedges=%i", hedges.count() );

  // Vertical
  ranges.clear();
  it.toFirst();
  for( ; it.current(); ++it )
  {
    ranges.append( DRange( it.current()->y(), it.current()->y() + it.current()->height() - 1 ) );
  }
  QValueList<int> vedges = dGuessGrid( ranges );

  it2 = ranges.begin();
  it.toFirst();
  for( ; it2 != ranges.end(); ++it2, ++it )
    it.current()->setGeometry( it.current()->x(), it2->left(),
			       it.current()->width(), it2->width() );

  debug("#vedges=%i", vedges.count() );

  DGridLayout* grid = new DGridLayout( _parent, vedges.count() - 1, hedges.count() - 1, 6, 6 );
#ifdef GRID_TEST
  // Hack
  return grid;
#endif

  it.toFirst();
  for( ; it.current(); ++it )
  {
    // Find the xposition and colspan
    int x = 0;
    int colspan = 1;
    QValueList<int>::Iterator iit = hedges.begin();
    int i = 0;
    bool done = FALSE;
    while( !done )
    {
      if ( it.current()->x() == *iit )
      {
	x = i;
	++iit;
	while( it.current()->x() + it.current()->width() > *iit )
	{
	  ++colspan;
	  ++iit;
	}
	done = TRUE;
      }
      else
      {
	++i;
	++iit;
      }
    }

    // Find the yposition and rowspan
    int y = 0;
    int rowspan = 1;
    iit = vedges.begin();
    i = 0;
    done = FALSE;
    while( !done )
    {
      if ( it.current()->y() == *iit )
      {
	y = i;
	++iit;
	while( it.current()->y() + it.current()->height() > *iit )
	{
	  ++rowspan;
	  ++iit;
	}
	done = TRUE;
      }
      else
      {
	++i;
	++iit;
      }
    }

    if ( colspan == 1 && rowspan == 1 )
      grid->addWidget2( it.current(), y, x );
    else
      grid->addMultiCellWidget2( it.current(), y, y + rowspan - 1, x, x + colspan - 1 );
  }

  return grid;
}

/********************************************
 *
 * DGridLayout
 *
 ********************************************/

DGridLayout::DGridLayout( QWidget* _parent, int _rows, int _cols, int _outborder, int _innerspace )
  : QGridLayout( _parent, _rows, _cols, _outborder, _innerspace )
{
  for( int i = 0; i < _rows * _cols; ++i )
    m_widgets.append( Widget() );
}

void DGridLayout::addWidget2( QWidget* _widget, int _row, int _col, int _align )
{
  Widget& w = m_widgets[ _row * numCols() + _col ];
  w.w = _widget;

  addWidget( _widget, _row, _col, _align );
}

void DGridLayout::addMultiCellWidget2( QWidget* _widget, int _fromrow, int _torow,
				       int _fromcol, int _tocol, int _align )
{
  Widget& w = m_widgets[ _fromrow * numCols() + _fromcol ];
  w.w = _widget;
  w.multicol = _tocol - _fromcol + 1;
  w.multirow = _torow - _fromrow + 1;
  
  for( int r = _fromrow; r <= _torow; ++r )
    for( int c = _fromcol; c <= _tocol; ++c )
      if ( r != _fromrow || c != _fromcol )
      { 
	Widget& w2 = m_widgets[ r * numCols() + c ];
	w2.w = _widget;
	w2.multicol = w.multicol - ( r - _fromrow );
	w2.multirow = w.multirow - ( c - _fromcol );
	w2.isOverlapped = TRUE;
      }

  addMultiCellWidget( _widget, _fromrow, _torow, _fromcol, _tocol, _align );
}

QWidget* DGridLayout::widget( int _row, int _col )
{
  Widget& w = m_widgets[ _row * numCols() + _col ];
  return w.w;
}

int DGridLayout::multicol( int _row, int _col )
{
  Widget& w = m_widgets[ _row * numCols() + _col ];
  return w.multicol;
}

int DGridLayout::multirow( int _row, int _col )
{
  Widget& w = m_widgets[ _row * numCols() + _col ];
  return w.multirow;
}

void DGridLayout::fillWithForms( DFormEditor* _editor, QWidget* _parent )
{
  for( int r = 0; r < numRows(); ++r )
  {
    for( int c = 0; c < numCols(); ++c )
    {
      Widget& w = m_widgets[ r * numCols() + c ];
      if ( w.w == 0 )
      {
	w.w = new DFormWidget( DFormWidget::LayoutHelper, _editor, _parent );
	addWidget( w.w, r, c );
      }
    }
  }
}

#include "gridopt.h"
#include "formeditor.h"
#include "xml.h"

#include <qmap.h>
#include <qtl.h>
#include <qxmlparser.h>
#include <qresource.h>

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

DGridLayout::DGridLayout( QWidget* _parent, const QResource& _resource )
  : QGridLayout( _parent, _resource )
{
}

DGridLayout::DGridLayout( QWidget* _parent, int _rows, int _cols, int _outborder, int _innerspace )
  : QGridLayout( _parent, _rows, _cols, _outborder, _innerspace ), m_matrix( _rows, _cols )
{
}

DGridLayout::DGridLayout( QWidget* _parent, const Matrix& _m, int _outborder, int _innerspace )
  : QGridLayout( _parent, _m.rows(), _m.cols(), _outborder, _innerspace ), m_matrix( _m )
{
  uint r, c;
  
  for( r = 0; r < _m.rows(); ++r )
    for( c = 0; c < _m.cols(); ++c )
      if ( !_m.cell( r, c ).isOverlapped && _m.cell( r, c ).w != 0 )
      {
	if ( _m.cell( r, c ).multirow > 1 || _m.cell( r, c ).multicol > 1 )
	  addMultiCellWidget( _m.cell( r, c ).w, r, r +  _m.cell( r, c ).multirow - 1,
			      c, c + _m.cell( r, c ).multicol - 1,  _m.cell( r, c ).align );
	else
	  addWidget( _m.cell( r, c ).w, r, c, _m.cell( r, c ).align );
      }

  for( r = 0; r < _m.rows(); ++r )
    setRowStretch( r, _m.row( r ).stretch );

  for( c = 0; c < _m.cols(); ++c )
    setColStretch( r, _m.col( c ).stretch );
}

void DGridLayout::addWidget2( QWidget* _widget, int _row, int _col, int _align )
{
  m_matrix.expand( _row + 1, _col + 1 );

  Cell c;
  c.w = _widget;
  c.align = _align;
  m_matrix.cell( _row, _col ) = c;
  
  addWidget( _widget, _row, _col, _align );
}

void DGridLayout::addMultiCellWidget2( QWidget* _widget, int _fromrow, int _torow,
				       int _fromcol, int _tocol, int _align )
{
  m_matrix.expand( _torow + 1, _tocol + 1 );

  Cell c1;
  c1.w = _widget;
  c1.multicol = _tocol - _fromcol + 1;
  c1.multirow = _torow - _fromrow + 1;
  c1.align = _align;
  m_matrix.cell( _fromrow, _fromcol ) = c1;

  for( int r = _fromrow; r <= _torow; ++r )
    for( int c = _fromcol; c <= _tocol; ++c )
      if ( r != _fromrow || c != _fromcol )
      { 
	Cell c2( c1 );
        c2.multicol = c1.multicol - ( r - _fromrow );
	c2.multirow = c1.multirow - ( c - _fromcol );
	c2.isOverlapped = TRUE;
	m_matrix.cell( _fromrow, _torow ) = c2;
      }

  addMultiCellWidget( _widget, _fromrow, _torow, _fromcol, _tocol, _align );
}


void DGridLayout::fillWithForms( DFormEditor* _editor )
{
  ASSERT( mainWidget() != 0 );
  
  for( uint r = 0; r < m_matrix.rows(); ++r )
  {
    for( uint c = 0; c < m_matrix.cols(); ++c )
    {
      Cell& cell = m_matrix.cell( r, c );
      if ( cell.w == 0 )
      {
	cell.w = new DFormWidget( DFormWidget::GridHelper, _editor, mainWidget() );
	_editor->addWidget( cell.w );
	cell.w->show();
	addWidget( cell.w, r, c );
      }
    }
  }
}

void DGridLayout::updateGeometry()
{
  for( uint r = 0; r < m_matrix.rows(); ++r )
  {
    int top = QCOORD_MAX, bottom = 0;
    for( uint c = 0; c < m_matrix.cols(); ++c )
    {
      if ( m_matrix.cell( r, c ).w &&
	   !m_matrix.cell( r, c ).isOverlapped && m_matrix.cell( r, c ).w->y() < top )
	top = m_matrix.cell( r, c ).w->y();
      if ( m_matrix.cell( r, c ).w && m_matrix.cell( r, c ).multirow == 1 &&
	   m_matrix.cell( r, c ).w->y() + m_matrix.cell( r, c ).w->height() > bottom )
	bottom = m_matrix.cell( r, c ).w->y() + m_matrix.cell( r, c ).w->height();
    }

    m_matrix.row( r ).top = top;
    m_matrix.row( r ).bottom = bottom;
  }

  for( uint c = 0; c < m_matrix.cols(); ++c )
  {
    int left = QCOORD_MAX, right = 0;
    for( uint r = 0; r < m_matrix.rows(); ++r )
    {
      if ( m_matrix.cell( r, c ).w &&
	   !m_matrix.cell( r, c ).isOverlapped && m_matrix.cell( r, c ).w->x() < left )
	left = m_matrix.cell( r, c ).w->x();
      if ( m_matrix.cell( r, c ).w && m_matrix.cell( r, c ).multicol == 1 &&
	   m_matrix.cell( r, c ).w->y() + m_matrix.cell( r, c ).w->width() > right )
	right = m_matrix.cell( r, c ).w->x() + m_matrix.cell( r, c ).w->width();
    }

    m_matrix.col( c ).left = left;
    m_matrix.col( c ).right = right;    
  }
}

DGridLayout::Insert DGridLayout::insertTest( const QPoint& _p, int *_row, int *_col )
{
  for( uint r = 0; r <= m_matrix.rows(); ++r )
  {
    int top, bottom;
    if ( r == 0 )
      top = 0;
    else
      top = m_matrix.row( r - 1 ).bottom;
    if ( r == m_matrix.rows() )
      bottom = mainWidget()->height();
    else
      bottom = m_matrix.row( r ).top;

    if ( _p.y() >= top && _p.y() <= bottom )
    {
      for( uint c = 0; c < m_matrix.cols(); ++c )
	if ( m_matrix.col( c ).left <= _p.x() &&
	     m_matrix.col( c ).right >= _p.x() )
	{
	  *_row = r;
	  *_col = c;
	  return InsertRow;
	}
    }
  }

  for( uint c = 0; c <= m_matrix.cols(); ++c )
  {
    int left, right;
    if ( c == 0 )
      left = 0;
    else
      left = m_matrix.col( c - 1 ).right;
    if ( c == m_matrix.cols() )
      right = mainWidget()->width();
    else
      right = m_matrix.col( c ).left;

    if ( _p.x() >= left && _p.x() <= right )
    {
      for( uint r = 0; r < m_matrix.rows(); ++r )
	if ( m_matrix.row( r ).top <= _p.y() &&
	     m_matrix.row( r ).bottom >= _p.y() )
	{    
	  *_row = r;
	  *_col = c;
	  return InsertCol;
	}
    }
  }
  
  return InsertNone;
}

QRect DGridLayout::insertRect( DGridLayout::Insert ins, uint r, uint c )
{
  if ( ins == InsertRow )
  {    
    uint top, bottom;
    if ( r == 0 )
      top = 0;
    else
      top = m_matrix.row( r - 1 ).bottom;
    if ( r == m_matrix.rows() )
      bottom = mainWidget()->height();
    else
      bottom = m_matrix.row( r ).top;

    uint left = m_matrix.col( c ).left;
    uint right = m_matrix.col( c ).right;

    return QRect( left, top, right - left, bottom - top );
  }

  if ( ins == InsertCol )
  {    
    uint left, right;
    if ( c == 0 )
      left = 0;
    else
      left = m_matrix.col( c - 1 ).right;
    if ( c == m_matrix.cols() )
      right = mainWidget()->width();
    else
      right = m_matrix.col( c ).left;

    uint top = m_matrix.row( r ).top;
    uint bottom = m_matrix.row( r ).bottom;

    return QRect( left, top, right - left, bottom - top );
  }
  
  return QRect();
}

QXMLTag* DGridLayout::save() const
{
  QXMLTag* t = new QXMLTag( "QGridLayout" );

  // TODO borders
  // TODO stretches rows/columns
  // Matrix::ConstIterator it = matrix.begin();
  // for( ; it != matrix.end(); ++it )

  // TODO: handle multicolumn/row
  for( uint y = 0; y < m_matrix.rows(); ++y )
  {
    QXMLTag* r = new QXMLTag( "Row" );
    t->insert( t->end(), r );

    for( uint x = 0; x < m_matrix.cols(); ++x )
    {
      QXMLTag* c = new QXMLTag( "Cell" );
      r->insert( r->end(), c );

      Cell cell = m_matrix.cell( y, x );
      if ( cell.w && cell.w->inherits( "DFormWidget" ) )
      {
	// Save gridhelper widgets as empty Cell tags
	if ( ((DFormWidget*)cell.w)->mode() != DFormWidget::GridHelper )
	  c->insert( ((DFormWidget*)cell.w)->save() );
      }
      else if ( cell.w )
      {
	QXMLTag* w = new QXMLTag( "Widget" );
	c->insert( w );
	w->insert( qObjectToXML( cell.w, TRUE ) );
      }
    }
  }
  
  return t;
}

bool DGridLayout::configure( const QResource& _resource )
{
  QResource irow = _resource.firstChild();
  uint r = 0;

  for( ; irow.isValid(); irow = irow.nextSibling() )
  {
    if ( irow.type() == "Row" )
    {
      QXMLConstIterator t = irow.xmlTree();
      if ( t->hasAttrib( "size" ) )
	addRowSpacing( r, t->intAttrib( "size" ) );
      if ( t->hasAttrib( "stretch" ) )
	setRowStretch( r, t->intAttrib( "stretch" ) );

      QResource icol = irow.firstChild();
      int c = 0;
      while( icol.isValid() )
      {
	if ( icol.type() == "Cell" )
	{
	  debug("QGridLayout child at %i %i", r, c );

	  QXMLConstIterator t = icol.xmlTree();

	  int multicol = 1;
	  int multirow = 1;
	  if ( t->hasAttrib( "multicol" ) )
	    multicol = t->intAttrib( "multicol" );
	  if ( multicol < 1 )
	    return FALSE;
	  if ( t->hasAttrib( "multirow" ) )
	    multirow = t->intAttrib( "multirow" );
	  if ( multirow < 1 )
	    return FALSE;
	  int align = 0;
	  int x,y;
	  if ( stringToAlign( t->attrib( "valign" ), &y ) )
	    align |= y & ( Qt::AlignVCenter | Qt::AlignBottom | Qt::AlignTop );
	  if ( stringToAlign( t->attrib( "halign" ), &x ) )
	    align |= x & ~Qt::AlignVCenter;

	  QResource cell = icol.firstChild();
	  QWidget *w = 0;
	  if ( cell.isValid() && cell.type() == "Widget" )
	  {
	    QResource r( cell.firstChild() );
	    if ( !r.isValid() )
	      return FALSE;
	    w = r.createWidget( mainWidget() );
	    if ( w == 0 )
	      return FALSE;
	    DFormEditor::loadingInstance()->addWidget( w );
	  }
	  else if ( cell.isValid() && cell.type() == "Layout" )
	  {
	    // This should never happen since we replace that construct
	    // with a "Widget" tag in the parse tree.
	    ASSERT( 0 );
	    /* QResource r( cell.firstChild() );
	    if ( !r.isValid() )
	      return FALSE;
	    w = new DFormWidget( DFormWidget::Container, DFormEditor::loadingInstance(), mainWidget() );
	    if ( !r.createLayout( w ) )
	      return FALSE;
	      DFormEditor::loadingInstance()->addWidget( w ); */
	  }
	  // Unknown tag ?
	  else if ( cell.isValid() )
	    return FALSE;

	  // #### QGridLayout does not like empty cells
	  // if ( !w )
	  // w = new QWidget( mainWidget() );

	  if ( w )
	  {
	    if ( multicol != 1 || multirow != 1 )
	      addMultiCellWidget2( w, r, r + multirow - 1, c, c + multicol - 1, align );
	    else
	      addWidget2( w, r, c, align );
	  }

	  if ( t->hasAttrib( "size" ) )
	    addColSpacing( c, t->intAttrib( "size" ) );
	  if ( t->hasAttrib( "stretch" ) )
	  {
	    debug("Setting stretch of col %i to %i",c,t->intAttrib( "stretch" ) );
	    setColStretch( c, t->intAttrib( "stretch" ) );
	  }
	  
	  icol = icol.nextSibling();
	  ++c;
	}
	else
	  return FALSE;
      }
      ++r;
    }
  }

  fillWithForms( DFormEditor::loadingInstance() );

  return TRUE;
}

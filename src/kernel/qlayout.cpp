/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayout.cpp#27 $
**
** Implementation of layout classes
**
** Created : 960416
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qlayout.h"
#include "qmenubar.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qlayout.cpp#27 $");


/*!
  \class QLayout qlayout.h
  \brief The QLayout class is the base class of geometry specifiers.

  This is an abstract base class. Various layout managers inherit
  from this one.

  Geometry management stops when the layout manager is deleted.

  To make a new layout manager, you need to implement the functions
  mainVerticalChain(), mainHorizontalChain() and initGM()
*/


/*!
  Creates a new top-level QLayout with main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder sets the value of defaultBorder(), which
  is interpreted by subclasses.	 If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  Having several top-level layouts for the same widget will cause
  considerable confusion.

*/

QLayout::QLayout( QWidget *parent, int border, int autoBorder, const char *name )
    : QObject( parent, name )
{
    topLevel	 = TRUE;
    bm		 = new QGManager( parent, name );
    parent->removeChild( bm );
    insertChild( bm );

    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    bm->setBorder( border );
}

/*!
  \fn const char *QLayout::name() const

  Returns the internal object name.
*/


/*!
  Returns the main widget of this layout, or 0 if this layout is
  a sub-layout which is not yet inserted.
*/

QWidget * QLayout::mainWidget()
{
    return bm ? bm->mainWidget() : 0;
}


/*!
  Constructs a new child QLayout,
  If \a autoBorder is -1, this QLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.
*/

QLayout::QLayout( int autoBorder, const char *name )
    : QObject( 0, name )
{
    topLevel	 = FALSE;
    bm		 = 0;
    defBorder	 = autoBorder;
}


/*!
  Deletes all children layouts. Geometry management stops when
  a toplevel layout is deleted.
  \internal
  The layout classes will probably be fatally confused if you delete
  a sublayout
*/

QLayout::~QLayout()
{
}


/*!
  This function is called from addLayout functions in subclasses,
  to add \a l layout as a sublayout.
*/

void QLayout::addChildLayout( QLayout *l )
{
    if ( l->topLevel ) {
#if defined(CHECK_NULL)
	warning( "QLayout: Attempt to add top-level layout as child" );
#endif
	return;
    }
    l->bm = bm;
    insertChild( l );
    if ( l->defBorder < 0 )
	l->defBorder = defBorder;
    l->initGM();
}

/*!
  \fn void QLayout::initGM()

  Implement this function to do what's necessary to initialize chains,
  once the layout has a basicManager().
*/

/*!
  \fn QGManager *QLayout::basicManager()

  Returns the QGManager for this layout. Returns 0 if
  this is a child layout which has not been inserted yet.
*/


/*!
  \fn QChain *QLayout::mainVerticalChain()
  Implement this function to return the main vertical chain.
*/

/*!
  \fn QChain *QLayout::mainHorizontalChain()
  Implement this function to return the main horizontal chain.
*/

/*!
  \fn QChain *QLayout::horChain( QLayout * )
  This function works around a dubious feature in
  the C++ language definition, to provide access to mainHorizontalChain().
 */


/*!
  \fn QChain *QLayout::verChain( QLayout * )
  This function works around a dubious feature in
  the C++ language definition, to provide access to mainVerticalChain().
*/


/*!
  \fn int QLayout::defaultBorder() const
  Returns the default border for the geometry manager.
*/

/*!
  Starts geometry management - analogous to show() for widgets.
  This function should only be called for top level layouts.
*/

bool QLayout::activate()
{
    if ( topLevel )
	return bm->activate();
#if defined(DEBUG)
    warning("QLayout::activate() for child layout");
#endif
    return FALSE;
}

/*!
  \overload void QLayout::freeze()

  This version of the method fixes the main widget at its minimum size.
  You can also achieve this with freeze( 0, 0 );
*/


/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  A frozen layout cannot be unfrozen, the only sensible thing to do
  is to delete it.

  The size is adjusted to a valid value. Thus freeze(0,0) fixes the
  widget to its minimum size.
*/

void QLayout::freeze( int w, int h )
{
    if ( !topLevel ) {
#if defined(CHECK_STATE)
	warning( "QLayout::freeze: Only top-level QLayout can be frozen" );
#endif
	return;
    }
#if defined(CHECK_NULL)
    ASSERT( bm != 0 );
#endif
    bm->freeze( w, h );
    delete bm;
    bm = 0;
}


/*!
  Makes the geometry manager take account of the menu bar \a w. All
  child widgets are placed below the bottom edge of the menu bar.

  A menu bar does its own geometry managing, never do addWidget()
  on a menu bar.
*/

void QLayout::setMenuBar( QMenuBar *w )
{
    if ( !topLevel ) {
#if defined(CHECK_NULL)
	warning( "QLayout::setMenuBar: Called for sub layout" );
#endif
	return;
    }
    ASSERT( bm );
    bm->setMenuBar( w );
}


/*!
  \class QBoxLayout qlayout.h
  \brief The QBoxLayout class specifies child widget geometry.

  Contents are arranged serially, either horizontal or vertical.
  The contents fill the available space.

  A QBoxLayout can contain widgets or other layouts.

  QHBoxLayout and QVBoxLayout are convenience classes.
 */

static inline bool horz( QGManager::Direction dir )
{
    return dir == QGManager::RightToLeft || dir == QGManager::LeftToRight;
}

static inline QGManager::Direction perp( QGManager::Direction dir )
{
    if ( horz( dir ))
	return QGManager::Down;
    else
	return QGManager::LeftToRight;
}

/*!
  Creates a new QBoxLayout with direction \a d and main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between adjacent managed children.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  \sa direction()
*/

QBoxLayout::QBoxLayout( QWidget *parent, Direction d,
			int border, int autoBorder, const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    pristine = TRUE;
    dir = (QGManager::Direction)d;

    serChain = basicManager()->newSerChain( dir );
    if ( horz( dir )  ) {
	basicManager()->add( basicManager()->xChain(), serChain );
	parChain = basicManager()->yChain();
    } else {
	basicManager()->add( basicManager()->yChain(), serChain );
	parChain = basicManager()->xChain();
    }
}

/*!
  If \a autoBorder is -1, this QBoxLayout will inherit its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this box into another layout before using it.
*/

QBoxLayout::QBoxLayout( Direction d,
			int autoBorder, const char *name )
    : QLayout( autoBorder, name )
{
    pristine = TRUE;
    dir = (QGManager::Direction)d;
    parChain = 0; // debug
    serChain = 0; // debug
}


/*!
  Destroys this box. 
*/

QBoxLayout::~QBoxLayout()
{
}

/*!
  Initializes this box.
*/

void QBoxLayout::initGM()
{
    serChain = basicManager()->newSerChain( dir );
    parChain = basicManager()->newParChain( perp( dir ) );
}


/*!
  Adds \a layout to the box, with serial stretch factor \a stretch.

  \sa addWidget(), addSpacing()
*/

void QBoxLayout::addLayout( QLayout *layout, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addLayout: Box must have a widget parent or be\n"
		 "                       added in another layout before use" );
#endif
	return;
    }
    addChildLayout( layout );
    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0,
				    defaultBorder() );
    addB( layout, stretch );
    pristine = FALSE;
}

void QBoxLayout::addB( QLayout * l, int stretch )
{
    if ( horz( dir ) ) {
	basicManager()->QGManager::add( parChain, verChain( l ) );
	basicManager()->QGManager::add( serChain, horChain( l ), stretch );
    } else {
	basicManager()->QGManager::add( parChain, horChain( l ) );
	basicManager()->QGManager::add( serChain, verChain( l ),
					    stretch );
    }
}


/*!
  Returns the main vertical chain, so that a box can be put into
  other boxes (or other types of QLayout).
*/

QChain * QBoxLayout::mainVerticalChain()
{
    if ( horz(dir) )
	return parChain;
    else
	return serChain;
}

/*!
  Returns the main horizontal chain, so that a box can be put into
  other boxes (or other types of QLayout).
*/

QChain * QBoxLayout::mainHorizontalChain()
{
    if ( horz(dir) )
	return serChain;
    else
	return parChain;
}

/*!
  Adds a non-stretchable space with size \a size.  QBoxLayout gives
  default border and spacing. This function adds additional space.

  \sa addStretch
*/
void QBoxLayout::addSpacing( int size )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning("QBoxLayout::addSpacing: Box must have a widget parent or be\n"
		"                        added in another layout before use.");
#endif
	return;
    }
    basicManager()->addSpacing( serChain, size, 0, size );
}

/*!
  Adds a stretchable space with zero minimum size
  and stretch factor \a stretch.

  \sa addSpacing
*/
//###... Should perhaps replace default space?
void QBoxLayout::addStretch( int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning("QBoxLayout::addStretch: Box must have a widget parent or be\n"
		 "                       added in another layout before use.");
#endif
	return;
    }
    basicManager()->addSpacing( serChain, 0, stretch );
}

/*!
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \a size. Other constraints may
  increase the limit.

  \sa addMaxStrut()
*/

void QBoxLayout::addStrut( int size )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addStrut: Box must have a widget parent or be\n"
		 "                      added in another layout before use." );
#endif
	return;
    }
    basicManager()->addSpacing( parChain, size );
}

/*
  Limits the perpendicular dimension of the box (e.g. height if
  the box is LeftToRight) to a maximum of \a size. Other constraints
  may decrease the limit.

  \sa addMinStrut()

void QBoxLayout::addMaxStrut( int size)
{
    gm->QGManager::addSpacing( parChain, 0, 0, size );
}
*/

/*!
  Adds \a widget to the box, with stretch factor \a stretch and
  alignment \a align.

  The stretch factor applies only in the \link direction() direction
  \endlink of the QBoxLayout, and is relative to the other boxes and
  widgets in this QBoxLayout.  Widgets and boxes with higher stretch
  factor grow more.

  If the stretch factor is 0 and nothing else in the QBoxLayout can
  grow at all, the widget may still grow up to its \link
  QWidget::setMaximumSize() maximum size. \endlink

  Alignment is perpendicular to direction(), alignment in the
  serial direction is done with addStretch().

  For horizontal boxes,	 the possible alignments are
  <ul>
  <li> \c AlignCenter centers vertically in the box.
  <li> \c AlignTop aligns to the top border of the box.
  <li> \c AlignBottom aligns to the bottom border of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c AlignCenter centers horizontally in the box.
  <li> \c AlignLeft aligns to the left border of the box.
  <li> \c AlignRight aligns to the right border of the box.
  </ul>

  Alignment only has effect if the size of the box is greater than the
  widget's maximum size.

  \sa addLayout(), addSpacing()
*/

#if QT_VERSION == 200
#error "Binary compatibility."
#endif


void QBoxLayout::addWidget( QWidget *widget, int stretch, int align )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addLayout: Box must have a widget parent or be\n"
		 "                       added in another layout before use.");
#endif
	return;
    }

    if ( !widget ) {
#if defined(CHECK_NULL)
	warning( "QBoxLayout::addWidget: Widget can't be null" );
#endif
	return;
    }

    const int first = AlignLeft | AlignTop;
    const int last  = AlignRight | AlignBottom;

    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0,
				    defaultBorder() );

    if ( 0/*a == alignBoth*/ ) {
	basicManager()->addWidget( parChain, widget, 0 );
    } else {
	QGManager::Direction d = perp( dir );
	QChain *sc = basicManager()->newSerChain( d );
	if ( align & last || align & AlignCenter ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->addWidget( sc, widget, 1 );
	if ( align & AlignCenter || align & first ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->add( parChain, sc );
    }
    basicManager()->addWidget( serChain, widget, stretch );
    pristine = FALSE;
}


/*!
  \fn QBoxLayout::Direction QBoxLayout::direction() const

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBoxLayout::addWidget Alignment \endlink
  works perpendicular to this direction.

  The directions are \c LeftToRight, \c RightToLeft, \c TopToBottom
  and \c BottomToTop. For the last two, the shorter aliases \c Down and
  \c Up are also available.

  \sa addWidget(), addBox(), addSpacing()
*/




/*!
  \class QHBoxLayout qlayout.h
  \brief The QHBoxLayout class provides a horizontal layout box

  The contents are arranged left to right, they will stretch to fill
  the available space. 
*/


/*!
  Creates a new top-level horizontal box.
 */
QHBoxLayout::QHBoxLayout( QWidget *parent, int border,
		int autoBorder, const char *name )
    :QBoxLayout( parent, LeftToRight, border, autoBorder, name )
{
    
}

/*!
  Creates a new horizontal box. You have to add it to another 
  layout before using it.
 */
QHBoxLayout::QHBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( LeftToRight, autoBorder, name )
{
}


/*!
  Destroys this box. 
*/

QHBoxLayout::~QHBoxLayout()
{
}



/*!
  \class QVBoxLayout qlayout.h
  \brief The QVBoxLayout class provides a vertical layout box

  The contents are arranged top to bottom, they will stretch to fill
  the available space.
*/

/*!
  Creates a new top-level vertical box.
 */
QVBoxLayout::QVBoxLayout( QWidget *parent, int border,
		int autoBorder, const char *name )
    :QBoxLayout( parent, TopToBottom, border, autoBorder, name )
{
    
}

/*!
  Creates a new vertical box. You have to add it to another 
  layout before using it.
 */
QVBoxLayout::QVBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( TopToBottom, autoBorder, name )
{
}

/*!
  Destroys this box. 
*/

QVBoxLayout::~QVBoxLayout()
{
}

/*!
  \class QGridLayout qlayout.h

  \brief The QGridLayout class specifies child widget geometry.

  Contents are arranged in a grid. If you need a more flexible layout,
  see the QHBoxLayout and QVBoxLayout classes.

  To avoid squashed widgets, each column and each row of the grid
  should have a a minimum size or a nonzero stretch factor. Stretch
  factors are set with setRowStretch() and setColStretch(). Minimum sizes
  are set by addColSpacing(), addRowSpacing() and by the minimum sizes
  of the widgets added. 

  Note that a widget which spans several rows or columns does not
  influence the minimum size of any of the rows/columns it spans.
*/


/*!
  Constructs a new QGridLayout with \a nRows rows, \a nCols columns
   and main widget \a  parent.	\a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between cells.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name.
*/

QGridLayout::QGridLayout( QWidget *parent, int nRows, int nCols, int border ,
			  int autoBorder , const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    horChain = basicManager()->newSerChain( QGManager::LeftToRight );
    verChain = basicManager()->newSerChain( QGManager::Down );
    basicManager()->add( basicManager()->xChain(), horChain );
    basicManager()->add( basicManager()->yChain(), verChain );
    init( nRows, nCols );
}


/*!
  Constructs a new grid with \a nRows rows and \a  nCols columns,
  If \a autoBorder is -1, this QGridLayout will inherits its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this grid into another layout before using it.
*/

QGridLayout::QGridLayout( int nRows, int nCols,
			  int autoBorder, const char *name )
     : QLayout( autoBorder, name )
{
    rr = nRows;
    cc = nCols;
}


/*!
  Deletes this grid. Geometry management is terminated if
  this is a top-level grid.
*/

QGridLayout::~QGridLayout()
{
    delete rows;
    delete cols;
}


/*!
  Initializes this grid.
*/

void QGridLayout::initGM()
{
    horChain = basicManager()->newSerChain( QGManager::LeftToRight );
    verChain = basicManager()->newSerChain( QGManager::Down );
    init( rr, cc );
}


/*!
  Sets up the table and other internal stuff
*/

void QGridLayout::init( int nRows, int nCols )
{
    rows = new QArray<QChain*> ( nRows );
    cols = new QArray<QChain*> ( nCols );

    int i;
    for ( i = 0; i< nRows; i++ ) {
	if ( i != 0 )
	    basicManager()->addSpacing( verChain, defaultBorder(), 0,
					defaultBorder() );
	(*rows)[i] = basicManager()->newParChain( QGManager::Down );
	basicManager()->add( verChain, (*rows)[i] );
    }

    for ( i = 0; i< nCols; i++ ) {
	if ( i != 0 )
	    basicManager()->addSpacing( horChain, defaultBorder(), 0,
					defaultBorder() );
	(*cols)[i] = basicManager()->newParChain( QGManager::LeftToRight );
	basicManager()->add( horChain, (*cols)[i] );
    }
}


/*!
  Adds the widget \a w to the cell grid at \a row, \a col.
  The top left position is (0,0)

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment().  Note that widgets take all the space they
  can get; alignment has no effect unless you have set
  QWidget::maximumSize().  

*/

void QGridLayout::addWidget( QWidget *w, int row, int col, int align )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
       warning("QGridLayout::addWidget: Grid must have a widget parent or be\n"
	       "                        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addWidget: Zero sized grid" );
#endif
	return;
    }

    addMultiCellWidget( w, row, row, col, col, align );
}

/*!
  Adds the widget \a w to the cell grid, spanning multiple rows/columns.

  Note that multicell widgets do not influence the minimum or maximum
  size of columns/rows they span. Use addColSpacing() or addRowSpacing()
  to set minimum sizes explicitly.

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment(), alignment has no effect unless you have set
  QWidget::maximumSize().  
*/

void QGridLayout::addMultiCellWidget( QWidget *w, int fromRow, int toRow,
				      int fromCol, int toCol, int align	 )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::addMultiCellWidget: "
		 "Grid must have a widget parent or be\n"
		 "        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addMultiCellWidget: Zero sized grid" );
#endif
	return;
    }
    const int hFlags = AlignHCenter | AlignLeft | AlignRight;
    const int vFlags = AlignVCenter | AlignTop | AlignBottom;

    int a = align & hFlags;

    QChain *c;
    if ( a || fromCol != toCol ) {
	c = basicManager()->newSerChain( QGManager::LeftToRight );
	if ( fromCol == toCol )
	    basicManager()->add( (*cols)[ fromCol ], c );
	else
	    basicManager()->addBranch( horChain, c, fromCol*2, toCol*2 );
    } else {
	c =  (*cols)[ fromCol ];
    }
    if ( a & (AlignHCenter|AlignRight) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w, 1 ); //stretch ignored in parallel chain
    if ( a & (AlignHCenter|AlignLeft) )
	basicManager()->addSpacing( c, 0 );

    // vertical dimension:
    a = align & vFlags;
    if ( a || fromRow != toRow ) {
	c = basicManager()->newSerChain( QGManager::Down );
	if ( fromRow == toRow )
	    basicManager()->add( (*rows)[ fromRow ], c );
	else
	    basicManager()->addBranch( verChain, c, fromRow*2, toRow*2 );
    } else {
	c =  (*rows)[ fromRow ];
    }
    if ( a & (AlignVCenter|AlignBottom) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w, 1 ); //stretch ignored in parallel chain
    if ( a & (AlignVCenter|AlignTop) )
	basicManager()->addSpacing( c, 0 );
}


/*!
  Places another layout at position (\a row, \a col) in the grid.
  The top left position is (0,0)
*/

void QGridLayout::addLayout( QLayout *layout, int row, int col)
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
       warning("QGridLayout::addLayout: Grid must have a widget parent or be\n"
	       "                        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addLayout: Zero sized grid" );
#endif
	return;
    }
    addChildLayout( layout );
    QChain *c =	 (*cols)[ col ];
    basicManager()->add( c, QLayout::horChain( layout ) );
    c =	 (*rows)[ row ];
    basicManager()->add( c, QLayout::verChain( layout ) );
}


/*!
  Sets the stretch factor of row \a row to \a stretch.
  The first row is number 0.

  The stretch factor  is relative to the other rows in this grid.
  Rows with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other row in this table can
  grow at all, the row may still grow.
*/

void QGridLayout::setRowStretch( int row, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setRowStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( rows->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setRowStretch: Zero sized grid" );
#endif
	return;
    }

    QChain *c =	 (*rows)[ row ];
    basicManager()->setStretch( c, stretch );
}


/*!
  Sets the stretch factor of column \a col to \a stretch.
  The first column is number 0.

  The stretch factor  is relative to the other columns in this grid.
  Columns with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other column in this table can
  grow at all, the column may still grow.
*/

void QGridLayout::setColStretch( int col, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( cols->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setColStretch: Zero sized grid" );
#endif
	return;
    }
    QChain *c =	 (*cols)[ col ];
    basicManager()->setStretch( c, stretch );
}


/*!
  Sets the minimum width of \a row to \a minsize pixels.
 */
void QGridLayout::addRowSpacing( int row, int minsize )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( rows->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addRowSpacing: Zero sized grid" );
#endif
	return;
    }

    QChain *c =	 (*rows)[ row ];
    basicManager()->addSpacing( c, minsize );
}

/*!
  Sets the minimum height of \a col to \a minsize pixels.
 */
void QGridLayout::addColSpacing( int col, int minsize )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( cols->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setColStretch: Zero sized grid" );
#endif
	return;
    }
    QChain *c =	 (*cols)[ col ];
    basicManager()->addSpacing( c, minsize );
}

/*!
  \fn QChain *QGridLayout::mainVerticalChain()
  This function returns the main vertical chain.
*/

/*!
  \fn QChain *QGridLayout::mainHorizontalChain()
  This function returns the main horizontal chain.
*/

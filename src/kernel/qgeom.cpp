/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.cpp#17 $
**
**  Geometry Management
**
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/
#include "qgeom.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qgeom.cpp#17 $");



/*!
  \class QLayout qgeom.h
  \brief QLayout is the base class of geometry specifiers.

*/



/*!
  Creates a new QLayout with main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder sets the value of defaultBorder(), which
  is interpreted by subclasses.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  \sa direction()
*/

QLayout::QLayout( QWidget *parent, int border, int autoBorder, const char *name )
    : QObject( parent, name )
{
    topLevel = TRUE;
    bm = new QBasicManager( parent, name );

    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    bm->setBorder( border );
}



/*!
  Constructs a new QLayout, within another QLayout \a parent.
  If \a autoBorder is -1, this QLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.
*/
QLayout::QLayout( QLayout *parent, int autoBorder, const char *name )
    : QObject( parent, name )
{
    topLevel = FALSE;
    bm = parent->bm;
    if ( autoBorder == -1 )
	defBorder =  parent->defBorder;
    else
	defBorder = autoBorder;
}



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
  This function works around a terrible case of brain damage in 
  the C++ language definition, to provide access to mainHorizontalChain().
 */


/*!
  \fn QChain *QLayout::verChain( QLayout * )
  This function works around a terrible case of brain damage in 
  the C++ language definition, to provide access to mainVerticalChain().
 */


/*!
  \fn int QLayout::defaultBorder() const
  Returns the default border for the geometry manager.
*/

/*!
  \fn bool QBoxLayout::doIt()

  Starts geometry management - equivalent to show() for widgets.
*/


/*!
  \overload void QLayout::freeze()

  This version of the method fixes the main widget at its minimum size.
  You can also achieve this with freeze( 0, 0 );
*/


/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets, for widgets which should not be
  resizable but where a QLayout subclass is used to set up the initial
  geometry.

  The size is adjusted to a valid value. Thus freeze(0,0) fixes the
  widget to its minimum size.  
*/

void QLayout::freeze( int w, int h )
{
    if ( !topLevel ) {
	warning( "Only top-level QBoxLayout can be frozen." );
	return;
    }
    bm->freeze( w, h );
    delete bm;
    bm = 0;
}

/*!
  \class QBoxLayout qgeom.h
  \brief The QBoxLayout class specifies child widget geometry.

  Contents are arranged serially, either horizontal or vertical.
  The contents fill the available space.

  A QBoxLayout (box for short) can contain widgets or other
  boxes.
*/

static inline bool horz( QBasicManager::Direction dir )
{
    return dir == QBasicManager::RightToLeft || dir == QBasicManager::LeftToRight;
}

static inline QBasicManager::Direction perp( QBasicManager::Direction dir )
{
    if ( horz( dir ))
	return QBasicManager::Down;
    else
	return QBasicManager::LeftToRight;
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

QBoxLayout::QBoxLayout( QWidget *parent, QBasicManager::Direction d,
			int border, int autoBorder, const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    pristine = TRUE;
    dir = d;

    serChain = basicManager()->newSerChain( d );
    if ( horz( d )  ) {
	basicManager()->add( basicManager()->xChain(), serChain );
	parChain = basicManager()->yChain();
    } else {
	basicManager()->add( basicManager()->yChain(), serChain );
	parChain = basicManager()->xChain();
    }
}

/*!
  Constructs a new box with direction \a d, within \a parent.
  If \a autoBorder is -1, this QBoxLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You then have to insert it into \a parent.
*/
QBoxLayout::QBoxLayout(	 QLayout *parent, QBasicManager::Direction d,
			 int autoBorder, const char *name )
    : QLayout( parent, autoBorder, name )
{
    pristine = TRUE;
    dir = d;
    serChain = basicManager()->newSerChain( d );
    // parChain is perpendicular to serChain
    parChain = basicManager()->newParChain( perp( d ) );
}



/*!
  Adds \a layout to the box, with serial stretch factor \a stretch.

  \warning The 

  \sa addWidget(), addSpacing()
*/
void QBoxLayout::addLayout( QLayout *layout, int stretch )
{
    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0, defaultBorder() );

    addB( layout, stretch );
    pristine = FALSE;
}

void QBoxLayout::addB( QLayout * l, int stretch )
{
    if ( horz( dir ) ) {
	basicManager()->QBasicManager::add( parChain, verChain( l ) );
	basicManager()->QBasicManager::add( serChain, horChain( l ),
					    stretch );
    } else {
	basicManager()->QBasicManager::add( parChain, horChain( l ) );
	basicManager()->QBasicManager::add( serChain, verChain( l ),
					    stretch );
    }
}


/*!
  Returns the main vertical chain, so that a box can be put into
  other boxes (or other types of QLayout.
*/

QChain * QBoxLayout::mainVerticalChain()
{
    if ( horz(direction()) )
	return parChain;
    else
	return serChain;
}

/*!
  Returns the main horizontal chain, so that a box can be put into
  other boxes (or other types of QLayout.
*/

QChain * QBoxLayout::mainHorizontalChain()
{
    if ( horz(direction()) )
	return serChain;
    else
	return parChain;
}

/*!
  Adds a non-stretchable space with size \a size.  QBoxLayout gives
  default border and spacing. This function adds additional space.

  \sa addStretch
*/
//###... Should perhaps replace default space?
void QBoxLayout::addSpacing( int size )
{
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
    basicManager()->addSpacing( parChain, size, 0, 0 );
}

/*
  Limits the perpendicular dimension of the box (e.g. height if
  the box is LeftToRight) to a maximum of \a size. Other constraints
  may decrease the limit.

  \sa addMinStrut()

void QBoxLayout::addMaxStrut( int size)
{
    gm->QBasicManager::addSpacing( parChain, 0, 0, size );
}
*/

/*!
  Adds \a widget to the box, with stretch factor \a stretch and
  alignment \a a.

  The stretch factor applies only in the \link direction() direction
  \endlink of the QBoxLayout, and is relative to the other boxes and
  widgets in this QBoxLayout.  Widgets and boxes with higher stretch
  factor grow more.

  If the stretch factor is 0 and nothing else in the QBoxLayout can
  grow at all, the widget may still grow up to its \link
  QWidget::setMaximumSize() maximum size. \endlink

  Alignment is perpendicular to direction(), alignment in the
  serial direction is done with addSpacing().

  For horizontal boxes,	 the possible alignments are
  <ul>
  <li> \c alignCenter centers vertically in the box.
  <li> \c alignTop aligns to the top border of the box.
  <li> \c alignBottom aligns to the bottom border of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c alignCenter centers horizontally in the box.
  <li> \c alignLeft aligns to the left border of the box.
  <li> \c alignRight aligns to the right border of the box.
  </ul>

  Alignment only has effect if the size of the box is greater than the
  widget's maximum size. 

  \sa addLayout(), addSpacing()
*/

void QBoxLayout::addWidget( QWidget *widget, int stretch, alignment a )
{

#if defined(DEBUG)
    if ( !widget ) {
	warning( "QBoxLayout::addWidget: widget == 0" );
	return;
    }

    QObject * ancestor = this;
    while ( ancestor && !ancestor->isWidgetType() )
	ancestor = ((QBoxLayout*)ancestor)->parent();
    if ( !ancestor || !ancestor->isWidgetType() ) {
	warning( "QBoxLayout::addWidget: no widget ancestor found" );
	return;
    }

    if ( (QWidget *)ancestor != widget->parentWidget() ) {
	warning( "QBoxLayout::addWidget: %s(%s) is not a child of %s(%s)",
		 widget->name(), widget->className(),
		 ancestor->name(), ancestor->className() );
	return;
    }
#endif

    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0, defaultBorder() );

    if ( 0/*a == alignBoth*/ ) {
	basicManager()->addWidget( parChain, widget, 0 );
    } else {
	QBasicManager::Direction d = perp( direction() );
	QChain *sc = basicManager()->newSerChain( d );
	if ( a == alignCenter || a == alignBottom ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->addWidget( sc, widget, 1 );
	if ( a == alignCenter ||  a == alignTop ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->add( parChain, sc );
    }
    basicManager()->addWidget( serChain, widget, stretch );
    pristine = FALSE;
}

/*!
  \fn QBasicManager::Direction QBoxLayout::direction() const

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBoxLayout::addWidget Alignment \endlink
  works perpendicular to this direction.
  \sa addWidget(), addBox(), addSpacing()
*/




/*!
  \class QGridLayout qgeom.h

  \brief The QGridLayout class specifies child widget geometry.

  Contents are arranged in a fixed grid. If you need a more flexible layout,
  see the QBoxLayout class.
*/

/*!
  Constructs a new QGridLayout with \a nRows, nCols columns
   and main widget \a  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between adjacent managed children.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name.
*/

QGridLayout::QGridLayout( QWidget *parent, int nRows, int nCols, int border ,
			  int autoBorder , const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    horChain = basicManager()->newSerChain( QBasicManager::LeftToRight );
    verChain = basicManager()->newSerChain( QBasicManager::Down );
    basicManager()->add( basicManager()->xChain(), horChain );
    basicManager()->add( basicManager()->yChain(), verChain );
    init( nRows, nCols );
}



/*!
  Constructs a new grid with direction \a d, within \a parent.
  If \a autoBorder is -1, this QGridLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You then have to insert it into \a parent.
*/
QGridLayout::QGridLayout( QLayout *parent, int nRows, int nCols, 
			   int autoBorder, const char *name )
     : QLayout( parent, autoBorder, name )
{
    horChain = basicManager()->newSerChain( QBasicManager::LeftToRight );
    verChain = basicManager()->newSerChain( QBasicManager::Down );
    init( nRows, nCols );
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
	(*rows)[i] = basicManager()->newParChain( QBasicManager::Down );
	basicManager()->add( verChain, (*rows)[i] );
    }

    for ( i = 0; i< nCols; i++ ) {
	if ( i != 0 )
	    basicManager()->addSpacing( horChain, defaultBorder(), 0, 
					defaultBorder() );
	(*cols)[i] = basicManager()->newParChain( QBasicManager::LeftToRight );
	basicManager()->add( horChain, (*cols)[i] );
    }
    
}


/*!

  Adds the widget \a w to the cell grid at \a row, \a col. Alignment is
  specified by \a align which takes the same arguments as QLabel::setAlignment().

  Note that widgets take all the space they can get; alignment has no effect unless
  you have set QWidget::maximumSize().

*/
void QGridLayout::addWidget( QWidget *w, int row, int col, int align )
{
    addMultiCellWidget( w, row, row, col, col, align );
}

/*!

  Adds the widget \a w to the cell grid, spanning multiple rows/columns.

  Note that multicell widgets do not define the columns/rows they
  span.  Each column must contain at least one widget that does not
  span multiple columns. Likewise, each row must contain one widget
  that does not span multiple rows. If your layout does not satisfy this,
  consider using the QBoxLayout class instead.

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment(), alignment has no effect unless you have set
  QWidget::maximumSize().

*/
void QGridLayout::addMultiCellWidget( QWidget *w, int fromRow, int toRow, 
					int fromCol, int toCol, int align  )
{
    const int hFlags = AlignHCenter | AlignLeft | AlignRight;
    const int vFlags = AlignVCenter | AlignTop | AlignBottom;

    int a = align & hFlags;

    QChain *c;
    if ( a || fromCol != toCol ) {
	c = basicManager()->newSerChain( QBasicManager::LeftToRight );
	if ( fromCol == toCol )
	    basicManager()->add( (*cols)[ fromCol ], c );
	else
	    basicManager()->addBranch( horChain, c, fromCol*2, toCol*2 );
    }
    else
	c =  (*cols)[ fromCol ];

    if ( a & (AlignHCenter|AlignLeft) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w );
    if ( a & (AlignHCenter|AlignRight) )
	basicManager()->addSpacing( c, 0 );

    // vertical dimension:
    a = align & vFlags;
    if ( a || fromRow != toRow ) {
	c = basicManager()->newSerChain( QBasicManager::Down );
	if ( fromRow == toRow )
	    basicManager()->add( (*rows)[ fromRow ], c );
	else
	    basicManager()->addBranch( verChain, c, fromRow*2, toRow*2 );
    }
    else
	c =  (*rows)[ fromRow ];

    if ( a & (AlignVCenter|AlignTop) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w );
    if ( a & (AlignVCenter|AlignBottom) )
	basicManager()->addSpacing( c, 0 );
}



/*!
  Places another layout at position (\a row, \a col) in the grid.
*/

void QGridLayout::addLayout( QLayout *layout, int row, int col)
{
    QChain *c =  (*cols)[ col ];
    basicManager()->add( c, QLayout::horChain( layout ) );

    c =  (*rows)[ row ];
    basicManager()->add( c, QLayout::verChain( layout ) );
}



/*!
  Sets the stretch factor of row \a row to \a stretch.

  The stretch factor  is relative to the other rows in this grid.
  Rows with higher stretch factor take more of the available space.

  If the stretch factor is 0 and no other row in this table can
  grow at all, the row may still grow.

*/

void QGridLayout::setRowStretch( int row, int stretch )
{
    QChain *c =  (*rows)[ row ];
    basicManager()->setStretch( c, stretch );
}


/*!
  Sets the stretch factor of column \a col to \a stretch.

  The stretch factor  is relative to the other columns in this grid.
  Columns with higher stretch factor take more of the available space.

  If the stretch factor is 0 and no other column in this table can
  grow at all, the column may still grow.

*/

void QGridLayout::setColStretch( int col, int stretch )
{
    QChain *c =  (*cols)[ col ];
    basicManager()->setStretch( c, stretch );
}

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.cpp#16 $
**
**  Geometry Management
**
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/
#include "qgeom.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qgeom.cpp#16 $");



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
    : QObject( parent, name )
{
    topLevel = TRUE;
    bm = new QBasicManager( parent, name );
    pristine = TRUE;
    dir = d;

    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    bm->setBorder( border );

    serChain = bm->newSerChain( d );
    if ( horz( d )  ) {
	bm->add( bm->xChain(), serChain );
	parChain = bm->yChain();
    } else {
	bm->add( bm->yChain(), serChain );
	parChain = bm->xChain();
    }
}



/*!
  \fn int QBoxLayout::defaultBorder() const
  Returns the default border for the geometry manager.
*/

/*!
  \fn bool QBoxLayout::doIt()

  Starts geometry management - equivalent to show() for widgets.
*/


/*!
  \overload void QBoxLayout::freeze()

  This version of the method fixes the widget at its minimum size.
  You can also achieve this with freeze( 0, 0 );
*/


/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets, for widgets which should not be
  resizable but where QBoxLayout is used to set up the initial geometry.

  The size is adjusted to a valid value. Thus freeze(0,0) fixes the
  widget to its minimum size.
*/

void QBoxLayout::freeze( int w, int h )
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
  \internal
  Constructs a new box with direction \a d, within \a parent.
*/
QBoxLayout::QBoxLayout(	 QBoxLayout *parent, QBasicManager::Direction d,
			 const char *name )
    : QObject( parent, name )
{
    topLevel = FALSE;
    pristine = TRUE;
    dir = d;
    bm = parent->bm;
    defBorder = parent->defBorder;
    serChain = bm->newSerChain( d );
    // parChain is perpendicular to serChain
    parChain = bm->newParChain( perp( d ) );
}



/*!
  Adds a non-stretchable space with size \a size.  QBoxLayout gives
  default border and spacing. This function adds additional space.

  \sa addStretch
*/
//###... Should perhaps replace default space?
void QBoxLayout::addSpacing( int size )
{
	bm->addSpacing( serChain, size, 0, size );
}

/*!
  Adds a stretchable space with zero minimum size
  and stretch factor \a stretch.

  \sa addSpacing
*/
//###... Should perhaps replace default space?
void QBoxLayout::addStretch( int stretch )
{
    bm->addSpacing( serChain, 0, stretch );
}



/*!
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \a size. Other constraints may
  increase the limit.

  \sa addMaxStrut()
*/
void QBoxLayout::addStrut( int size )
{
    bm->addSpacing( parChain, size, 0, 0 );
}


/*
  Limits the perpendicular dimension of the box (e.g. height if
  the box is LeftToRight) to a maximum of \a size. Other constraints
  may decrease the limit.

  \sa addMinStrut()

void QBox::addMaxStrut( int size)
{
    gm->QBasicManager::addSpacing( parChain, 0, 0, size );
}
*/

/*!
  Adds \a widget to the box, with stretch factor \a stretch and
  alignment \a a.

  The stretch factor applies only in the \link direction() direction
  \endlink of the QBoxLayout, and is relative to the other boxes and
  widgets in this QBoxLayout.  Widgets and boxes with higher strech
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
  <li> \c alignBoth aligns to both the top and bottom borders of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c alignCenter centers horizontally in the box.
  <li> \c alignLeft aligns to the left border of the box.
  <li> \c alignRight aligns to the right border of the box.
  <li> \c alignBoth aligns to both the right and left borders of the box.
  </ul>

  Alignment only has effect if the size of the box is greater than the
  widget's maximum size. \c alignBoth limits the maximum size of the
  box.

  \sa addNewBox(), addSpacing()
*/

void QBoxLayout::addWidget( QWidget *widget, int stretch, alignment a )
{

#if defined(DEBUG)
    if ( !widget ) {
	debug( "QBoxLayout::addWidget: widget == 0" );
	return;
    }

    QBoxLayout * ancestor = this;
    while ( ancestor && !ancestor->isWidgetType() )
	ancestor = (QBoxLayout *)(ancestor->parent());
    if ( !ancestor || !ancestor->isWidgetType() ) {
	debug( "QBoxLayout::addWidget: no widget ancestor found" );
	return;
    }

    // ooh! a cast and then ANOTHER cast!
    if ( (QWidget *)((QObject *)ancestor) != widget->parentWidget() ) {
	debug( "QBoxLayout::addWidget: %s(%s) is not a child of %s(%s)",
	       widget->name(), widget->className(),
	       ancestor->name(), ancestor->className() );
	return;
    }
#endif

    if ( !pristine && defaultBorder() )
	bm->addSpacing( serChain, defaultBorder(), 0, defaultBorder() );

    if ( 0/*a == alignBoth*/ ) {
	bm->addWidget( parChain, widget, 0 );
    } else {
	QBasicManager::Direction d = perp( direction() );
	QChain *sc = bm->newSerChain( d );
	if ( a == alignCenter || a == alignBottom ) {
	    bm->addSpacing(sc, 0);
	}
	bm->addWidget( sc, widget, 1 );
	if ( a == alignCenter ||  a == alignTop ) {
	    bm->addSpacing(sc, 0);
	}
	bm->add( parChain, sc );
    }
    bm->addWidget( serChain, widget, stretch );
    pristine = FALSE;
}

/*!
  Creates a new box and adds it, with serial stretch factor \a stretch.
  and \link addWidget alignment \endlink \a a. Returns a pointer to the new
  box.

  \sa addWidget(), addSpacing()
*/
QBoxLayout *QBoxLayout::addNewBox( QBasicManager::Direction d, int stretch )
{
    if ( !pristine && defaultBorder() )
	bm->addSpacing( serChain, defaultBorder(), 0, defaultBorder() );

    QBoxLayout *b = new QBoxLayout( this, d );

#if 1
    addB( b, stretch );
#else
//### duplication of logic from addWidget
    if ( a == alignBoth ) {
	addB( b, stretch );
    } else {
	QBasicManager::Direction d = perp( direction() );
	QBoxLayout *sb = new QBoxLayout( b, d );
	if ( a == alignCenter || a == alignBottom ) {
	    sb->addSpacing(0);
	}
	sb->addB( b, 1 );
	if ( a == alignCenter ||  a == alignTop ) {
	    sb->addSpacing(0);
	}
	addB( sb, stretch );
    }
#endif
    pristine = FALSE;
    return b;
}


/*!
  \fn QBasicManager::Direction QBox::direction() const

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBox::addWidget Alignment \endlink
  works perpendicular to this direction.
  \sa addWidget(), addBox(), addSpacing()
*/


void QBoxLayout::addB( QBoxLayout * b,	int stretch )
{
    if ( horz( dir ) == horz( b->dir ) ) {
	bm->QBasicManager::add( parChain, b->parChain );
	bm->QBasicManager::add( serChain, b->serChain, stretch );
    } else {
	bm->QBasicManager::add( parChain, b->serChain );
	bm->QBasicManager::add( serChain, b->parChain, stretch );
    }
}



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
    : QObject( parent, name )
{
    bm = new QBasicManager( parent, name );

    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    bm->setBorder( border );

    horChain = bm->newSerChain( QBasicManager::LeftToRight );
    verChain = bm->newSerChain( QBasicManager::Down );
    bm->add( bm->xChain(), horChain );
    bm->add( bm->yChain(), verChain );

    rows = new QArray<QChain*> ( nRows );
    cols = new QArray<QChain*> ( nCols );


    int i;
    for ( i = 0; i< nRows; i++ ) {
	if ( i != 0 )
	    bm->addSpacing( verChain, defBorder, 0, defBorder );
	(*rows)[i] = bm->newParChain( QBasicManager::Down );
	bm->add( verChain, (*rows)[i] );
    }

    for ( i = 0; i< nCols; i++ ) {
	if ( i != 0 )
	    bm->addSpacing( horChain, defBorder, 0, defBorder );
	(*cols)[i] = bm->newParChain( QBasicManager::LeftToRight );
	bm->add( horChain, (*cols)[i] );
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
    int col = fromCol;

    const int hFlags = AlignHCenter | AlignLeft | AlignRight;
    const int vFlags = AlignVCenter | AlignTop | AlignBottom;


    int a = align & hFlags;

    QChain *c;
    if ( a || fromCol != toCol ) {
	c = bm->newSerChain( QBasicManager::LeftToRight );
	if ( fromCol == toCol )
	    bm->add( (*cols)[ fromCol ], c );
	else
	    bm->addBranch( horChain, c, fromCol*2, toCol*2 );
    }
    else
	c =  (*cols)[ col ];

    if ( a ) {
	if ( a & (AlignHCenter|AlignLeft) )
	    bm->addSpacing( c, 0 );
	bm->addWidget( c, w );
	if ( a & (AlignHCenter|AlignRight) )
	    bm->addSpacing( c, 0 );
    } else
	bm->addWidget( c, w );


    int row = fromRow;
    if ( fromRow != toRow )
	warning( "multi row not implemented" );
    a = align & vFlags;
    if ( a ) {
	QChain *c = bm->newSerChain( QBasicManager::Down );
	bm->add( (*rows)[ row ], c );
	if ( a & (AlignVCenter|AlignTop) )
	    bm->addSpacing( c, 0 );
	bm->addWidget( c, w );
	if ( a & (AlignVCenter|AlignBottom) )
	    bm->addSpacing( c, 0 );
    } else
	bm->addWidget( (*rows)[ row ], w );
}


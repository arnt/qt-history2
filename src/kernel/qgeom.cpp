/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.cpp#14 $
**
**  Studies in Geometry Management
**
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/
#include "qgeom.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qgeom.cpp#14 $");



/*!
  \class QBoxLayout qgeom.h
  \brief The QBoxLayout class specifies child widget geometry.

  Contents arranged serially, either horizontal or vertical.
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
  Creates a new QBoxLayout with direction \e d and main widget \e
  parent.  \e parent may not be 0.

  \e border is the number of pixels between the edge of the widget and
  the managed children.	 \e autoBorder is the default number of pixels
  between adjacent managed children.  If \e autoBorder is -1 the value
  of \e border is used.

  \e name is passed to the QObject constructor.

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
  Constructs a new box with direction \e d, within \e parent.
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
  Adds a non-stretchable space with size \e size.  QBoxLayout gives
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
  and stretch factor \e stretch.

  \sa addSpacing
*/
//###... Should perhaps replace default space?
void QBoxLayout::addStretch( int stretch )
{
    bm->addSpacing( serChain, 0, stretch );
}



/*!
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \e size. Other constraints may
  increase the limit.

  \sa addMaxStrut()
*/
void QBoxLayout::addStrut( int size )
{
    bm->addSpacing( parChain, size, 0, 0 );
}


/*
  Limits the perpendicular dimension of the box (e.g. height if
  the box is LeftToRight) to a maximum of \e size. Other constraints
  may decrease the limit.

  \sa addMinStrut()

void QBox::addMaxStrut( int size)
{
    gm->QBasicManager::addSpacing( parChain, 0, 0, size );
}
*/

/*!
  Adds \e widget to the box, with stretch factor \e stretch and
  alignment \e a.

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
  Creates a new box and adds it, with serial stretch factor \e stretch.
  and \link addWidget alignment \endlink \e a. Returns a pointer to the new
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

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.cpp#1 $
**
**  Studies in Geometry Management
**
**  Author:   Paul Olav Tvete
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include "qgeom.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qgeom.cpp#1 $")



/*!
  \class QGeomManager qgeom.h
  \brief The QGeomManager class manages child widget geometry.

  \sa QBox
  */

/*

  Example of use:
  \code
    void MyWidget::MyWidget()
    {


	QGeomManager * gm = new QGeomManager( this, QGeomManager::Down, 5 ); 
	QBox * b1 = new ( gm, QGeomManager::LeftToRight );
	
	gm->box()->addBox(b1);

	gm->doIt()
    }
  \endcode

  Automatic deletion of orphans???


  \sa QBox
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
  Creates a new QGeomManager with direction \e d and main widget \e parent.

  /e border is space between edge of widget and area controlled by QGeomManager
  /e autoBorder is default space between objects. If /e autoBorder is -1 the 
  value of /e border is used. 
  \sa QBox, QBox::direction()
 */

QGeomManager::QGeomManager( QWidget *parent, Direction d, int border, int autoBorder, const char *name )
    :QBasicManager( parent, name )
{
    initBox( d, border, autoBorder );
}

void QGeomManager::initBox( Direction d, int border, int autoBorder )
{
    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    setBorder( border );

    QChain *s = newSerChain( d ); 
    if ( horz( d )  ) {
	QBasicManager::add( xChain(), s);
	topBox = new QBox( this, yChain(), s, d );
    } else {
	QBasicManager::add( yChain(), s);
	topBox = new QBox( this, xChain(), s, d );
    }
}



/*!
  \fn int QGeomManager::defaultBorder() const 
  Returns the default border for the geometry manager.
  */

/*!
  \fn QBox *QGeomManager::box()
  Returns the top level box into which everything is to be put.
  \sa QBox, QBox::addBox, QBox::addWidget, QBox::addSpacing
  */


/*!
  \class QBox qgeom.h
  \brief The QBox class specifies geometry for QGeomManager.

  See addWidget() for a description of the alignment flags.


  \sa QGeomManager
  */


/*!
  Constructs a new box with direction \e d, within \e geom. Use
  addBox() to place the box.
  \sa QGeomManager::box(), addBox(), direction()
 */
QBox::QBox( QGeomManager *geom, QGeomManager::Direction d )
{
    pristine = TRUE;
    dir = d;
    gm = geom;
    serChain = geom->newSerChain( d );
    // parChain is perpendicular to serChain
    parChain = geom->newParChain( perp( d ) );

}

/*!
  For esoteric or internal use. Don't use it if you don't mean it.
  */
QBox::QBox( QGeomManager * g, QChain * p, QChain * s, QGeomManager::Direction d )
{
    pristine = TRUE;
    dir = d;
    gm = g;
    parChain = p;
    serChain = s;
}


/*!
  If \e stretch is zero, adds a non-stretchable space with size
  \e size. Otherwise, adds a stretchable space with minimum size
  \e size and stretch factor \e stretch.
  
  QGeomManager gives default border and spacing. This function
  adds additional space. ... Should perhaps replace default space?
  */

void QBox::addSpacing( int size, int stretch )
{
    if ( stretch )
	gm->QBasicManager::addSpacing( serChain, size, stretch ); 
    else
	gm->QBasicManager::addSpacing( serChain, size, 0, size ); 
}

/*!  
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \e size. Other constraints may
  increase the limit.

  \sa addMaxStrut() */
void QBox::addMinStrut( int size )
{
    gm->QBasicManager::addSpacing( parChain, size, 0, 0 ); 
}


/*!
  Limits the perpendicular dimension of the box (e.g. height if
  the box is LeftToRight) to a maximum of \e size. Other constraints 
  may decrease the limit.

  \sa addMinStrut()
  */
void QBox::addMaxStrut( int size)
{
    gm->QBasicManager::addSpacing( parChain, 0, 0, size ); 
}

/*!
  Adds \e widget to the box, with serial stretch factor \e stretch
  and alignment \e a

  Alignment is perpendicular to direction(), alignment in the
  serial direction is done with addSpacing() ... unless someone
  persuades me to add some convenience functions.


  For horizontal boxes,  the possible alignments are
  <ul>
  <li> \c alignFixed makes the widget the same height as the box (default.)
  <li> \c alignCenter centers vertically in the box.
  <li> \c alignTop aligns to the top border of the box.
  <li> \c alignBottom aligns to the bottom border of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c alignFixed makes the widget the same width as the box (default.)
  <li> \c alignCenter centers horizontally in the box.
  <li> \c alignLeft aligns to the left border of the box.
  <li> \c alignRight aligns to the right border of the box.
  </ul>

  \sa addBox(), addSpacing()
  */
void QBox::addWidget( QWidget *widget, int stretch, alignment a )
{
    if ( !pristine && defBorder() )
	    gm->QBasicManager::addSpacing( serChain, defBorder(), 0, defBorder() ); 
	
    if ( a != alignFixed ) {
	QBasicManager::Direction d = perp( direction() );
	QChain *sc = gm->QBasicManager::newSerChain( d );
	if ( a == alignCenter || a == alignBottom ) {
	    gm->QBasicManager::addSpacing(sc, 0);
	}
	gm->QBasicManager::addWidget( sc, widget, 1 ); 
	if ( a == alignCenter ||  a == alignTop ) {
	    gm->QBasicManager::addSpacing(sc, 0);
	}
	gm->QBasicManager::add( parChain, sc );
    } else {
	gm->QBasicManager::addWidget( parChain, widget, 0 );
    }
    gm->QBasicManager::addWidget( serChain, widget, stretch );
    
    pristine = FALSE;
}

/*!
  Adds the box \e b to the box, with serial stretch factor \e stretch
  and \link QBox::addWidget alignment \endlink \e a.

  \sa addWidget(), addSpacing()
  */
void QBox::addBox( QBox * b, int stretch, alignment a )
{
//### duplication of logic from addWidget...
    if ( !pristine && defBorder() )
	    gm->QBasicManager::addSpacing( serChain, defBorder(), 0, defBorder() ); 

    if ( a != alignFixed ) {
	QBasicManager::Direction d = perp( direction() );
	QBox *sb = new QBox( gm, d );
	if ( a == alignCenter || a == alignBottom ) {
	    sb->addSpacing(0);
	}
	sb->addB( b, 1 ); 
	if ( a == alignCenter ||  a == alignTop ) {
	    sb->addSpacing(0);
	}
	addB( sb, stretch );
    } else {
	addB( b, stretch );
    }

    pristine = FALSE;
}


/*!
  \fn QGeomManager::Direction QBox::direction() const 

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBox::addWidget Alignment \endlink 
  works perpendicular to this direction.
  \sa addWidget(), addBox(), addSpacing()
  */

void QBox::addB( QBox * b,  int stretch )
{
    if ( horz( dir ) == horz( b->dir ) ) {
	gm->QBasicManager::add( parChain, b->parChain ); 
	gm->QBasicManager::add( serChain, b->serChain, stretch ); 
    } else {
	gm->QBasicManager::add( parChain, b->serChain ); 
	gm->QBasicManager::add( serChain, b->parChain, stretch ); 
    }
}




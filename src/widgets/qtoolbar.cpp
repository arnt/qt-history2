/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#7 $
**
** Implementation of something useful.
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtoolbar.h"

#include "qmainwindow.h"
#include "qpushbt.h"
#include "qtooltip.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjcoll.h"
#include "qpainter.h"
#include "qdrawutl.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qtoolbar.cpp#7 $");



/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a simple tool bar.

  Very simple, even.
  
  \sa QToolButton 
  <a href="http://www.iarchitect.com/visual.htm">Isys on Visual Design</a>
*/


/*! \fn void QToolBar::useBigPixmaps( bool );

  This signal is emitted when items in teh toolbar need to foo */


/*!  Constructs an empty tool bar. */

QToolBar::QToolBar( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    o = Horizontal;
    QToolTip::add( this, name );
}


/*! Destroys the object and frees any allocated resources.

*/

QToolBar::~QToolBar()
{
    delete b;
    b = 0;
    // delete d; as soon as there is a d
}


/*!  Adds a separator in here.  Cool, man. */

void QToolBar::addSeparator()
{
    QFrame * f = new QFrame( this, "tool bar separator" );
    f->setFrameStyle( QFrame::NoFrame ); // old-style whatevers
}


/*!  Sets this toolbar to organize its content vertically if \a
  newOrientation is \c Vertical and horizontally if \a newOrientation
  is \c Horizontal.
*/

void QToolBar::setOrientation( Orientation newOrientation )
{
    if ( o != newOrientation ) {
	o = newOrientation;
	setUpGM();
    }
}


/*! \fn QToolBar::Orientation QToolBar::orientation() const

  Returns the current orientation of the toolbar.

*/

/*!  Reimplemented to set up geometry management. */

void QToolBar::show()
{
    setUpGM();
    QWidget::show();
}


/*!  Sets up geometry management for this toolbar. */

void QToolBar::setUpGM()
{
    delete b;
    b = new QBoxLayout( this, orientation() == Vertical
			? QBoxLayout::Down : QBoxLayout::LeftToRight,
			style() == MotifStyle ? 2 : 1, 0 );

    if ( style() == WindowsStyle )
	b->addSpacing( 8 );

    QBoxLayout * fl = 0;

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() ) {
	    if ( !strcmp( "tool bar separator", o->name() ) &&
		 !strcmp( "QFrame", o->className() ) ) {
		QFrame * f = (QFrame *)o;
		if ( style() == MotifStyle ) {
		    f->setFrameStyle( QFrame::NoFrame );
		    if ( orientation() == Vertical ) {
			f->setMinimumSize( 0, 6 );
			f->setMaximumSize( 32767, 6 );
		    } else {
			f->setMinimumSize( 6, 0 );
			f->setMaximumSize( 6, 32767 );
		    }
		    b->addWidget( (QWidget *)o, 0 );
		} else {
		    if ( orientation() == Vertical ) {
			f->setMinimumSize( 0, 8 );
			f->setMaximumSize( 32767, 8 );
			f->setFrameStyle( QFrame::HLine + QFrame::Sunken );
			fl = new QBoxLayout( QBoxLayout::LeftToRight );
		    } else {
			f->setMinimumSize( 8, 0 );
			f->setMaximumSize( 8, 32767 );
			f->setFrameStyle( QFrame::VLine + QFrame::Sunken );
			fl = new QBoxLayout( QBoxLayout::Down );
		    }
		    b->addLayout( fl );
		    fl->addSpacing( 1 );
		    fl->addWidget( (QWidget *)o, 1 );
		    fl->addSpacing( 1 );
		}
	    } else {
		b->addWidget( (QWidget *)o, 0 );
	    }
	}
    }

    b->activate();
}


/*!  Paint the handle.

*/

void QToolBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    if ( style() == MotifStyle ) {
	qDrawShadePanel( &p, 0, 0, width(), height(),
			 colorGroup(), FALSE, 1, 0 );
    } else {
	qDrawShadePanel( &p, 0, 0, width(), height(),
			 colorGroup(), FALSE, 1, 0 );
	if ( orientation() == Vertical )
	    qDrawShadePanel( &p, 3, 3, width() - 6, 3,
			     colorGroup(), FALSE, 1, 0 );
	else
	    qDrawShadePanel( &p, 3, 3, 3, height() - 6,
			     colorGroup(), FALSE, 1, 0 );
    }
}

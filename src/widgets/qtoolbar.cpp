/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#2 $
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

RCSTAG("$Id: //depot/qt/main/src/widgets/qtoolbar.cpp#2 $");



/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a simple tool bar.

  Very simple, even.
*/


/*!  Constructs an empty tool bar. */

QToolBar::QToolBar( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    o = Horizontal;
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
			? QBoxLayout::Up : QBoxLayout::LeftToRight, 1, 0 );

    if ( style() == WindowsStyle )
	b->addSpacing( 8 );

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() ) {
	    if ( !strcmp( "tool bar separator", o->name() ) &&
		 !strcmp( "QFrame", o->className() ) ) {
		QFrame * f = (QFrame *)o;
		if ( orientation() == Vertical ) {
		    f->setMinimumSize( 0, 4 );
		    f->setMaximumSize( 32767, 4 );
		    // f->setFrameStyle( QFrame::HLine + QFRame::Raised );
		} else {
		    f->setMinimumSize( 4, 0 );
		    f->setMaximumSize( 4, 32767 );
		    // f->setFrameStyle( QFrame::VLine + QFRame::Raised );
		}
	    }
	    b->addWidget( (QWidget *)o, 0 );
	    //QWidget * dbg = (QWidget *)o;
	
	    //debug( "%s/%s: %d,%d %d,%d", o->name(), o->className(),
	    //	   dbg->width(), dbg->height(),
	    //	   dbg->minimumSize().width(), dbg->minimumSize().height() );
	}
    }

    b->activate();
}


/*!  Paint the handle.

*/

void QToolBar::paintEvent( QPaintEvent * )
{
    if ( style() != WindowsStyle )
	return;

    QPainter p( this );
    qDrawShadePanel( &p, 0, 0, width(), height(),
		     colorGroup(), FALSE, 1, 0 );
    if ( orientation() == Vertical )
        qDrawShadePanel( &p, 3, 3, width() - 6, 3,
			 colorGroup(), FALSE, 1, 0 );
    else
        qDrawShadePanel( &p, 3, 3, 3, height() - 6,
			 colorGroup(), FALSE, 1, 0 );
}

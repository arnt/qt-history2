/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#19 $
**
** Implementation of QToolBar class
**
** Created : 980315
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

/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a simple tool bar.

  \ingroup realwidgets
  \ingroup application

  Very simple, even.

  \sa QToolButton QMainWindow
  <a href="http://www.iarchitect.com/visual.htm">Isys on Visual Design,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Bar</a>
*/


/*!  Constructs an empty tool bar which is a chilf od \a parent and
  managed by \a parent, initially in \a dock, labelled \a and starting
  a new line in the dock if \a newLine is TRUE.  \a name is the object
  name, as usual.
*/

QToolBar::QToolBar( const char * label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    mw = parent;
    sw = 0;
    o = (dock == QMainWindow::Left || dock == QMainWindow::Right )
	? Vertical : Horizontal;
    parent->addToolBar( this, label, dock, newLine );
}


/*!  Constructs an empty horizontal tool bar which is a parent of \a
  parent and managed by \a mainWindow.  The \a label and \a newLine
  are passed straight to QMainWindow::addToolBar().  \a name is the
  object name and \a f is the widget flags.

  This is the constructor to use if you want to create torn-off
  toolbars, or toolbars in the status bar.
*/

QToolBar::QToolBar( const char * label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QWidget( parent, name, f )
{
    d = 0;
    b = 0;
    mw = mainWindow;
    sw = 0;
    o = Horizontal;
    mainWindow->addToolBar( this, label, QMainWindow::Unmanaged, newLine );
}


/*!  Constructs an empty tool bar in the top dock of its parent,
  without any label and without requiring a newline.  This is mostly
  useless. */

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    o = Horizontal;
    sw = 0;
    mw = parent;
    parent->addToolBar( this, 0, QMainWindow::Top );
}


/*! Destroys the object and frees any allocated resources. */

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
			style() == WindowsStyle ? 2 : 1, 0 );

    if ( style() == WindowsStyle )
	b->addSpacing( 9 );

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() ) {
	    QWidget * w = (QWidget *)o;
	    if ( !qstrcmp( "tool bar separator", o->name() ) &&
		 !qstrcmp( "QFrame", o->className() ) ) {
		QFrame * f = (QFrame *)o;
		if ( orientation() == Vertical ) {
		    f->setMinimumSize( 0, 6 );
		    f->setMaximumSize( 32767, 6 );
		    if ( style() == WindowsStyle )
			f->setFrameStyle( QFrame::HLine + QFrame::Sunken );
		    else
			f->setFrameStyle( QFrame::NoFrame );
		} else {
		    f->setMinimumSize( 6, 0 );
		    f->setMaximumSize( 6, 32767 );
		    if ( style() == WindowsStyle )
			f->setFrameStyle( QFrame::VLine + QFrame::Sunken );
		    else
			f->setFrameStyle( QFrame::NoFrame );
		}
	    } else {
		QSize s( w->sizeHint() );
		if ( s.width() > 0 && s.height() > 0 )
		    w->setMinimumSize( s );
		else if ( s.width() > 0 )
		    w->setMinimumWidth( s.width() );
		else if ( s.height() > 0 )
		    w->setMinimumHeight( s.width() );
	    }
	    b->addWidget( w, w == sw ? 42 : 0 );
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
	if ( orientation() == Vertical ) {
	    qDrawShadePanel( &p, 2, 4, width() - 4, 3,
			     colorGroup(), FALSE, 1, 0 );
	    qDrawShadePanel( &p, 2, 7, width() - 4, 3,
			     colorGroup(), FALSE, 1, 0 );
	} else {
	    qDrawShadePanel( &p, 4, 2, 3, height() - 4,
			     colorGroup(), FALSE, 1, 0 );
	    qDrawShadePanel( &p, 7, 2, 3, height() - 4,
			     colorGroup(), FALSE, 1, 0 );
	}
    }
}



/*!  Returns a pointer to the QMainWindow which controls this tool bar.
*/

QMainWindow * QToolBar::mainWindow()
{
    return mw;
}


/*!  Sets \a w to be expanded if this toolbar is requested to stretch
  (because QMainWindow right-justifies the dock it's in).
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
}

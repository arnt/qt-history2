/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#56 $
**
** Implementation of QToolBar class
**
** Created : 980315
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtoolbar.h"

#include "qmainwindow.h"
#include "qpushbutton.h"
#include "qtooltip.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"

class QToolBarPrivate
{
public:
    QToolBarPrivate() : moving( FALSE ), stretchable( FALSE ) {}
    bool moving;
    bool stretchable;
};


class QToolBarSeparator : public QFrame
{
    Q_OBJECT;
public:
    QToolBarSeparator( Orientation, QToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;
    Orientation orientation() const { return orient; }
public slots:
   void setOrientation( Orientation );
private:
    Orientation orient;
};



QToolBarSeparator::QToolBarSeparator(Orientation o , QToolBar *parent,
				     const char* name )
    :QFrame( parent, name )
{
    connect( parent, SIGNAL(orientationChanged(Orientation)),
	     this, SLOT(setOrientation(Orientation)) );
    setOrientation( o );
}



void QToolBarSeparator::setOrientation( Orientation o )
{
    orient = o;
    if ( style() == WindowsStyle ) {
	if ( orientation() == Vertical )
	    setFrameStyle( HLine + Sunken );
	else
	    setFrameStyle( VLine + Sunken );
    } else {
	    setFrameStyle( NoFrame );
    }
}


QSize QToolBarSeparator::sizeHint() const
{
    return orientation() == Vertical ? QSize( 0, 6 ) : QSize( 6, 0 );
}

QSizePolicy QToolBarSeparator::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

#include "qtoolbar.moc"


// NOT REVISED
/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a tool bar.

  \ingroup realwidgets
  \ingroup application

  A toolbar is a panel that contains a set of controls, usually
  represented by small icons.  It's purpose is to provide quick access
  to frequently used commands or options. Within a main window, the
  user can drag toolbars freely around and hide them with a click on
  the toolbar handle.

  To use QToolBar, you simply create a QToolBar as child of a
  QMainWindow, create a number of QToolButton widgets (or other
  widgets) in left to right (or top to bottom) order, call
  addSeparator() when you want a separator, and that's all.

  The application/application.cpp example does precisely this.

  You may use any kind of widget within a toolbar, with QToolButton
  and QComboBox being the two most common ones.

  Each QToolBar lives in a \link QMainWindow dock \endlink in a
  QMainWindow, and can optionally start a new line in its dock.  Tool
  bars that start a new line are always positioned at the left end or
  top of the tool bar dock; others are placed next to the previous
  tool bar and word-wrapped as necessary.

  Usually, a toolbar gets just the space it needs. However, with
  setStretchable() or setStretchableWidget() you can advise the main
  window to expand the toolbar horizontally to fill all available width.

  The tool bar arranges its buttons either horizontally or vertically
  (see setOrientation() for details). Generally, QMainWindow will set
  the orientation correctly for you. The toolbar emits a signal
  orientationChanged() each time the orientation changes, in case some
  child widgets need adjustments.

  \sa QToolButton QMainWindow
  <a href="http://www.iarchitect.com/visual.htm">Parts of Isys on Visual Design</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Bar.</a>
*/


/*!  Constructs an empty tool bar which is a child of \a parent and
  managed by \a parent, initially in \a dock, labelled \a and starting
  a new line in the dock if \a newLine is TRUE.  \a name is the object
  name, as usual.
*/

QToolBar::QToolBar( const QString &label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QWidget( parent, name )
{
    mw = parent;
    o = (dock == QMainWindow::Left || dock == QMainWindow::Right )
	? Vertical : Horizontal;
    init();

    if ( parent )
	parent->addToolBar( this, label, dock, newLine );
}


/*!  Constructs an empty horizontal tool bar with a parent of \a
  parent and managed by \a mainWindow.  The \a label and \a newLine
  are passed straight to QMainWindow::addToolBar().  \a name is the
  object name and \a f is the widget flags.

  This is the constructor to use if you want to create torn-off
  toolbars, or toolbars in the status bar.
*/

QToolBar::QToolBar( const QString &label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QWidget( parent, name, f )
{
    mw = mainWindow;
    o = Horizontal;
    init();

    if ( mainWindow )
	mainWindow->addToolBar( this, label, QMainWindow::Unmanaged, newLine );
}


/*!  Constructs an empty tool bar in the top dock of its parent,
  without any label and without requiring a newline.  This is mostly
  useless. */

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QWidget( parent, name )
{
    mw = parent;
    o = Horizontal;
    init();

    if ( parent )
	parent->addToolBar( this, QString::null, QMainWindow::Top );
}

/*!
  Common initialization code. Requires that \c mw and \c o are set.
  Does not call QMainWindow::addToolBar
*/
void QToolBar::init()
{
    d = new QToolBarPrivate;
    b = 0;
    sw = 0;

    b = new QBoxLayout( this, orientation() == Vertical
			? QBoxLayout::Down : QBoxLayout::LeftToRight,
			style() == WindowsStyle ? 2 : 1, 0 );
    b->setAutoAdd( TRUE );
    b->addSpacing( 9 );

    if ( mw ) {
	connect( mw, SIGNAL( startMovingToolbar( QToolBar * ) ),
		 this, SLOT( startMoving( QToolBar * ) ) );
	connect( mw, SIGNAL( endMovingToolbar( QToolBar * ) ),
		 this, SLOT( endMoving( QToolBar * ) ) );
    }
#if defined(CHECK_NULL)
    else
	qWarning( "QToolBar::QToolBar main window cannot be 0." );
#endif
    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
}


/*! Destructs the object and frees any allocated resources. */

QToolBar::~QToolBar()
{
    delete b;
    b = 0;
    delete d;
    d = 0;
}


/*!  Adds a separator to the end of the toolbar. */

void QToolBar::addSeparator()
{
    (void) new QToolBarSeparator( orientation(), this, "tool bar separator" );
}


/*!  Sets this toolbar to organize its content vertically if \a
  newOrientation is \c Vertical and horizontally if \a newOrientation
  is \c Horizontal.

  Emits the orientationChanged() signal.

  \sa orientation()
*/

void QToolBar::setOrientation( Orientation newOrientation )
{
    if ( o != newOrientation ) {
	o = newOrientation;
	if ( b )
	    b->setDirection( o==Horizontal ? QBoxLayout::LeftToRight :
			     QBoxLayout::TopToBottom );
	emit orientationChanged( newOrientation );
    }
}


/*! \fn Orientation QToolBar::orientation() const

  Returns the current orientation of the toolbar.
*/

/*!  \reimp. */

void QToolBar::show()
{
    //Does nothing, present for binary compatibility
    QWidget::show();
}


void QToolBar::setUpGM()
{
    //Does nothing, present for binary compatibility
}


/*!
  Paint the handle.  The Motif style is rather close to Netscape
  and even closer to KDE.
*/

void QToolBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    qDrawShadePanel( &p, 0, 0, width(), height(),
		     colorGroup(), FALSE, 1, 0 );
    style().drawToolBarHandle( &p, QRect( 0, 0, width(), height() ),
			       orientation(), d->moving, colorGroup() );
}



/*!  Returns a pointer to the QMainWindow which controls this tool bar.
*/

QMainWindow * QToolBar::mainWindow()
{
    return mw;
}


/*!
  Sets \a w to be expanded if this toolbar is requested to stretch
  (because QMainWindow right-justifies the dock it's in or isStretchable()
  of this toolbar is TRUE).

  If you call setStretchableWidget() and the toolbar is not stretchable
  yet, setStretchable(  ) is called.

  \sa QMainWindow::setRightJustification(), setStretchable()
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
    b->setStretchFactor( w, 1 );

    if ( !isStretchable() )
	setStretchable( TRUE );
}


/*! \reimp */

bool QToolBar::event( QEvent * e )
{
    bool r =  QWidget::event( e );
    //after the event filters have dealt with it:
    if ( e->type() == QEvent::ChildInserted ) {
	QObject * child = ((QChildEvent*)e)->child();
	if ( child && child->isWidgetType() && ((QWidget*)child) == sw )
	    b->setStretchFactor( (QWidget*)child, 1 );
    }
    return r;
}


/*! \reimp */

bool QToolBar::eventFilter( QObject * obj, QEvent * e )
{
    //Does nothing, present for binary compatibility
    return QWidget::eventFilter( obj, e );
}


/*!  Sets the label of this tool bar to \a label.  The label is not
currently used; it will be used in a forthcoming tool bar
configuration dialog.

\sa label()
*/

void QToolBar::setLabel( const QString & label )
{
    l = label;
}


/*!  Returns the label of this tool bar.

  \sa setLabel()
*/

QString QToolBar::label() const
{
    return l;
}


/*!
  Clears the toolbar, deleting all childwidgets.
 */
void QToolBar::clear()
{
    if ( !children() )
	return;
    QObjectList list = *children();
    for (QObjectListIt it(list); it.current(); ++it) {
	if ( it.current()->isWidgetType() )
	    delete it.current();
    }
}

/*!
  \internal
*/

void QToolBar::startMoving( QToolBar *tb )
{
    if ( tb == this ) {
	d->moving = TRUE;
	repaint( FALSE );
    }
}

/*!
  \internal
*/

void QToolBar::endMoving( QToolBar *tb )
{
    if ( tb == this && d->moving ) {
	d->moving = FALSE;
	repaint( TRUE );
    }
}

/*!
  Sets the toolbar to be stretchable if \a stretchable is TRUE, or
  non-stretchable otherwise.

  A stretchable toolbar fills the available width in a toolbar dock. A
  non-stretchable toolbar usually gets just the space it needs.

  The default is FALSE.

  \sa QMainWindow::setRightJustification(), isStretchable()
*/

void QToolBar::setStretchable( bool b )
{

    if ( d->stretchable != b ) {
	d->stretchable = b;
	if ( mw )
	    mw->triggerLayout( FALSE );
    }
}

/*!
  Returns whether the toolbar is stretchable or not.

  A stretchable toolbar fills all available width in a toolbar dock. A
  non-stretchable toolbar usually gets just the space it needs.

  \sa setStretchable()
*/

bool QToolBar::isStretchable() const
{
    return d->stretchable;
}


/*!
  \fn void QToolBar::orientationChanged( Orientation newOrientation )

  This signal is emitted when the toolbar changed its orientation to
  \a newOrientation.
*/


/* from chaunsee:

1.  Toolbars should contain only high-frequency functions.  Avoid putting
things like About and Exit on a toolbar unless they are frequent functions.

2.  All toolbar buttons must have some keyboard access method (it can be a
menu or shortcut key or a function in a dialog box that can be accessed
through the keyboard).

3.  Make toolbar functions as efficient as possible (the common example is to
Print in Microsoft applications, it doesn't bring up the Print dialog box, it
prints immediately to the default printer).

4.  Avoid turning toolbars into graphical menu bars.  To me, a toolbar should
be efficient. Once you make almost all the items in a toolbar into graphical
pull-down menus, you start to loose efficiency.

5.  Make sure that adjacent icons are distinctive. There are some toolbars
where you see a group of 4-5 icons that represent related functions, but they
are so similar that you can't differentiate among them.  These toolbars are
often a poor attempt at a "common visual language".

6.  Use any de facto standard icons of your platform (for windows use the
cut, copy, and paste icons provided in dev kits rather than designing your
own).

7.  Avoid putting a highly destructive toolbar button (delete database) by a
safe, high-frequency button (Find) -- this will yield 1-0ff errors).

8.  Tooltips in many Microsoft products simply reiterate the menu text even
when that is not explanatory.  Consider making your tooltips slightly more
verbose and explanatory than the corresponding menu item.

9.  Keep the toolbar as stable as possible when you click on different
objects. Consider disabling toolbar buttons if they are used in most, but not
all contexts.

10.  If you have multiple toolbars (like the Microsoft MMC snap-ins have),
put the most stable toolbar to at the left with less stable ones to the
right. This arrangement (stable to less stable) makes the toolbar somewhat
more predictable.

11.  Keep a single toolbar to fewer than 20 items divided into 4-7 groups of
items.
*/

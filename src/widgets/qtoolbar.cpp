/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#56 $
**
** Implementation of QToolBar class
**
** Created : 980315
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtoolbar.h"
#ifndef QT_NO_TOOLBAR

#include "qmainwindow.h"
#include "qpushbutton.h"
#include "qtooltip.h"
#include "qcursor.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjectlist.h"
#include "qintdict.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qtoolbutton.h"
#include "qpopupmenu.h"
#include "qtimer.h"
#include "qwidgetlist.h"

static const char * arrow_v_xpm[] = {
"7 9 3 1",
" 	c None",
".	c #000000",
"+	c none",
".+++++.",
"..+++..",
"+..+..+",
"++...++",
".++.++.",
"..+++..",
"+..+..+",
"++...++",
"+++.+++"};

static const char * arrow_h_xpm[] = {
"9 7 3 1",
" 	c None",
".	c #000000",
"+	c none",
"..++..+++",
"+..++..++",
"++..++..+",
"+++..++..",
"++..++..+",
"+..++..++",
"..++..+++"};

class QToolBarExtensionWidget;

class QToolBarPrivate
{
public:
    QToolBarPrivate() : moving( FALSE ), button( 0 ) {}

    bool moving;
    QToolBarExtensionWidget *extension;
    QPopupMenu *extensionPopup;
    QIntDict<QButton> hiddenItems;
    QButton *button;

};


class QToolBarSeparator : public QFrame
{
    Q_OBJECT
public:
    QToolBarSeparator( Orientation, QToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Orientation orientation() const { return orient; }
public slots:
   void setOrientation( Orientation );
protected:
    void styleChange( QStyle& );
private:
    Orientation orient;
};

class QToolBarExtensionWidget : public QWidget
{
    Q_OBJECT
	
public:
    QToolBarExtensionWidget( QWidget *w );
    void setOrientation( Orientation o );
    QToolButton *button() const { return tb; }

protected:
    void resizeEvent( QResizeEvent *e ) {
	QWidget::resizeEvent( e );
	layOut();
    }

private:
    void layOut();
    QToolButton *tb;
    Orientation orient;

};

QToolBarExtensionWidget::QToolBarExtensionWidget( QWidget *w )
    : QWidget( w, "qt_dockwidget_internal" )
{
    tb = new QToolButton( this );
    tb->setAutoRaise( TRUE );
    setOrientation( Horizontal );
}

void QToolBarExtensionWidget::setOrientation( Orientation o )
{
    orient = o;
    if ( orient == Horizontal )
	tb->setPixmap( QPixmap( arrow_h_xpm ) );
    else
	tb->setPixmap( QPixmap( arrow_v_xpm ) );
    layOut();
}

void QToolBarExtensionWidget::layOut()
{
    tb->setGeometry( 2, 2, width() - 4, height() - 4 );
}

QToolBarSeparator::QToolBarSeparator(Orientation o , QToolBar *parent,
				     const char* name )
    : QFrame( parent, name )
{
    connect( parent, SIGNAL(orientationChanged(Orientation)),
	     this, SLOT(setOrientation(Orientation)) );
    setOrientation( o );
    setBackgroundMode( parent->backgroundMode() );
    setBackgroundOrigin( ParentOrigin );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
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

void QToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize QToolBarSeparator::sizeHint() const
{
    return orientation() == Vertical ? QSize( 0, 6 ) : QSize( 6, 0 );
}

#include "qtoolbar.moc"


// NOT REVISED
/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a movable panel containing
  widgets such as tool buttons.

  \ingroup application

  A toolbar is a panel that contains a set of controls, usually
  represented by small icons.  It's purpose is to provide quick access
  to frequently used commands or options. Within a main window, the
  user can drag toolbars freely around and hide them with a click on
  the toolbar handle.

  A QToolBar is a special QDockWindow, and so provides also all
  functionality of a QDockWindow. For example, within a QMainWindow the
  user can drag tool bars freely around.
  
  To use QToolBar you simply create a QToolBar as child of a
  QMainWindow, create a number of QToolButton widgets (or other
  widgets) in left to right (or top to bottom) order, call
  addSeparator() when you want a separator - that's all.

  The application/application.cpp example does precisely this.

  You may use any kind of widget within a tool bar, with QToolButton
  and QComboBox being the two most common.

  As QDockWindows, QToolBars live in QDockAreas. A QMainWindow already
  contains four QDockAreas, one on each edge. When you create a QToolBar
  as child of a QMainWindow, QMainWindow locates it in one of the dock
  areas.

  The main window can be resized to a smaller size than a tool bar would
  need to show all items. If this happens, QToolbar shows a little arrow
  button at its right or bottom end. Clicking on on that button pops up a
  menu that shows the "overflowing" items.

  Usually a toolbar gets just the space it needs. However, with
  setHorizontalStretchable()/setVerticalStretchable() or
  setStretchableWidget() you can advise the main window to expand
  the tool bar to fill all available width in the specified orientation.

  The tool bar arranges its buttons either horizontally or vertically (see
  orientation() for details). Generally, QDockArea will set the
  orientation correctly for you, and in case anyone needs readjustment,
  the orientationChanged() signal is emitted every time the orientation
  changes.

  You can use the clear() method to remove all items from a tool bar.

  \sa QToolButton QMainWindow
  <a href="http://www.iarchitect.com/visual.htm">Parts of Isys on Visual Design</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Bar.</a>
*/


/*!  Constructs an empty tool bar that is a child of \a parent and
  managed by \a parent, initially in \a dock, labeled \a and starting
  a newline in the dock if \a newLine is TRUE.  \a name is the object
  name, as usual.
*/

QToolBar::QToolBar( const QString &label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QDockWindow( InDock, parent, name )
{
    mw = parent;
    init();

    if ( parent )
	parent->addToolBar( this, label, dock, newLine );
}


/*!  Constructs an empty horizontal tool bar with a parent of \a
  parent and managed by \a mainWindow.  The \a label and \a newLine
  are passed straight to QMainWindow::addDockWindow().  \a name is the
  object name and \a f is the widget flags.

  This is the constructor to use if you want to create torn-off
  tool bars or tool bars in the status bar.
*/

QToolBar::QToolBar( const QString &label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QDockWindow( InDock, parent, name, f )
{
    mw = mainWindow;
    init();

    if ( mainWindow )
	mainWindow->addToolBar( this, label, QMainWindow::Unmanaged, newLine );
}


/*!  Constructs an empty tool bar in the top dock of its parent,
  without any label and without requiring a newline.
*/

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QDockWindow( InDock, parent, name )
{
    mw = parent;
    init();

    if ( parent )
	parent->addToolBar( this, QString::null, QMainWindow::Top );
}

/*!
  Common initialization code. Requires that \c mw and \c o are set.
  Does not call QMainWindow::addDockWindow()
*/
void QToolBar::init()
{
    d = new QToolBarPrivate;
    d->extension = new QToolBarExtensionWidget( this );
    d->extension->hide();
    d->extensionPopup = new QPopupMenu( this, "qt_dockwidget_internal" );
    connect( d->extensionPopup, SIGNAL( activated( int ) ),
	     this, SLOT( popupSelected( int ) ) );
    connect( d->extensionPopup, SIGNAL( aboutToShow() ),
	     this, SLOT( setupArrowMenu() ) );
    d->extension->button()->setPopup( d->extensionPopup );
    d->extension->button()->setPopupDelay( -1 );
    sw = 0;

#if defined(QT_CHECK_NULL)
    if ( !mw )
	qWarning( "QToolBar::QToolBar main window cannot be 0." );
#endif
    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
}

/*! \reimp */

QToolBar::~QToolBar()
{
    delete d;
    d = 0;
}

/*! \reimp */

void QToolBar::setOrientation( Orientation o )
{
    QDockWindow::setOrientation( o );
    d->extension->setOrientation( o );
}

/*!  Adds a separator to the end of the tool bar. */

void QToolBar::addSeparator()
{
    (void) new QToolBarSeparator( orientation(), this, "tool bar separator" );
}


/*!  \reimp. */

void QToolBar::show()
{
    QDockWindow::show();
    if ( mw )
	mw->triggerLayout( FALSE );
}


/*!
  \reimp
*/

void QToolBar::hide()
{
    QDockWindow::hide();
    if ( mw )
	mw->triggerLayout( FALSE );
}

/*!  Returns a pointer to the QMainWindow which controls this tool bar.
*/

QMainWindow * QToolBar::mainWindow()
{
    return mw;
}


/*! Sets \a w to be expanded if this tool bar is requested to stretch
  (because QMainWindow right-justifies the dock it's in, or
  isVerticalStretchable() or isHorizontalStretchable() if this tool bar is
  TRUE).

  If you call setStretchableWidget() and the tool bar is not yet
  stretchable, setStretchable( ) is called.

  \sa QMainWindow::setRightJustification(), setVerticalStretchable(),
  setHorizontalStretchable()
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
    boxLayout()->setStretchFactor( w, 1 );

    if ( !isHorizontalStretchable() && !isVerticalStretchable() ) {
	if ( orientation() == Horizontal )
	    setHorizontalStretchable( TRUE );
	else
	    setVerticalStretchable( TRUE );
    }
}


/*! \reimp */

bool QToolBar::event( QEvent * e )
{
    bool r =  QDockWindow::event( e );
    //after the event filters have dealt with it:
    if ( e->type() == QEvent::ChildInserted ) {
	QObject * child = ((QChildEvent*)e)->child();
	if ( child && child->isWidgetType() && !((QWidget*)child)->isTopLevel() &&
	     child->parent() == this && qstrcmp( "qt_dockwidget_internal", child->name() ) != 0 ) {
	    boxLayout()->addWidget( (QWidget*)child );
	    if ( isVisibleTo(0) )
		( (QWidget*)child )->show();
	}
	if ( child && child->isWidgetType() && ((QWidget*)child) == sw )
	    boxLayout()->setStretchFactor( (QWidget*)child, 1 );
    }
    return r;
}


/*!  Sets the label of this tool bar to \a label.

Whenever a user drags the toolbar and drops it elsewhere
on the desktop, the toolbar becomes a window on its own with
\a label as caption.

\sa label()
*/

void QToolBar::setLabel( const QString & label )
{
    l = label;
    setCaption( l );
}


/*!  Returns the label of this tool bar.

  \sa setLabel()
*/

QString QToolBar::label() const
{
    return l;
}


/*! Clears the tool bar, deleting all child widgets. */

void QToolBar::clear()
{
    if ( !children() )
	return;
    QObjectListIt it( *children() );
    QObject * obj;
    while( (obj=it.current()) != 0 ) {
	++it;
	if ( obj->isWidgetType() &&
	     qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 )
	    delete obj;
    }
}

/*!  \reimp */

QSize QToolBar::minimumSize() const
{
    if ( orientation() == Horizontal )
	return QSize( 0, QDockWindow::minimumSize().height() );
    return QSize( QDockWindow::minimumSize().width(), 0 );
}

/*!
  \reimp
*/

QSize QToolBar::minimumSizeHint() const
{
    if ( orientation() == Horizontal )
	return QSize( 0, QDockWindow::minimumSizeHint().height() );
    return QSize( QDockWindow::minimumSizeHint().width(), 0 );
}

/*!
  \reimp
*/

void QToolBar::resizeEvent( QResizeEvent *e )
{
    bool tooSmall;
    if ( orientation() ==  Horizontal )
	tooSmall = e->size().width() < sizeHint().width();
    else
	tooSmall = e->size().height() < sizeHint().height();
    if ( tooSmall ) {
	QObjectListIt it( *children() );
	bool hide = FALSE;
	bool doHide = FALSE;
	d->extensionPopup->clear();
	d->hiddenItems.clear();
	while ( it.current() ) {
	    if ( !it.current()->isWidgetType() ||
		 qstrcmp( "qt_dockwidget_internal", it.current()->name() ) == 0 ) {
		++it;
		continue;
	    }
	    QWidget *w = (QWidget*)it.current();
	    hide = FALSE;
	    if ( orientation() == Horizontal ) {
		if ( w->x() + w->width() > e->size().width() )
		    hide = TRUE;
	    } else {
		if ( w->y() + w->height() > e->size().height() )
		    hide = TRUE;
	    }
	    if ( hide ) {
		doHide = TRUE;
		if ( w->inherits( "QToolButton" ) ) {
		    QToolButton *b = (QToolButton*)w;
		    QString s = b->textLabel();
		    if ( s.isEmpty() )
			s = "";
		    int id = d->extensionPopup->insertItem( b->iconSet(), s );
		    d->hiddenItems.insert( id, b );
		    if ( b->isToggleButton() )
			d->extensionPopup->setItemChecked( id, b->isOn() );
		    if ( !b->isEnabled() )
			d->extensionPopup->setItemEnabled( id, FALSE );
		} else if ( w->inherits( "QButton" ) ) {
		    QButton *b = (QButton*)w;
		    QString s = b->text();
		    if ( s.isEmpty() )
			s = "";
		    int id = -1;
		    if ( b->pixmap() )
			id = d->extensionPopup->insertItem( *b->pixmap(), s );
		    else
			id = d->extensionPopup->insertItem( s );
		    d->hiddenItems.insert( id, b );
		    if ( b->isToggleButton() )
			d->extensionPopup->setItemChecked( id, b->isOn() );
		    if ( !b->isEnabled() )
			d->extensionPopup->setItemEnabled( id, FALSE );
		}
	    }
	    ++it;
	}
	if ( doHide ) {
	    if ( orientation() == Horizontal )
		d->extension->setGeometry( width() - 20, 1, 19, height() - 2 );
	    else
		d->extension->setGeometry( 1, height() - 20, width() - 2, 19 );
	    d->extension->show();
	    d->extension->raise();
	} else {
	    d->extension->hide();
	}
    } else {
	d->extension->hide();
    }
}

void QToolBar::popupSelected( int id )
{
    QButton *b = d->hiddenItems.find( id );
    d->button = b;
    if ( d->button )
	QTimer::singleShot( 0, this, SLOT( emulateButtonClicked() ) );
}

void QToolBar::emulateButtonClicked()
{
    if ( !d->button )
	return;

    if ( d->button->inherits( "QPushButton" ) &&
	 ( (QPushButton*)d->button )->popup() ) {
	( (QPushButton*)d->button )->popup()->exec( QCursor::pos() );
    } else if ( d->button->inherits( "QToolButton" ) &&
	   ( (QToolButton*)d->button )->popup() ) {
	( (QToolButton*)d->button )->popup()->exec( QCursor::pos() );
    } else if ( d->button->isToggleButton() ) {
	d->button->setOn( !d->button->isOn() );
	emit d->button->pressed();
	emit d->button->released();
	emit d->button->clicked();
	if ( d->button->inherits( "QWhatsThisButton" ) )
	    d->button->setOn( FALSE );
    } else {
	emit d->button->pressed();
	emit d->button->released();
	emit d->button->clicked();
    }
    d->button = 0;
}

void QToolBar::setupArrowMenu()
{
    QIntDictIterator<QButton> it( d->hiddenItems );
    while ( it.current() ) {
	d->extensionPopup->setItemEnabled( it.currentKey(), it.current()->isEnabled() );
	d->extensionPopup->setItemChecked( it.currentKey(), it.current()->isOn() );
	++it;
    }
}

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
pull-down menus, you start to lose efficiency.

5.  Make sure that adjacent icons are distinctive. There are some toolbars
where you see a group of 4-5 icons that represent related functions, but they
are so similar that you can't differentiate among them.  These toolbars are
often a poor attempt at a "common visual language".

6.  Use any de facto standard icons of your platform (for windows use the
cut, copy and paste icons provided in dev kits rather than designing your
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
#endif

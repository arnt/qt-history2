/****************************************************************************
** $Id: $
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
#include "qtoolbutton.h"
#include "qpopupmenu.h"
#include "qtimer.h"
#include "qwidgetlist.h"
#include "qstyle.h"

static const char * arrow_v_xpm[] = {
    "7 9 3 1",
    "	    c None",
    ".	    c #000000",
    "+	    c none",
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
    "	    c None",
    ".	    c #000000",
    "+	    c none",
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
    QToolBarPrivate() : moving( FALSE ), firstShow( TRUE ), button( 0 ) {}

    bool moving;
    bool firstShow;
    QToolBarExtensionWidget *extension;
    QPopupMenu *extensionPopup;
    QIntDict<QButton> hiddenItems;
    QButton *button;

};


class QToolBarSeparator : public QWidget
{
    Q_OBJECT
public:
    QToolBarSeparator( Orientation, QToolBar *parent, const char* Q_NAME );

    QSize sizeHint() const;
    Orientation orientation() const { return orient; }
public slots:
    void setOrientation( Orientation );
protected:
    void styleChange( QStyle& );
    void paintEvent( QPaintEvent * );

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
    tb = new QToolButton( this, "qt_toolbar_ext_button" );
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
    : QWidget( parent, name )
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
}

void QToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize QToolBarSeparator::sizeHint() const
{
    int extent = style().pixelMetric( QStyle::PM_DockWindowSeparatorExtent,
				      this );
    if ( orient == Horizontal )
	return QSize( extent, 0 );
    else
	return QSize( 0, extent );
}

void QToolBarSeparator::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    QStyle::SFlags flags = QStyle::Style_Default;

    if ( orientation() == Horizontal )
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive( QStyle::PE_DockWindowSeparator, &p, rect(),
			   colorGroup(), flags );
}

#include "qtoolbar.moc"


/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a movable panel containing
  widgets such as tool buttons.

  \ingroup application
  \mainclass

  A toolbar is a panel that contains a set of controls, usually
  represented by small icons.  It's purpose is to provide quick access
  to frequently used commands or options. Within a QMainWindow the user
  can drag toolbars within and between the dock areas. Toolbars can also
  be dragged out of any dock area to float freely as top level windows.

  QToolBar is a specialization of QDockWindow, and so provides
  all the functionality of a QDockWindow.

  To use QToolBar you simply create a QToolBar as a child of a
  QMainWindow, create a number of QToolButton widgets (or other widgets)
  in left to right (or top to bottom) order and call addSeparator() when
  you want a separator. When a toolbar is floated the caption used is
  the label given in the constructor call. This can be changed with
  setLabel().

  \quotefile action/application.cpp
  \skipto new QToolBar
  \printuntil fileSaveAction

  This extract from the \l application/application.cpp example shows the
  creation of a new toolbar as a child of a QMainWindow and adding two
  QActions.

  You may use most widgets within a toolbar, with QToolButton
  and QComboBox being the most common.

  QToolBars, like QDockWindows, are located in QDockAreas or float as
  top level windows. QMainWindow provides four QDockAreas (top, left,
  right and bottom). When you create a new toolbar (as in the example
  above) as a child of a QMainWindow the toolbar will be added to the
  top dock area. You can move it to another dock area (or float it) by
  calling QMainWindow::moveDockWindow(). QDock areas lay out their
  windows in \link qdockarea.html#lines Lines\endlink.

  If the main window is resized so that the area occupied by the toolbar
  is too small to show all its widgets a little arrow button (which
  looks like a right-pointing chevron, '&#187;') will appear at the right or
  bottom of the toolbar depending on its orientation. Clicking this
  button pops up a menu that shows the 'overflowing' items.

  Usually a toolbar will get precisely the space it needs. However, with
  setHorizontalStretchable(), setVerticalStretchable() or
  setStretchableWidget() you can tell the main window to expand the
  toolbar to fill all available space in the specified orientation.

  The toolbar arranges its buttons either horizontally or vertically (see
  orientation() for details). Generally, QDockArea will set the
  orientation correctly for you, but you can set it yourself with
  setOrientation() and track any changes by connecting to the
  orientationChanged() signal.

  You can use the clear() method to remove all items from a toolbar.

  \sa QToolButton QMainWindow
  \link http://www.iarchitect.com/visual.htm Parts of Isys on Visual Design\endlink
  \link guibooks.html#fowler GUI Design Handbook: Tool Bar\endlink.
*/

/*!
    \fn QToolBar::QToolBar( const QString &label,
	      QMainWindow *, ToolBarDock = Top,
	      bool newLine = FALSE, const char * name = 0 );
    \obsolete
*/

/*!  Constructs an empty toolbar.

    The toolbar is a child of \a parent and is managed by \a parent. It
    is initially located in dock area \a dock and is labeled \a label.
    If \a newLine is TRUE the toolbar will be placed on a new line in
    the dock area.
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


/*!
    \overload

    Constructs an empty horizontal toolbar.

    The toolbar is a child of \a parent and is managed by \a mainWindow.
    The \a label and \a newLine parameters are passed straight to
    QMainWindow::addDockWindow(). \a name is the object name and \a f is
    the widget flags.

  Use this constructor if you want to create torn-off (undocked,
  floating) toolbars or toolbars in the status bar.
*/

QToolBar::QToolBar( const QString &label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QDockWindow( InDock, parent, name, f )
{
    mw = mainWindow;
    init();

    clearWFlags( WType_Dialog | WStyle_Customize | WStyle_NoBorder );
    reparent( parent, QPoint( 0, 0 ), FALSE );


    if ( mainWindow )
	mainWindow->addToolBar( this, label, QMainWindow::DockUnmanaged, newLine );
}


/*!
    \overload

    Constructs an empty toolbar, with parent \a parent and name \a name,
    in its \a parent's top dock area, without any label and without
    requiring a newline.
*/

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QDockWindow( InDock, parent, name )
{
    mw = parent;
    init();

    if ( parent )
	parent->addToolBar( this, QString::null, QMainWindow::DockTop );
}

/*!
    \internal

  Common initialization code. Requires that \c mw and \c o are set.
  Does not call QMainWindow::addDockWindow().
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
    setFrameStyle( QFrame::ToolBarPanel | QFrame::Raised);
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
    QObjectList *childs = queryList( "QToolBarSeparator" );
    if ( childs ) {
        QObject *ob = 0;
	for ( ob = childs->first(); ob; ob = childs->next() ) {
	    QToolBarSeparator* w = (QToolBarSeparator*)ob;
	    w->setOrientation( o );
        }
    }
    delete childs;
}

/*!  Adds a separator to the right/bottom of the toolbar. */

void QToolBar::addSeparator()
{
    (void) new QToolBarSeparator( orientation(), this, "toolbar separator" );
}

/*!\reimp
*/

void QToolBar::styleChange( QStyle& )
{
    QObjectList *childs = queryList( "QWidget" );
    if ( childs ) {
        QObject *ob = 0;
	for ( ob = childs->first(); ob; ob = childs->next() ) {
            if ( ob->inherits( "QToolButton" ) ) {
                QToolButton* w = (QToolButton*)ob;
                w->setStyle( &style() );
            } else if ( ob->inherits( "QToolBarSeparator" ) ) {
                QToolBarSeparator* w = (QToolBarSeparator*)ob;
                w->setStyle( &style() );
            }
        }
    }
    delete childs;
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

/*!  Returns a pointer to the QMainWindow which manages this toolbar.
*/

QMainWindow * QToolBar::mainWindow() const
{
    return mw;
}


/*! Sets the widget \a w to be expanded if this toolbar is requested to
 stretch.

    The request to stretch might occur because QMainWindow
    right-justifies the dock it's in, or because this toolbar's
    isVerticalStretchable() or isHorizontalStretchable() is set to TRUE.

  If you call this function and the toolbar is not yet
  stretchable, setStretchable() is called.

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
    } else if ( e->type() == QEvent::Show && d->firstShow ) {
	QWidget::layout()->invalidate();
	QWidget::layout()->activate();
	d->firstShow = FALSE;
    } else if ( e->type() == QEvent::LayoutHint && place() == OutsideDock ) {
	adjustSize();
    }
    return r;
}


/*!
  \property QToolBar::label
  \brief the label of the toolbar.

  If the toolbar is floated the label becomes the toolbar window's
  caption.
*/

void QToolBar::setLabel( const QString & label )
{
    l = label;
    setCaption( l );
}

QString QToolBar::label() const
{
    return l;
}


/*! Deletes all the toolbar's child widgets. */

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
			s = b->text();
		    int id;
		    if ( b->popup() && b->popupDelay() == 0 )
			id = d->extensionPopup->insertItem( b->iconSet(), s, b->popup() );
		    else
			id = d->extensionPopup->insertItem( b->iconSet(), s );

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

#ifndef QT_NO_PUSHBUTTON
    if ( d->button->inherits( "QPushButton" ) &&
	 ( (QPushButton*)d->button )->popup() ) {
	( (QPushButton*)d->button )->popup()->exec( QCursor::pos() );
    } else
#endif
    if ( d->button->inherits( "QToolButton" ) &&
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
    if ( d )
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

/*! \reimp */

void QToolBar::setMinimumSize( int, int )
{
}

/* from chaunsee:

1.  Tool Bars should contain only high-frequency functions.  Avoid putting
things like About and Exit on a tool bar unless they are frequent functions.

2.  All tool bar buttons must have some keyboard access method (it can be a
menu or shortcut key or a function in a dialog box that can be accessed
through the keyboard).

3.  Make tool bar functions as efficient as possible (the common example is to
Print in Microsoft applications, it doesn't bring up the Print dialog box, it
prints immediately to the default printer).

4.  Avoid turning tool bars into graphical menu bars.  To me, a tool bar should
be efficient. Once you make almost all the items in a tool bar into graphical
pull-down menus, you start to lose efficiency.

5.  Make sure that adjacent icons are distinctive. There are some tool bars
where you see a group of 4-5 icons that represent related functions, but they
are so similar that you can't differentiate among them.	 These tool bars are
often a poor attempt at a "common visual language".

6.  Use any de facto standard icons of your platform (for windows use the
cut, copy and paste icons provided in dev kits rather than designing your
own).

7.  Avoid putting a highly destructive tool bar button (delete database) by a
safe, high-frequency button (Find) -- this will yield 1-0ff errors).

8.  Tooltips in many Microsoft products simply reiterate the menu text even
when that is not explanatory.  Consider making your tooltips slightly more
verbose and explanatory than the corresponding menu item.

9.  Keep the tool bar as stable as possible when you click on different
objects. Consider disabling tool bar buttons if they are used in most, but not
all contexts.

10.  If you have multiple tool bars (like the Microsoft MMC snap-ins have),
put the most stable tool bar to at the left with less stable ones to the
right. This arrangement (stable to less stable) makes the tool bar somewhat
more predictable.

11.  Keep a single tool bar to fewer than 20 items divided into 4-7 groups of
items.
*/
#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Implementation of the QDockWindow class
**
** Created : 001010
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qdockwindow.h"
#include "qdockarea.h"
#include "qwidgetresizehandler.h"
#include "qtitlebar_p.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qtimer.h>

#if defined( Q_WS_MAC )
#define MAC_DRAG_HACK
#endif

static const char * close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

class QDockWindowResizeHandle : public QWidget
{
public:
    QDockWindowResizeHandle( Qt::Orientation o, QWidget *parent, QDockWindow *w, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    void startLineDraw();
    void endLineDraw();
    void drawLine( const QPoint &globalPos );

private:
    Qt::Orientation orient;
    bool mousePressed;
    QPainter *unclippedPainter;
    QPoint lastPos, firstPos;
    QDockWindow *dockWindow;

};

QDockWindowResizeHandle::QDockWindowResizeHandle( Qt::Orientation o, QWidget *parent,
						  QDockWindow *w, const char * )
    : QWidget( parent, "qt_dockwidget_internal" ), mousePressed( FALSE ), unclippedPainter( 0 ), dockWindow( w )
{
    setOrientation( o );
}

QSize QDockWindowResizeHandle::sizeHint() const
{
    int sw = 2 * style().splitterWidth() / 3;
    return QSize(sw,sw).expandedTo( QApplication::globalStrut() );
}

void QDockWindowResizeHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
    if ( o == QDockArea::Horizontal ) {
#ifndef QT_NO_CURSOR
	setCursor( splitVCursor );
#endif
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
#ifndef QT_NO_CURSOR
	setCursor( splitHCursor );
#endif
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    }
}
void QDockWindowResizeHandle::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    mousePressed = TRUE;
    if ( !dockWindow->opaqueMoving() )
	startLineDraw();
    lastPos = firstPos = e->globalPos();
    if ( !dockWindow->opaqueMoving() )
	drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    if ( !dockWindow->opaqueMoving() ) {
	if ( orientation() != dockWindow->area()->orientation() ) {
	    if ( orientation() == Horizontal ) {
		int minpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).y();
		int maxpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).y() + dockWindow->area()->height();
		if ( e->globalPos().y() < minpos || e->globalPos().y() > maxpos )
		    return;
	    } else {
		int minpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).x();
		int maxpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).x() + dockWindow->area()->width();
		if ( e->globalPos().x() < minpos || e->globalPos().x() > maxpos )
		    return;
	    }
	} else {
	    QWidget *w = dockWindow->area()->topLevelWidget();
	    if ( w ) {
		if ( orientation() == Horizontal ) {
		    int minpos = w->mapToGlobal( QPoint( 0, 0 ) ).y();
		    int maxpos = w->mapToGlobal( QPoint( 0, 0 ) ).y() + w->height();
		    if ( e->globalPos().y() < minpos || e->globalPos().y() > maxpos )
			return;
		} else {
		    int minpos = w->mapToGlobal( QPoint( 0, 0 ) ).x();
		    int maxpos = w->mapToGlobal( QPoint( 0, 0 ) ).x() + w->width();
		    if ( e->globalPos().x() < minpos || e->globalPos().x() > maxpos )
			return;
		}
	    }
	}
    }

    if ( !dockWindow->opaqueMoving() )
	drawLine( lastPos );
    lastPos = e->globalPos();
    if ( dockWindow->opaqueMoving() ) {
	mouseReleaseEvent( e );
	mousePressed = TRUE;
	firstPos = e->globalPos();
    }
    if ( !dockWindow->opaqueMoving() )
	drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	if ( !dockWindow->opaqueMoving() ) {
	    drawLine( lastPos );
	    endLineDraw();
	}
	if ( orientation() != dockWindow->area()->orientation() )
	    dockWindow->area()->invalidNextOffset( dockWindow );
	if ( orientation() == Horizontal ) {
	    int dy;
	    if ( dockWindow->area()->handlePosition() == QDockArea::Normal || orientation() != dockWindow->area()->orientation() )
		dy = e->globalPos().y() - firstPos.y();
	    else
		dy =  firstPos.y() - e->globalPos().y();
	    int d = dockWindow->height() + dy;
	    if ( orientation() != dockWindow->area()->orientation() ) {
		dockWindow->setFixedExtentHeight( -1 );
		d = QMAX( d, dockWindow->minimumHeight() );
		int ms = dockWindow->area()->maxSpace( d, dockWindow );
		d = QMIN( d, ms );
		dockWindow->setFixedExtentHeight( d );
	    } else {
		dockWindow->area()->setFixedExtent( d, dockWindow );
	    }
	} else {
	    int dx;
	    if ( dockWindow->area()->handlePosition() == QDockArea::Normal || orientation() != dockWindow->area()->orientation() )
		dx = e->globalPos().x() - firstPos.x();
	    else
		dx = firstPos.x() - e->globalPos().x();
	    int d = dockWindow->width() + dx;
	    if ( orientation() != dockWindow->area()->orientation() ) {
		dockWindow->setFixedExtentWidth( -1 );
		d = QMAX( d, dockWindow->minimumWidth() );
		int ms = dockWindow->area()->maxSpace( d, dockWindow );
		d = QMIN( d, ms );
		dockWindow->setFixedExtentWidth( d );
	    } else {
		dockWindow->area()->setFixedExtent( d, dockWindow );
	    }
	}
    }

    QApplication::postEvent( dockWindow->area(), new QEvent( QEvent::LayoutHint ) );
    mousePressed = FALSE;
}

void QDockWindowResizeHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawSplitter( &p, 0, 0, width(), height(), colorGroup(), orientation() == Horizontal ? Vertical : Horizontal );
}

void QDockWindowResizeHandle::startLineDraw()
{
    if ( unclippedPainter )
	endLineDraw();
#ifdef MAC_DRAG_HACK
    QWidget *paint_on = topLevelWidget();
#else
    QWidget *paint_on = QApplication::desktop();
#endif
    unclippedPainter = new QPainter( paint_on, TRUE );
    unclippedPainter->setPen( QPen( gray, orientation() == Horizontal ? height() : width() ) );
    unclippedPainter->setRasterOp( XorROP );
}

void QDockWindowResizeHandle::endLineDraw()
{
    if ( !unclippedPainter )
	return;
    delete unclippedPainter;
    unclippedPainter = 0;
}

void QDockWindowResizeHandle::drawLine( const QPoint &globalPos )
{
#ifdef MAC_DRAG_HACK
    QPoint start = mapTo(topLevelWidget(), QPoint(0, 0));
    QPoint starta = dockWindow->area()->mapTo(topLevelWidget(), QPoint(0, 0));
    QPoint end = globalPos - topLevelWidget()->pos();
#else
    QPoint start = mapToGlobal( QPoint( 0, 0 ) );
    QPoint starta = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) );
    QPoint end = globalPos;
#endif

    if ( orientation() == Horizontal ) {
	if ( orientation() == dockWindow->orientation() )
	    unclippedPainter->drawLine( starta.x() , end.y(), starta.x() + dockWindow->area()->width(), end.y() );
	else
	    unclippedPainter->drawLine( start.x(), end.y(), start.x() + width(), end.y() );
    } else {
	if ( orientation() == dockWindow->orientation() )
	    unclippedPainter->drawLine( end.x(), starta.y(), end.x(), starta.y() + dockWindow->area()->height() );
	else
	    unclippedPainter->drawLine( end.x(), start.y(), end.x(), start.y() + height() );
    }
}




static QPoint realWidgetPos( QWidget *w )
{
    if ( !w->parentWidget() || !w->parentWidget()->inherits( "QDockArea" ) )
	return w->pos();
    return w->parentWidget()->mapToGlobal( w->geometry().topLeft() );
}

class QDockWindowHandle : public QWidget
{
    Q_OBJECT
    friend class QDockWindow;

public:
    QDockWindowHandle( QDockWindow *dw );
    void updateGui();

    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }
    QSizePolicy sizePolicy() const;
    void setOpaqueMoving( bool b ) { opaque = b; }

signals:
    void doubleClicked();

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );

private slots:
    void minimize();

private:
    QDockWindow *dockWindow;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;
    QTimer *timer;
    bool opaque;

};

QDockWindowHandle::QDockWindowHandle( QDockWindow *dw )
    : QWidget( dw, "qt_dockwidget_internal" ), dockWindow( dw ),
      mousePressed( FALSE ), closeButton( 0 ), opaque( FALSE )
{
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( minimize() ) );
}

void QDockWindowHandle::paintEvent( QPaintEvent *e )
{
    if ( !dockWindow->dockArea )
	return;
    QPainter p( this );
    if ( !dockWindow->area() || !dockWindow->isCloseEnabled() ) {
	style().drawToolBarHandle( &p, QRect( 0, 0, width(), height() ),
				   dockWindow->orientation(), FALSE, colorGroup() );
    } else {
	if ( dockWindow->area()->orientation() == Horizontal ) {
	    int offs = 0;
	    if ( dockWindow->isCloseEnabled() && style() != WindowsStyle )
		offs += 2;
	    style().drawToolBarHandle( &p, QRect( offs, 15, width() - offs, height() - 15 ),
				       dockWindow->orientation(), FALSE, colorGroup() );
	} else {
	    int offs = 1;
	    if ( dockWindow->isCloseEnabled() && style() != WindowsStyle )
		offs++;
	    style().drawToolBarHandle( &p, QRect( 0, offs, width() - 15, height() - offs ),
				       dockWindow->orientation(), FALSE, colorGroup() );
	}
    }

    QWidget::paintEvent( e );
}

void QDockWindowHandle::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    hadDblClick = FALSE;
    mousePressed = TRUE;
    offset = e->pos();
    dockWindow->startRectDraw( e->pos(), !opaque );
    if ( !opaque )
	qApp->installEventFilter( dockWindow );
}

void QDockWindowHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed || e->pos() == offset )
	return;
    dockWindow->handleMove( e->globalPos() - offset, e->globalPos(), !opaque );
    if ( opaque )
	dockWindow->updatePosition( e->globalPos() );
}

void QDockWindowHandle::mouseReleaseEvent( QMouseEvent *e )
{
    qApp->removeEventFilter( dockWindow );
    if ( !mousePressed )
	return;
    dockWindow->endRectDraw( !opaque );
    mousePressed = FALSE;
    if ( !hadDblClick && offset == e->pos() ) {
	timer->start( QApplication::doubleClickInterval(), TRUE );
    } else if ( !hadDblClick ) {
	dockWindow->updatePosition( e->globalPos() );
    }
}

void QDockWindowHandle::minimize()
{
    if ( dockWindow->area() && dockWindow->area()->parentWidget() &&
	 dockWindow->area()->parentWidget()->inherits( "QMainWindow" ) ) {
	QMainWindow *mw = (QMainWindow*)dockWindow->area()->parentWidget();
	if ( mw->isDockEnabled( dockWindow, Qt::Minimized ) )
	    mw->moveDockWindow( dockWindow, Qt::Minimized );
    }
}

void QDockWindowHandle::resizeEvent( QResizeEvent * )
{
    updateGui();
}

void QDockWindowHandle::updateGui()
{
    if ( !closeButton ) {
	closeButton = new QToolButton( this );
	closeButton->setPixmap( close_xpm );
	closeButton->setFixedSize( 12, 12 );
	connect( closeButton, SIGNAL( clicked() ),
		 dockWindow, SLOT( hide() ) );
    }

    if ( dockWindow->isCloseEnabled() && dockWindow->area() )
	closeButton->show();
    else
	closeButton->hide();

    if ( !dockWindow->area() )
	return;

    if ( dockWindow->area()->orientation() == Horizontal )
	closeButton->move( 2, 2 );
    else
	closeButton->move( width() - closeButton->width() - 2, 2 );
}

QSize QDockWindowHandle::minimumSizeHint() const
{
    if ( dockWindow->orientation() == Horizontal )
	return QSize( dockWindow->isCloseEnabled() ? 17 : 12, 0 );
    return QSize( 0, dockWindow->isCloseEnabled() ? 17 : 12 );
}

QSizePolicy QDockWindowHandle::sizePolicy() const
{
    if ( dockWindow->orientation() != Horizontal )
	return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
}

void QDockWindowHandle::mouseDoubleClickEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    timer->stop();
    emit doubleClicked();
    hadDblClick = TRUE;
}

class QDockWindowTitleBar : public QTitleBarLabel
{
    Q_OBJECT
    friend class QDockWindow;

public:
    QDockWindowTitleBar( QDockWindow *dw );
    void updateGui();
    void setOpaqueMoving( bool b ) { opaque = b; }

protected:
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );

signals:
    void doubleClicked();

private:
    QDockWindow *dockWindow;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;
    bool opaque;

};

QDockWindowTitleBar::QDockWindowTitleBar( QDockWindow *dw )
    : QTitleBarLabel( dw, "qt_dockwidget_internal" ), dockWindow( dw ), mousePressed( FALSE ),
      closeButton( 0 ), opaque( FALSE )
{
    setMouseTracking( TRUE );
    setMinimumHeight( 13 );
}

void QDockWindowTitleBar::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    if ( e->y() < 3 )
	return;
    e->accept();
    mousePressed = TRUE;
    hadDblClick = FALSE;
    offset = e->pos();
    dockWindow->startRectDraw( e->pos(), !opaque );
    if ( !opaque )
	qApp->installEventFilter( dockWindow );
}

void QDockWindowTitleBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    e->accept();
    dockWindow->handleMove( e->globalPos() - offset, e->globalPos(), !opaque );
    if ( opaque )
	dockWindow->updatePosition( e->globalPos() );
}

void QDockWindowTitleBar::mouseReleaseEvent( QMouseEvent *e )
{
    qApp->removeEventFilter( dockWindow );
    if ( !mousePressed )
	return;
    dockWindow->endRectDraw( !opaque );
    mousePressed = FALSE;
    if ( !hadDblClick )
	dockWindow->updatePosition( e->globalPos() );
}

void QDockWindowTitleBar::resizeEvent( QResizeEvent *e )
{
    updateGui();
    QTitleBarLabel::resizeEvent( e );
}

void QDockWindowTitleBar::updateGui()
{
    if ( !closeButton ) {
	closeButton = new QToolButton( this );
	closeButton->setPixmap( close_xpm );
	closeButton->setFixedSize( 12, 12 );
	connect( closeButton, SIGNAL( clicked() ),
		 dockWindow, SLOT( hide() ) );
    }

    if ( dockWindow->isCloseEnabled() )
	closeButton->show();
    else
	closeButton->hide();

    closeButton->move( width() - closeButton->width() - 1, 1 );
}

void QDockWindowTitleBar::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
    hadDblClick = TRUE;
}

/*! \class QDockWindow qdockwindow.h

  \brief The QDockWindow class provides a widget which can be docked
  inside a QDockArea or floated as a top level window on the desktop.

  \ingroup application

    This class handles moving, resizing, docking and undocking dock
    windows. QToolBar is a subclass of QDockWindow so the functionality
    provided for dock windows is available with the same API for
    toolbars.

    If the user drags the dock window into the dock area the dock window
    will be docked. If the user drags the dock area outside any dock
    areas the dock window will be undocked (floated) and will become a
    top level window. Double clicking a floating dock window's titlebar
    will dock the dock window to the last dock area it was docked in.
    Double clicking a docked dock window's handle will undock (float)
    the dock window. Single clicking a docked dock window's handle will
    minimize the dock window (only its handle will appear, below the
    menu bar). Single clicking the minimized handle will restore the
    dock window to the last dock area that it was docked in. If the user
    clicks the close button (which by default, appears on floating dock
    windows) the dock window will disappear. In QMainWindow a
    right-click popup menu (the dock window menu) is available which
    lists dock windows and can be used to show or hide them. You can
    control whether or not a dock window has a close button with
    setCloseMode(). When a dock window's visibility changes, i.e. it is
    shown or hidden, the visibilityChanged() signal is emitted.

    QMainWindow provides four dock areas (top, left, right and bottom)
    which can be used by dock windows. For most applications using the
    dock areas provided by QMainWindow will be sufficient. (See the \l
    QDockArea documentation if you want to create your own dock areas.)

    When you construct a dock window you \e must pass it a QDockArea or
    a QMainWindow as its parent if you want it docked. Pass 0 for the
    parent if you want it floated.

    \code
    QToolBar *fileTools = new QToolBar( this, "File Actions" );
    moveDockWindow( fileTools, Left );
    \endcode

    In the example above we create a new QToolBar in the constructor of
    a QMainWindow subclass (so that the \e this pointer points to the
    QMainWindow). By default the toolbar will be added to the \c Top
    dock area, but we've moved it to the \c Left dock area.

    A dock window is often used to contain a single widget. In these
    cases the widget can be set by calling setWidget(). If you're
    constructing a dock window that contains multiple widgets, e.g. a
    toolbar, arrange the widgets within a box layout inside the dock
    window. To do this use the boxLayout() function to get a pointer to
    the dock window's box layout, then add widgets to the layout using
    the box layout's QBoxLayout::addWidget() function. The dock window
    will dynamically set the orientation of the layout to be vertical
    or horizontal as necessary, although you can control this yourself
    with setOrientation().

    Although a common use of dock windows is for toolbars, they can be
    used with any widgets. (See the <i>Qt Designer</i> and <i>Qt
    Linguist</i> applications, for example.) When using larger widgets
    it may make sense for the dock window to be resizable by calling
    setResizeEnabled(). Resizable dock windows are given splitter-like
    handles to allow the user to resize them within their dock area.
    When resizable dock windows are undocked they become top level
    windows and can be resized like any other top level windows, e.g. by
    dragging a corner or edge.

    Dock windows can be docked and undocked programatically using dock()
    and undock(). A dock window's orientation can be set with
    setOrientation(). You can also use QDockArea::moveDockWindow() and
    QDockArea::removeDockWindow(). If you're using a QMainWindow,
    QMainWindow::moveDockWindow() and QMainWindow::removeDockWindow()
    are available.

    A dock window can have some preferred settings, for example, you can
    set a
    preferred offset from the left edge (or top edge for vertical dock
    areas) of the dock area using setOffset(). If you'd prefer a dock
    window to start on a new line when it is docked use setNewLine().
    The setFixedExtentWidth() and setFixedExtentHeight()
    functions can be used to define the dock window's preferred size,
    and the setHorizontalStretchable() and setVerticalStretchable()
    functions set whether the dock window can be stretched or not.
    Dock windows can be moved by default, but this can be changed with
    setMovingEnabled(). When a dock window is moved it is shown as a
    rectangular outline, but it can be shown normally using
    setOpaqueMoving().

*/

/*!
  \enum QDockWindow::Place

  This enum specifies the possible locations for a QDockWindow:

  \value InDock  inside a QDockArea.
  \value OutsideDock  floating as a top level window on the desktop.
*/

/*!
  \enum QDockWindow::CloseMode

  This enum type specifies when (if ever) a dock window has a close
  button.

  \value Never  The dock window never has a close button and cannot be
  closed by the user.
  \value Docked  The dock window has a close button only when docked.
  \value Undocked  The dock window has a close button only when floating.
  \value Always The dock window always has a close button.

  Note that dock windows can always be minimized if the user clicks
  their dock window handle when they are docked.
*/

/*! \fn void QDockWindow::orientationChanged( Orientation o )

  This signal is emitted when the orientation of the dock window is
  changed. The new orientation is \a o.
*/

/*! \fn void QDockWindow::placeChanged( QDockWindow::Place p )

  This signal is emitted when the dock window is docked (\a p is \c
  InDock) or undocked (\a p is \c OutsideDock).

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/

/*! \fn void QDockWindow::visibilityChanged( bool visible )

  This signal is emitted if the visibility of the dock window is
  changed. If \a visible is TRUE, the QDockWindow is now visible,
  otherwise it has been hidden.

  A dock window can be hidden if it has a close button which the user
  has clicked. In the case of a QMainWindow a dock window can have its
  visibility changed (hidden or shown) by clicking its name in the
  dock window menu that lists the QMainWindow's dock windows.
*/

/*! \fn QDockArea *QDockWindow::area() const

  Returns the QDockArea in which this dock window is docked,
  or 0 if the dock window is floating.
*/

/*! \fn QDockWindow::Place QDockWindow::place() const

  Returns the current place of the dock window. This is either \c InDock
  or \c OutsideDock.

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/


/*! Constructs a QDockWindow.

    If \a p is \c InDock, the dock window is docked into a QDockArea and
    \a parent \e must be a QDockArea or a QMainWindow. If the \a parent
    is a QMainWindow the dock window will be docked in the main window's
    \c Top dock area.

    If \a p is \c OutsideDock, the parent \e must be 0 and the dock
    window is created as a floating window.

    We recommend creating the dock area \c InDock with a QMainWindow as
    parent then calling QMainWindow::moveDockWindow() to move the dock
    window where you want it.
*/

QDockWindow::QDockWindow( Place p, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | ( p == OutsideDock ? (WType_Dialog | WStyle_Customize | WStyle_NoBorder) : 0 ) ),
      curPlace( p ), wid( 0 ), unclippedPainter( 0 ), dockArea( 0 ), tmpDockArea( 0 ), resizeEnabled( FALSE ),
      moveEnabled( TRUE ), cMode( Never ), offs( 0 ), fExtent( -1, -1 ), nl( FALSE ), dockWindowData( 0 ),
      lastPos( -1, -1 ), opaque( FALSE )
{
    widgetResizeHandler = new QWidgetResizeHandler( this );
    widgetResizeHandler->setMovingEnabled( FALSE );

    hbox = new QVBoxLayout( this );
    hbox->setMargin( isResizeEnabled() || p == OutsideDock ? 2 : 0 );
    hbox->setSpacing( 1 );
    titleBar = new QDockWindowTitleBar( this );
    horHandle = new QDockWindowHandle( this );
    hHandleTop = new QDockWindowResizeHandle( Qt::Horizontal, this, this, "horz. handle" );
    hbox->addWidget( titleBar );
    hbox->addWidget( horHandle );
    hbox->addWidget( hHandleTop );
    vbox = new QHBoxLayout( hbox );
    verHandle = new QDockWindowHandle( this );
    vHandleLeft = new QDockWindowResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    vbox->addWidget( verHandle );
    vbox->addWidget( vHandleLeft );
    layout = new QBoxLayout( vbox, QBoxLayout::LeftToRight );
    vHandleRight = new QDockWindowResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    vbox->addWidget( vHandleRight );
    hHandleBottom = new QDockWindowResizeHandle( Qt::Horizontal, this, this, "horz. handle" );
    hbox->addWidget( hHandleBottom );
    hHandleBottom->hide();
    vHandleRight->hide();
    hHandleTop->hide();
    vHandleLeft->hide();
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );
    updateGui();
    stretchable[ Horizontal ] = FALSE;
    stretchable[ Vertical ] = FALSE;

    connect( titleBar, SIGNAL( doubleClicked() ), this, SLOT( dock() ) );
    connect( verHandle, SIGNAL( doubleClicked() ), this, SLOT( undock() ) );
    connect( horHandle, SIGNAL( doubleClicked() ), this, SLOT( undock() ) );
    connect( this, SIGNAL( orientationChanged( Orientation ) ),
	     this, SLOT( setOrientation( Orientation ) ) );

    if ( parent && parent->inherits( "QDockArea" ) && p == InDock ) {
	( (QDockArea*)parent )->moveDockWindow( this );
    } else if ( p == InDock ) {
	p = OutsideDock;
    }
}

/*! Sets the orientation of the dock window to \a o. The orientation is
  propagated to the layout boxLayout().
*/

void QDockWindow::setOrientation( Orientation o )
{
    boxLayout()->setDirection( o == Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    QEvent *e = new QEvent( QEvent::LayoutHint );
    QApplication::postEvent( this, e );
}

/*! \reimp 

    Destroys the dock window and its child widgets.
*/

QDockWindow::~QDockWindow()
{
    qApp->removeEventFilter( this );
    if ( area() )
	area()->removeDockWindow( this, FALSE, FALSE );
    if ( area()  && area()->parentWidget() &&
	 area()->parentWidget()->inherits( "QMainWindow" ) )
	( (QMainWindow*)area()->parentWidget() )->removeDockWindow( this );

    delete (QDockArea::DockWindowData*)dockWindowData;
}

/*!  \reimp
*/

void QDockWindow::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    updateGui();
}


void QDockWindow::swapRect( QRect &r, Qt::Orientation o, const QPoint &offset, QDockArea *area )
{
    if ( !area ) {
	r.setSize( size() );
	return;
    }

    QBoxLayout *bl = boxLayout()->createTmpCopy();
    bl->setDirection( o == Vertical ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
    bl->activate();
    r.setSize( bl->sizeHint() );
    bl->data = 0;
    delete bl;
    if ( o == Qt::Horizontal )
	r.moveBy( -r.width() / 2, 0 );
    else
	r.moveBy( 0, -r.height() / 2 );
    r.moveBy( offset.x(), offset.y() );
}

QWidget *QDockWindow::areaAt( const QPoint &gp )
{
    QWidget *w = qApp->widgetAt( gp, TRUE );
    QMainWindow *mw = 0;
    while ( w ) {
	if ( w->inherits( "QDockArea" ) ) {
	    QDockArea *a = (QDockArea*)w;
	    if ( a->isDockWindowAccepted( this ) )
		return w;
	}
	if ( w->inherits( "QMainWindow" ) ) {
	    mw = (QMainWindow*)w;
	    QDockArea *a = mw->dockingArea( mw->mapFromGlobal( gp ) );
	    if ( a && a->isDockWindowAccepted( this ) )
		return a;
	}
	w = w->parentWidget( TRUE );
    }
    return 0;
}

void QDockWindow::handleMove( const QPoint &pos, const QPoint &gp, bool drawRect )
{
    if ( !unclippedPainter )
	return;

    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = areaAt( gp );
    QPoint offset( mapFromGlobal( pos ) );
    currRect.moveBy( offset.x(), offset.y() );
    if ( !w || !w->inherits( "QDockArea" ) ) {
	if ( startOrientation != Horizontal )
	    swapRect( currRect, Horizontal, startOffset, (QDockArea*)w );
	if ( drawRect ) {
	    unclippedPainter->setPen( QPen( gray, 3 ) );
	    QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	    dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
	}
	state = OutsideDock;
	return;
    }

    state = InDock;
    QDockArea *area = (QDockArea*)w;
    if ( startOrientation != ( area ? area->orientation() : Horizontal ) )
	    swapRect( currRect, orientation(), startOffset, area );
    if ( drawRect ) {
	unclippedPainter->setPen( QPen( gray, 1 ) );
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    tmpDockArea = area;
}

void QDockWindow::updateGui()
{
    if ( curPlace == OutsideDock ) {
	hbox->setMargin( 2 );
 	horHandle->hide();
 	verHandle->hide();
	if ( moveEnabled )
	    titleBar->show();
	else
	    titleBar->hide();
	titleBar->updateGui();
	hHandleTop->hide();
	vHandleLeft->hide();
	hHandleBottom->hide();
	vHandleRight->hide();
	setLineWidth( 2 );
	widgetResizeHandler->setActive( isResizeEnabled() );
	widgetResizeHandler->setExtraHeight( titleBar->height() );
    } else {
	hbox->setMargin( isResizeEnabled() ? 0 : 2 );
	titleBar->hide();
	if ( orientation() == Horizontal ) {
	    horHandle->hide();
	    if ( moveEnabled )
		verHandle->show();
	    else
		verHandle->hide();
	    verHandle->updateGui();
	} else {
	    if ( moveEnabled )
		horHandle->show();
	    else
		horHandle->hide();
	    horHandle->updateGui();
	    verHandle->hide();
	}
	if ( isResizeEnabled() ) {
	    if ( orientation() == Horizontal ) {
		hHandleBottom->raise();
		hHandleTop->raise();
	    } else {
		vHandleRight->raise();
		vHandleLeft->raise();
	    }

	    if ( area() ) {
		if ( orientation() == Horizontal ) {
		    if ( area()->handlePosition() == QDockArea::Normal ) {
			hHandleBottom->show();
			hHandleTop->hide();
		    } else {
			hHandleTop->show();
			hHandleBottom->hide();
		    }
		    if ( !area()->isLastDockWindow( this ) )
			vHandleRight->show();
		    else
			vHandleRight->hide();
		    vHandleLeft->hide();
		} else {
		    if ( area()->handlePosition() == QDockArea::Normal ) {
			vHandleRight->show();
			vHandleLeft->hide();
		    } else {
			vHandleLeft->show();
			vHandleRight->hide();
		    }
		    if ( !area()->isLastDockWindow( this ) )
			hHandleBottom->show();
		    else
			hHandleBottom->hide();
		    hHandleTop->hide();
		}
	    }
	}
	if ( moveEnabled )
	    setLineWidth( 1 );
	else
	    setLineWidth( 0 );
	widgetResizeHandler->setActive( FALSE );
    }
}

void QDockWindow::updatePosition( const QPoint &globalPos )
{
    bool doAdjustSize = curPlace != state && state == OutsideDock;
    bool doUpdate = TRUE;
    bool doOrientationChange = TRUE;
    if ( state != curPlace && state == InDock ) {
	doUpdate = FALSE;
	curPlace = state;
	emit placeChanged( curPlace );
	updateGui();
	QApplication::sendPostedEvents();
    }
    Orientation oo = orientation();

    if ( state == InDock ) {
	if ( tmpDockArea ) {
	    bool differentDocks = FALSE;
	    if ( dockArea && dockArea != tmpDockArea ) {
		differentDocks = TRUE;
		delete (QDockArea::DockWindowData*)dockWindowData;
		dockWindowData = dockArea->dockWindowData( this );
		dockArea->removeDockWindow( this, FALSE, FALSE );
	    }
	    dockArea = tmpDockArea;
	    if ( differentDocks ) {
		if ( doUpdate ) {
		    doUpdate = FALSE;
		    curPlace = state;
		    emit placeChanged( curPlace );
		    updateGui();
		}
		emit orientationChanged( tmpDockArea->orientation() );
		doOrientationChange = FALSE;
	    } else {
		updateGui();
	    }
	    dockArea->moveDockWindow( this, globalPos, currRect, startOrientation != oo );
	}
    } else {
	if ( dockArea ) {
	    QMainWindow *mw = (QMainWindow*)dockArea->parentWidget();
	    if ( mw && mw->inherits( "QMainWindow" ) &&
		 ( !mw->isDockEnabled( QMainWindow::TornOff ) || !mw->isDockEnabled( this, QMainWindow::TornOff ) ) )
		return;
	    delete (QDockArea::DockWindowData*)dockWindowData;
	    dockWindowData = dockArea->dockWindowData( this );
	    dockArea->removeDockWindow( this, TRUE, startOrientation != Horizontal );
	}
	dockArea = 0;
	move( currRect.topLeft() );
    }
    if ( doUpdate ) {
	curPlace = state;
	emit placeChanged( curPlace );
	updateGui();
    }
    if ( doOrientationChange )
	emit orientationChanged( orientation() );
    tmpDockArea = 0;
    if ( doAdjustSize ) {
	QApplication::sendPostedEvents( this, QEvent::LayoutHint );
	if ( inherits( "QToolBar" ) )
	    adjustSize();
	show();
    }
}

/*! Sets the dock window's main widget.

  \sa boxLayout()
*/

void QDockWindow::setWidget( QWidget *w )
{
    wid = w;
    boxLayout()->addWidget( w );
    updateGui();
}

/*!  Returns the dock window's main widget.

  \sa setWidget()
*/

QWidget *QDockWindow::widget() const
{
    return wid;
}

void QDockWindow::startRectDraw( const QPoint &so, bool drawRect )
{
    state = place();
    if ( unclippedPainter )
	endRectDraw( !opaque );
#ifdef MAC_DRAG_HACK
    QWidget *paint_on = topLevelWidget();
#else
    QWidget *paint_on = QApplication::desktop();
#endif
    unclippedPainter = new QPainter( paint_on, TRUE );
    unclippedPainter->setPen( QPen( gray, 3 ) );
    unclippedPainter->setRasterOp( XorROP );
    currRect = QRect( realWidgetPos( this ), size() );
    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    startRect = currRect;
    startOrientation = orientation();
    startOffset = so;
}

void QDockWindow::endRectDraw( bool drawRect )
{
    if ( !unclippedPainter )
	return;
    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    delete unclippedPainter;
    unclippedPainter = 0;
}

/*!

    If \a b is TRUE, the dock window becomes resizeable. If \a b is
    FALSE the dock window cannot be resized.

    A resizeable QDockWindow can be resized using splitter-like handles
    inside a QDockArea and like every other top level window when
    floating.
*/

void QDockWindow::setResizeEnabled( bool b )
{
    resizeEnabled = b;
    hbox->setMargin( b ? 0 : 2 );
    updateGui();
}

/*! If \a b is TRUE, the user can move the dock window within the
  QDockArea, move the dock window to another dock area, or float the
  dock window. If \a b is FALSE the dock window cannot be moved at all.

  \sa isMovingEnabled() dock() undock() QDockArea::moveDockWindow() QMainWindow::moveDockWindow()
*/

void QDockWindow::setMovingEnabled( bool b )
{
    moveEnabled = b;
    updateGui();
}

/*! Returns TRUE if the dock window is resizeable, otherwise returns
 FALSE.

  \sa setResizeEnabled()
*/

bool QDockWindow::isResizeEnabled() const
{
    return resizeEnabled;
}

/*! Returns TRUE if the dock window is movable
 otherwise returns FALSE. A movable dock window can be moved \e within a
 dock area, to another dock area, or floated.

  \sa setMovingEnabled()
*/

bool QDockWindow::isMovingEnabled() const
{
    return moveEnabled;
}

/*!
    Sets when (if ever) the dock window has a close button. The choices
    are \c Never, \c Docked (i.e. only when docked), \c Undocked (only
    when undocked - floated) or \c Always.

    \sa closeMode()
*/

void QDockWindow::setCloseMode( int m )
{
    cMode = m;
    if ( place() == InDock ) {
	horHandle->updateGui();
	verHandle->updateGui();
    } else {
	titleBar->updateGui();
    }
}

/*! Returns TRUE if the dock window has a close button, otherwise
    returns FALSE. The result depends on the dock window's \l Place and
    its \l CloseMode.

  \sa setCloseMode()
*/

bool QDockWindow::isCloseEnabled() const
{
    return  ( ( cMode & Docked ) == Docked && place() == InDock ||
	      ( cMode & Undocked ) == Undocked && place() == OutsideDock );
}

/*! Returns the dock window's CloseMode.

  \sa setCloseMode()
*/

int QDockWindow::closeMode() const
{
    return cMode;
}

/*!
    If \a b is TRUE the dock window will be horizontally stretchable.
    If \a b is FALSE the dock window will not be horizontally
    stretchable..

    \sa setVerticalStretchable()
*/

void QDockWindow::setHorizontalStretchable( bool b )
{
    stretchable[ Horizontal ] = b;
}

/*!
    If \a b is TRUE the dock window will be vertically stretchable.
    If \a b is FALSE the dock window will not be vertically stretchable.

    \sa setHorizontalStretchable()
*/

void QDockWindow::setVerticalStretchable( bool b )
{
    stretchable[ Vertical ] = b;
}

/*!  Returns TRUE if the dock window is horizontally stretchable,
    otherwise returns FALSE.

  \sa setHorizontalStretchable()
 */

bool QDockWindow::isHorizontalStretchable() const
{
    return isResizeEnabled() || stretchable[ Horizontal ];
}

/*!  Returns TRUE if the dock window is vertically stretchable,
    otherwise returns FALSE.

  \sa setVerticalStretchable()
 */

bool QDockWindow::isVerticalStretchable() const
{
    return isResizeEnabled() || stretchable[ Vertical ];
}

/*! Returns TRUE if the dock window is stretchable in the current
  orientation(), otherwise returns FALSE.

  \sa isVerticalStretchable() isHorizontalStretchable()
*/

bool QDockWindow::isStretchable() const
{
    if ( orientation() == Horizontal )
	return isHorizontalStretchable();
    return isVerticalStretchable();
}

/*! Returns the orientation of the dock window.

  \sa orientationChanged()
*/

Qt::Orientation QDockWindow::orientation() const
{
    if ( !dockArea || dockArea->orientation() == Horizontal )
	return Horizontal;
    return Vertical;
}

/*! Returns the dock window's preferred offset from the dock area's left
    edge (top edge for vertical dock areas).

    \sa setOffset()
*/

int QDockWindow::offset() const
{
    return offs;
}

/*!
    Sets the dock window's preferred offset from the dock area's left
    edge (top edge for vertical dock areas) to \a o.

    \sa offset()
*/

void QDockWindow::setOffset( int o )
{
    offs = o;
}

/*! Returns dock window's preferred size (fixed extent).

    \sa setFixedExtent()
*/

QSize QDockWindow::fixedExtent() const
{
    return fExtent;
}

/*! Sets the dock window's preferred width for its fixed extent (size)
    to \a w.

    \sa setFixedExtentHeight()
*/

void QDockWindow::setFixedExtentWidth( int w )
{
    fExtent.setWidth( w );
}

/*!
     Sets the dock window's preferred height for its fixed extent (size)
     to \a h.

    \sa setFixedExtentWidth()
*/

void QDockWindow::setFixedExtentHeight( int h )
{
    fExtent.setHeight( h );
}

/*!
    If \a b is TRUE the dock window prefers to start a new line in the
    dock area. If \a b is FALSE the dock window does not care whether it
    starts a new line or not.

    \sa newLine()
*/

void QDockWindow::setNewLine( bool b )
{
    nl = b;
}

/*! Returns TRUE if the dock window prefers to start a new line in a
  QDockArea, otherwise returns FALSE.

  \sa setNewLine()
*/

bool QDockWindow::newLine() const
{
    return nl;
}

/*! Returns the layout which is used for adding widgets to
  the dock window. The layout's orientation is set automatically to
  match the orientation of the dock window. You can add widgets to the
  layout using the box layout's QBoxLayout::addWidget() function.
    
  If the dock window only needs to contain a single widget use
  setWidget() instead.

  \sa setWidget() setOrientation()
*/

QBoxLayout *QDockWindow::boxLayout()
{
    return layout;
}

/*! \reimp
 */

QSize QDockWindow::sizeHint() const
{
    QSize sh( QFrame::sizeHint() );
    sh = sh.expandedTo( fixedExtent() );
    sh = sh.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    sh.setWidth( sh.width() + 2 * style().splitterWidth() / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    sh.setHeight( sh.height() + 2 * style().splitterWidth() / 3 );
    }
    return sh;
}

/*! \reimp
 */

QSize QDockWindow::minimumSize() const
{
    QSize ms( QFrame::minimumSize() );
    ms = ms.expandedTo( fixedExtent() );
    ms = ms.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    ms.setWidth( ms.width() + 2 * style().splitterWidth() / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    ms.setHeight( ms.height() + 2 * style().splitterWidth() / 3 );
    }
    return ms;
}

/*! \reimp
 */

QSize QDockWindow::minimumSizeHint() const
{
    QSize msh( QFrame::minimumSize() );
    msh = msh.expandedTo( fixedExtent() );
    msh = msh.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    msh.setWidth( msh.width() + 2 * style().splitterWidth() / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    msh.setHeight( msh.height() + 2 * style().splitterWidth() / 3 );
    }
    return msh;
}

/*!

  Undocks the QDockWindow from its current QDockArea, if it is
  docked, otherwise does nothing.

  Do not pass any \a w paramenter, this is for internal use only.

  \sa dock() QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/

void QDockWindow::undock( QWidget *w )
{
    if ( place() == OutsideDock && !w )
	return;

    QPoint p( 50, 50 );
    if ( topLevelWidget() )
	p = topLevelWidget()->pos() + QPoint( 20, 20 );
    if ( dockArea ) {
	delete (QDockArea::DockWindowData*)dockWindowData;
	dockWindowData = dockArea->dockWindowData( this );
	dockArea->removeDockWindow( this, TRUE, orientation() != Horizontal );
    }
    dockArea = 0;
    if ( lastPos != QPoint( -1, -1 ) && lastPos.x() > 0 && lastPos.y() > 0 )
	move( lastPos );
    else
	move( p );
    curPlace = OutsideDock;
    updateGui();
    emit orientationChanged( orientation() );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    if ( inherits( "QToolBar" ) )
	adjustSize();
    if ( !w ) {
	show();
    } else {
	reparent( w, 0, QPoint( 0, 0 ), FALSE );
	resize( 1, 1 );
	move( -width() - 5, -height() - 5 );
	show();
    }
}

void QDockWindow::removeFromDock( bool fixNewLines )
{
    if ( dockArea )
	dockArea->removeDockWindow( this, FALSE, FALSE, fixNewLines );
}

/*! Docks the dock window into the last QDockArea in which it was
    docked.

    If the dock window has no last dock area (e.g. it was created as a
    floating window and has never been docked), or if the last dock area
    it was docked in does not exist (e.g. the dock area has been
    deleted), nothing happens.

    \sa undock()
*/

void QDockWindow::dock()
{
    if ( !(QDockArea::DockWindowData*)dockWindowData ||
	 !( (QDockArea::DockWindowData*)dockWindowData )->area )
	return;
    curPlace = InDock;
    lastPos = pos();
    ( (QDockArea::DockWindowData*)dockWindowData )->
	area->dockWindow( this, (QDockArea::DockWindowData*)dockWindowData );
    emit orientationChanged( orientation() );
}

/*! \reimp
 */

void QDockWindow::hideEvent( QHideEvent *e )
{
    QFrame::hideEvent( e );
    if ( !parentWidget() || parentWidget()->isVisible() )
	emit visibilityChanged( FALSE );
}

/*! \reimp
 */

void QDockWindow::showEvent( QShowEvent *e )
{
    QFrame::showEvent( e );
    emit visibilityChanged( TRUE );
}

/*!
    If \a b is TRUE, the dock window will be shown normally when it is
    moved. If \a b is FALSE the dock window will be represented by an
    outline rectangle when it is moved.

    \sa opaqueMoving()
*/

void QDockWindow::setOpaqueMoving( bool b )
{
    opaque = b;
    horHandle->setOpaqueMoving( b );
    verHandle->setOpaqueMoving( b );
    titleBar->setOpaqueMoving( b );
}

/*! Returns TRUE if the dock window will be shown normally when moved.
    Returns FALSE if the dock window will be shown as an outline
    rectangle when moved.

  \sa setOpaqueMoving()
*/

bool QDockWindow::opaqueMoving() const
{
    return opaque;
}

/*! \reimp */

void QDockWindow::setCaption( const QString &s )
{
    titleBar->setText( s );
    QFrame::setCaption( s );
}

void QDockWindow::updateSplitterVisibility( bool visible )
{
    if ( area() && isResizeEnabled() ) {
	if ( orientation() == Horizontal ) {
	    if ( visible )
		vHandleRight->show();
	    else
		vHandleRight->hide();
	    vHandleLeft->hide();
	} else {
	    if ( visible )
		hHandleBottom->show();
	    else
		hHandleBottom->hide();
	    hHandleTop->hide();
	}
    }
}

bool QDockWindow::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Escape ) {
	    horHandle->mousePressed = FALSE;
	    verHandle->mousePressed = FALSE;
	    titleBar->mousePressed = FALSE;
	    endRectDraw( !opaque );
	    qApp->removeEventFilter( this );
	    return TRUE;
	}
    }

    return QFrame::eventFilter( o, e );
}



void QDockWindow::contextMenuEvent( QContextMenuEvent *e )
{
    QObject *o = this;
    while ( o ) {
	if ( o->inherits( "QMainWindow" ) )
	    break;
	o = o->parent();
    }
    if ( !o )
	return;
    if ( ( (QMainWindow*)o )->showDockMenu( e->globalPos() ) )
	e->accept();
}

#include "qdockwindow.moc"

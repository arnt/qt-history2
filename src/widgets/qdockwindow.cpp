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

void QDockWindowTitleBar::resizeEvent( QResizeEvent * )
{
    updateGui();
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

  \brief The QDockWindow class provides a widget which can live inside
  a QDockArea or as a floating window on the desktop

  \ingroup application

  The QDockWindow class provides a widget which can live and be
  managed inside a QDockArea or float as top level window on the
  desktop. All operations to move, resize, dock, undock, etc. this
  window are handled by this class. If the user moved a QDockWindow
  onto a QDockArea, the QDockWindow gets docked into that. If he/she
  moves it outside the area, the QDockWindow gets undocked and
  floating.

  QMainWindow contains four docking areas (one at each edge), which
  are used for docking windows into a mainwindow, like QToolBars or
  other QDockWindows. For usual applications you just use that
  functionality of QMainWindow and do not use QDockArea directly.

  It is importent that you pass the QDockWindow on construction either
  a QDockArea or a QMainWindow as parent.

  Normally a QDockWindow is used to handle the docking of one
  widget. In that case just use the setWidget() function. In special
  cases (like the QToolBar), multiple widget should be arranged in a
  box layout (depending on the orientation) inside the
  QDockWindow. For that you can use the boxLayout() function to access
  the box layout which should be used for that.

  In some cases it makes sense that the QDockWindow can be resized,
  e.g. when its widget() is a view which shows some data (like a
  QListView). To specify the resizeable state of the QDockWindow, use
  the setResizeEnabled() function. A resizeable QDockWindow can be
  resized inside a QDockArea using splitter-like handles, and when
  floating just like every other top level window.

  Also it sometimes makes sense to allow the user to close a
  QDockWindow, and somethimes not. To specify that, use the
  setCloseMode() function. Using the visibilityChanged() signal, you
  can keep track of the visibility of the QDockWindow.

  To programmatically dock or undock a QDockWindow, use the dock() and
  undock() functions, or call one of QDockArea::moveDockWindow(),
  QDockArea::removeDockWindow(), QMainWindow::moveDockWindow(),
  QMainWindow::removeDockWindow()

*/

/*!
  \enum QDockWindow::Place

  This enum specifies the possible locations for a QDockWindow:

  \value InDock  inside a QDockArea
  \value OutsideDock  floating on the desktop
*/

/*!
  \enum QDockWindow::CloseMode

  This enum type specifies how a QDockWindow is closed. The currently
  possible values are:

  \value Never  The dock window cannot be closed by the user
  \value Docked  The dock window has a close button only when docked.
  \value Undocked  The dock window has a close button only when floating.
  \value Always The dock window always has a close button.
*/

/*! \fn void QDockWindow::orientationChanged( Orientation o )

  This signal is emitted if the orientation of a QDockWindow changes
  to \a o.
*/

/*! \fn void QDockWindow::placeChanged( QDockWindow::Place p )

  This signal is emitted when the place of the dockwindow changed,
  i.e. it got docked into a dock area or undocked.

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/

/*! \fn void QDockWindow::visibilityChanged( bool visible )

  This signal is emitted if the visibility of the QDockWindow has been
  changed. If \a visible is TRUE, the QDockWindow got visible, else it
  got invisible.
*/

/*! \fn QDockArea *QDockWindow::area() const

  Returns the QDockArea, into which this QDockWindow is docked at the
  moment, or 0 if it is floating at the moment.
*/

/*! \fn QDockWindow::Place QDockWindow::place() const

  Returns the current place of the dockwindow. This is either InDock
  or OutsideDock.

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/


/*! Constructs a QDockWindow. If \a p is \c OutsideDock, it is created
  as floating window, else (\c InDock) it is docked into a
  QDockArea. If it should be docked, \a parent has to be either a
  QDockArea or a QMainWindow.
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

/*! Sets the orientation of the dockwindow to \a o. The orientation is
  propagated to the layout boxLayout().
*/

void QDockWindow::setOrientation( Orientation o )
{
    boxLayout()->setDirection( o == Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    QEvent *e = new QEvent( QEvent::LayoutHint );
    QApplication::postEvent( this, e );
}

/*! \reimp */

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
	dr.moveBy(-topLevelWidget()->x(), -topLevelWidget()->y());
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
	    dr.moveBy(-topLevelWidget()->x(), -topLevelWidget()->y());
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
	dr.moveBy(-topLevelWidget()->x(), -topLevelWidget()->y());
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

/*! Sets the main widget of this QDockWindow.

  \sa boxLayout()
*/

void QDockWindow::setWidget( QWidget *w )
{
    wid = w;
    boxLayout()->addWidget( w );
    updateGui();
}

/*!  Returns the main widget of this QDockWindow.

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
	dr.moveBy(-topLevelWidget()->x(), -topLevelWidget()->y());
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
	dr.moveBy(-topLevelWidget()->x(), -topLevelWidget()->y());
#endif
	unclippedPainter->drawRect( dr );
    }
    delete unclippedPainter;
    unclippedPainter = 0;
}

/*! If \a b is TRUE, this QDockWindow becomes resizeable, else not. A
  resizeable QDockWindow can be resized using splitter-like handles
  inside a QDockArea and like every other top level window when
  floating.
*/

void QDockWindow::setResizeEnabled( bool b )
{
    resizeEnabled = b;
    hbox->setMargin( b ? 0 : 2 );
    updateGui();
}

/*! If \a b is TRUE, the user can move this QDockWindow inside a
  QDockArea, else not.
*/

void QDockWindow::setMovingEnabled( bool b )
{
    moveEnabled = b;
    updateGui();
}

/*! Returns of the QDockWindow is resizeable or not.

  \sa setResizeEnabled()
*/

bool QDockWindow::isResizeEnabled() const
{
    return resizeEnabled;
}

/*! Returns of the QDockWindow is movable inside a QDockArea or not.

  \sa setMovingEnabled()
*/

bool QDockWindow::isMovingEnabled() const
{
    return moveEnabled;
}

/*! Specifies the close mode of this QDockWindow to be \a m. This can
  be \c Never, \c Docked, \c Undocked or \c Always.
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

/*! Returns whether the QDockWindow can be closed in the current
  place().

  \sa setCloseMode()
*/

bool QDockWindow::isCloseEnabled() const
{
    return  ( ( cMode & Docked ) == Docked && place() == InDock ||
	      ( cMode & Undocked ) == Undocked && place() == OutsideDock );
}

/*! Returns the close mode of that QDockWindow.

  \sa setCloseMode()
*/

int QDockWindow::closeMode() const
{
    return cMode;
}

/*!  Sets the QDockWindow to be horizontally stretchable, if \a b is
  TRUE, else not.
*/

void QDockWindow::setHorizontalStretchable( bool b )
{
    stretchable[ Horizontal ] = b;
}

/*!  Sets the QDockWindow to be vertically stretchable, if \a b is
  TRUE, else not.
*/

void QDockWindow::setVerticalStretchable( bool b )
{
    stretchable[ Vertical ] = b;
}

/*!  Returns whether the QDockWindow is horizontally stretchable.

  \sa setHorizontalStretchable()
 */

bool QDockWindow::isHorizontalStretchable() const
{
    return isResizeEnabled() || stretchable[ Horizontal ];
}

/*!  Returns whether the QDockWindow is vertically stretchable.

  \sa setVerticalStretchable()
 */

bool QDockWindow::isVerticalStretchable() const
{
    return isResizeEnabled() || stretchable[ Vertical ];
}

/*! Returns whether the QDockWindow is stretchable is the current
  orientation()
*/

bool QDockWindow::isStretchable() const
{
    if ( orientation() == Horizontal )
	return isHorizontalStretchable();
    return isVerticalStretchable();
}

/*! Returns the current orientation of the QDockWindow.

  \sa orientationChanged()
*/

Qt::Orientation QDockWindow::orientation() const
{
    if ( !dockArea || dockArea->orientation() == Horizontal )
	return Horizontal;
    return Vertical;
}

/*! Returns the offset which this QDockWindow wants to have in a
  QDockArea.
*/

int QDockWindow::offset() const
{
    return offs;
}

/*! Sets the offset which this QDockWindow wants to have in a
  QDockArea.
*/

void QDockWindow::setOffset( int o )
{
    offs = o;
}

/*! Returns the fixed extent (size) which this QDockWindow wants to
  have in a QDockArea.
*/

QSize QDockWindow::fixedExtent() const
{
    return fExtent;
}

/*! Sets the width of the fixed extent (size) which this QDockWindow
  wants to have in a QDockArea.
*/

void QDockWindow::setFixedExtentWidth( int w )
{
    fExtent.setWidth( w );
}

/*! Sets the height of the fixed extent (size) which this QDockWindow
  wants to have in a QDockArea.
*/

void QDockWindow::setFixedExtentHeight( int h )
{
    fExtent.setHeight( h );
}

/*! Sets whether this QDockWindow wants to start a new line in a
  QDockArea.
*/

void QDockWindow::setNewLine( bool b )
{
    nl = b;
}

/*! Returns whether this QDockWindow wants to start a new line in a
  QDockArea.
*/

bool QDockWindow::newLine() const
{
    return nl;
}

/*! Returns the layout which should be used for arranging widgets in
  this QDockWindow. The direction of it gets automatically adjusted to
  the orientation of the QDockWindow.

  \sa setWidget()
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

/*! \fn void QDockWindow::undock()

  Undocks the QDockWindow from its current QDockArea, if it is
  docked in one.

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
  QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/

/*! Undocks the QDockWindow from its current QDockArea, if it is
  docked in one. The \a w paramenter is for internal use only.

  \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
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

/*! Docks this QDockWindow back into the QDockArea, in which it has
  been docked last, if it was docked in one and this QDockArea still
  exists.
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

/*! If \a b is TRUE, the docking window will be used opaque, else
  transparently.
*/

void QDockWindow::setOpaqueMoving( bool b )
{
    opaque = b;
    horHandle->setOpaqueMoving( b );
    verHandle->setOpaqueMoving( b );
    titleBar->setOpaqueMoving( b );
}

/*! Returns whether the docking window will be moved opaque or
  transparently.

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


#include "qdockwindow.moc"

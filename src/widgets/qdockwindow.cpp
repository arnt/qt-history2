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

#include <qpainter.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qtimer.h>

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
    QDockWindow *dockWidget;

};

QDockWindowResizeHandle::QDockWindowResizeHandle( Qt::Orientation o, QWidget *parent,
						  QDockWindow *w, const char * )
    : QWidget( parent, "qt_dockwidget_internal" ), mousePressed( FALSE ), unclippedPainter( 0 ), dockWidget( w )
{
    setOrientation( o );
}

QSize QDockWindowResizeHandle::sizeHint() const
{
    int sw = style().splitterWidth();
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
    startLineDraw();
    lastPos = firstPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    drawLine( lastPos );
    lastPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	drawLine( lastPos );
	endLineDraw();
	if ( orientation() == Horizontal ) {
	    int dy;
	    if ( dockWidget->area()->gravity() == QDockArea::Normal )
		dy = e->globalPos().y() - firstPos.y();
	    else
		dy =  firstPos.y() - e->globalPos().y();
	    int d = dockWidget->height() + dy;
	    if ( orientation() != dockWidget->area()->orientation() )
		d = QMAX( d, dockWidget->minimumHeight() );
	    dockWidget->setFixedExtentHeight( d );
	} else {
	    int dx;
	    if ( dockWidget->area()->gravity() == QDockArea::Normal )
		dx = e->globalPos().x() - firstPos.x();
	    else
		dx = firstPos.x() - e->globalPos().x();
	    int d = dockWidget->width() + dx;
	    if ( orientation() != dockWidget->area()->orientation() )
		d = QMAX( d, dockWidget->minimumWidth() );
	    dockWidget->setFixedExtentWidth( d );
	}
	if ( orientation() != dockWidget->area()->orientation() )
	    dockWidget->area()->invalidNextOffset( dockWidget );
    }

    QApplication::postEvent( dockWidget->area(), new QEvent( QEvent::LayoutHint ) );
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
    bool unclipped = QApplication::desktop()->testWFlags( WPaintUnclipped );
    ( (QDockWindowResizeHandle*)QApplication::desktop() )->setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( QApplication::desktop() );
    if ( !unclipped )
	( (QDockWindowResizeHandle*)QApplication::desktop() )->clearWFlags( WPaintUnclipped );
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
    QPoint start = mapToGlobal( QPoint( 0, 0 ) );
    QPoint starta = dockWidget->area()->mapToGlobal( QPoint( 0, 0 ) );
    if ( orientation() == Horizontal ) {
	if ( orientation() == dockWidget->orientation() )
	    unclippedPainter->drawLine( starta.x() , globalPos.y(), starta.x() + dockWidget->area()->width(), globalPos.y() );
	else
	    unclippedPainter->drawLine( start.x(), globalPos.y(), start.x() + width(), globalPos.y() );
    } else {
	if ( orientation() == dockWidget->orientation() )
	    unclippedPainter->drawLine( globalPos.x(), starta.y(), globalPos.x(), starta.y() + dockWidget->area()->height() );
	else
	    unclippedPainter->drawLine( globalPos.x(), start.y(), globalPos.x(), start.y() + height() );
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

public:
    QDockWindowHandle( QDockWindow *dw );
    void updateGui();

    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }
    QSizePolicy sizePolicy() const;

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
    QDockWindow *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;
    QTimer *timer;

};

QDockWindowHandle::QDockWindowHandle( QDockWindow *dw )
    : QWidget( dw, "qt_dockwidget_internal" ), dockWidget( dw ), mousePressed( FALSE ), closeButton( 0 )
{
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( minimize() ) );
}

void QDockWindowHandle::paintEvent( QPaintEvent *e )
{
    if ( !dockWidget->dockArea )
	return;
    QPainter p( this );
    if ( !dockWidget->area() || !dockWidget->isCloseEnabled() ) {
	style().drawToolBarHandle( &p, QRect( 0, 0, width(), height() ),
				   dockWidget->orientation(), FALSE, colorGroup() );
    } else {
	if ( dockWidget->area()->orientation() == Horizontal )
	    style().drawToolBarHandle( &p, QRect( 0, 12, width(), height() - 12 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
	else
	    style().drawToolBarHandle( &p, QRect( 0, 1, width() - 12, height() - 1 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
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
    dockWidget->startRectDraw( e->pos() );
}

void QDockWindowHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed || e->pos() == offset )
	return;
    dockWidget->handleMove( e->globalPos() - offset, e->globalPos() );
}

void QDockWindowHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    if ( !hadDblClick && offset == e->pos() ) {
	timer->start( QApplication::doubleClickInterval(), TRUE );
    } else if ( !hadDblClick ) {
	dockWidget->updatePosition( e->globalPos() );
    }
}

void QDockWindowHandle::minimize()
{
    if ( dockWidget->area() && dockWidget->area()->parentWidget() &&
	 dockWidget->area()->parentWidget()->inherits( "QMainWindow" ) ) {
	QMainWindow *mw = (QMainWindow*)dockWidget->area()->parentWidget();
	if ( mw->isDockEnabled( dockWidget, Qt::Minimized ) )
	    mw->moveDockWindow( dockWidget, Qt::Minimized );
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
		 dockWidget, SLOT( hide() ) );
    }

    if ( dockWidget->isCloseEnabled() && dockWidget->area() )
	closeButton->show();
    else
	closeButton->hide();

    if ( !dockWidget->area() )
	return;

    if ( dockWidget->area()->orientation() == Horizontal )
	closeButton->move( 2, 2 );
    else
	closeButton->move( width() - closeButton->width() - 2, 2 );
}

QSize QDockWindowHandle::minimumSizeHint() const
{
    if ( dockWidget->orientation() == Horizontal )
	return QSize( dockWidget->isCloseEnabled() ? 18 : 12, 0 );
    return QSize( 0, dockWidget->isCloseEnabled() ? 18 : 12 );
}

QSizePolicy QDockWindowHandle::sizePolicy() const
{
    if ( dockWidget->orientation() != Horizontal )
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

class QDockWindowTitleBar : public QWidget
{
    Q_OBJECT

public:
    QDockWindowTitleBar( QDockWindow *dw );
    void updateGui();

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );

signals:
    void doubleClicked();

private:
    QDockWindow *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;

};

QDockWindowTitleBar::QDockWindowTitleBar( QDockWindow *dw )
    : QWidget( dw, "qt_dockwidget_internal" ), dockWidget( dw ), mousePressed( FALSE ),
      closeButton( 0 )
{
    setMouseTracking( TRUE );
    setMinimumHeight( 13 );
}

void QDockWindowTitleBar::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    mousePressed = TRUE;
    hadDblClick = FALSE;
    offset = e->pos();
    dockWidget->startRectDraw( e->pos() );
}

void QDockWindowTitleBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->handleMove( e->globalPos() - offset, e->globalPos() );
}

void QDockWindowTitleBar::mouseReleaseEvent( QMouseEvent *e )
{
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    if ( !hadDblClick )
	dockWidget->updatePosition( e->globalPos() );
}

void QDockWindowTitleBar::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );
    QPainter p( this );
    p.fillRect( rect(), colorGroup().highlight() );
    QFont f = p.font();
    f.setPointSize( f.pointSize() - 1 ); // #### we need a font size for that defined be the desktop environment....
    p.setFont( f );
    p.setPen( colorGroup().highlightedText() );
    p.drawText( 5, height() - p.fontMetrics().descent(), dockWidget->caption() );
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
		 dockWidget, SLOT( hide() ) );
    }

    if ( dockWidget->isCloseEnabled() )
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
  managed inside a QDockArea or float as toplevel window on the
  desktop. All operations to move, resize, dock, undock, etc. this
  window are handled by this class. If the user moved a QDockWindow
  onto a QDockArea, the QDockWindow gets docked into that. If he/she
  moves it outside the area, the QDockWindow gets undocked and
  floating.

  QMainWindow contains four docking areas (one at each edge), which
  are used for docking windows into a mainwindow, like QToolBars or
  other QDockWindows.

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
  floating just like every other toplevel window.

  Also it sometimes makes sense to allow the user to close a
  QDockWindow, and somethimes not. To specify that, use the
  setCloseMode() function. Using the visibilityChanged() signal, you
  can keep track of the visibility of the QDockWindow.

  To programmatically dock or undock a QDockWindow, use the dock() and
  undock() functions.
*/

/*!
  \enum QDockWindow::Place

  A QDockWindow can be either placed
  <ul>
  <li>\c InDock - inside a QDockArea, or
  <li>\c OutsideDock - floating on the desktop
  </ul>

*/

/*!
  \enum QDockWindow::CloseMode

  Using setCloseMode() you can specify the close mode of a
  QDockWindow. This can be one of

  <ul>
  <li>\c Never - Can be never closed by the user
  <li>\c Docked - Gets a close button on the handle if docked
  <li>\c Undocked - Gets a close button on the titelbar if floating
  <li>\c Always - Always has a closebutton
  </ul>

*/

/*! \fn void QDockWindow::orientationChanged( Orientation o )

  This signal is emitted if the orientation of a QDockWindow changes
  to \a o.
*/

/*! \fn void placeChanged( Place p )

  This signal is emitted when the place of the QDockWindow has been
  changed to \a p
*/

/*! \fn void visibilityChanged( bool visible )

  This signal is emitted if the visibility of the QDockWindow has been
  changed. If \a visible is TRUE, the QDockWindow got visible, else it
  got invisible.
*/


/*! Constructs a QDockWindow. If \a p is \c OutsideDock, it is created
  as floating window, else (\c InDock) it is docked into a
  QDockArea. If it should be docked, \a parent has to be either a
  QDockArea or a QMainWindow.
*/

QDockWindow::QDockWindow( Place p, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | ( p == OutsideDock ? WStyle_Customize | WStyle_NoBorderEx | WType_TopLevel | WStyle_Dialog : 0 ) ),
      curPlace( p ), wid( 0 ), unclippedPainter( 0 ), dockArea( 0 ), tmpDockArea( 0 ), resizeEnabled( FALSE ),
      moveEnabled( TRUE ), cMode( Never ), offs( 0 ), fExtent( -1, -1 ), nl( FALSE ), dockWidgetData( 0 ),
      lastPos( -1, -1 )
{
    widgetResizeHandler = new QWidgetResizeHandler( this );
    widgetResizeHandler->setMovingEnabled( FALSE );

    hbox = new QVBoxLayout( this );
    hbox->setMargin( 2 );
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
	( (QDockArea*)parent )->addDockWindow( this );
    } else if ( p == InDock ) {
	p = OutsideDock;
	setWFlags( WStyle_Customize | WStyle_NoBorderEx | WType_TopLevel | WStyle_Dialog );
    }
}

void QDockWindow::setOrientation( Orientation o )
{
    boxLayout()->setDirection( o == Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
}


QDockWindow::~QDockWindow()
{
    delete (QDockArea::DockWindowData*)dockWidgetData;
}

/*!  \reimp
*/

void QDockWindow::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    updateGui();
}


static void swapRect( QRect &r, Qt::Orientation o, const QPoint &offset )
{
    int w = r.width();
    if ( o == Qt::Horizontal )
	r.moveBy( -r.height() / 2, 0 );
    else
	r.moveBy( 0, -r.width() / 2 );
    r.moveBy( offset.x(), offset.y() );
    r.setWidth( r.height() );
    r.setHeight( w );
}

QWidget *QDockWindow::areaAt( const QPoint &gp )
{
    QWidget *w = qApp->widgetAt( gp, TRUE );
    QMainWindow *mw = 0;
    while ( w ) {
	if ( w->inherits( "QDockArea" ) ) {
	    QDockArea *a = (QDockArea*)w;
	    if ( !a->isDockWindowAccepted( this ) )
		return 0;
	    return w;
	}
	if ( w->inherits( "QMainWindow" ) )
	    mw = (QMainWindow*)w;
	w = w->parentWidget();
    }

    if ( !mw )
	return 0;

    QDockArea *a = mw->dockingArea( mw->mapFromGlobal( gp ) );
    if ( !a || !a->isDockWindowAccepted( this ) )
	return 0;
    return a;
}

void QDockWindow::handleMove( const QPoint &pos, const QPoint &gp )
{
    if ( !unclippedPainter )
	return;

    unclippedPainter->drawRect( currRect );
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = areaAt( gp );
    QPoint offset( mapFromGlobal( pos ) );
    currRect.moveBy( offset.x(), offset.y() );
    if ( !w || !w->inherits( "QDockArea" ) ) {
	if ( startOrientation != Horizontal )
	    swapRect( currRect, Horizontal, startOffset );
	unclippedPainter->setPen( QPen( gray, 3 ) );
	unclippedPainter->drawRect( currRect );
	state = OutsideDock;
	return;
    }

    state = InDock;
    QDockArea *area = (QDockArea*)w;
    if ( startOrientation != ( area ? area->orientation() : Horizontal ) )
	    swapRect( currRect, orientation(), startOffset );
    unclippedPainter->setPen( QPen( gray, 1 ) );
    unclippedPainter->drawRect( currRect );
    tmpDockArea = area;
}

void QDockWindow::updateGui()
{
    if ( curPlace == OutsideDock ) {
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
		    if ( area()->gravity() == QDockArea::Normal ) {
			hHandleBottom->show();
			hHandleTop->hide();
		    } else {
			hHandleTop->show();
			hHandleBottom->hide();
		    }
		    vHandleRight->show();
		    vHandleLeft->hide();
		} else {
		    if ( area()->gravity() == QDockArea::Normal ) {
			vHandleRight->show();
			vHandleLeft->hide();
		    } else {
			vHandleLeft->show();
			vHandleRight->hide();
		    }
		    hHandleBottom->show();
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
    if ( state == InDock ) {
	if ( tmpDockArea ) {
	    if ( dockArea && dockArea != tmpDockArea ) {
		delete (QDockArea::DockWindowData*)dockWidgetData;
		dockWidgetData = dockArea->dockWidgetData( this );
		dockArea->removeDockWindow( this, FALSE, FALSE );
	    }
	    dockArea = tmpDockArea;
	    dockArea->moveDockWindow( this, globalPos, currRect, startOrientation != orientation() );
	}
    } else {
	if ( dockArea ) {
	    delete (QDockArea::DockWindowData*)dockWidgetData;
	    dockWidgetData = dockArea->dockWidgetData( this );
	    dockArea->removeDockWindow( this, TRUE, startOrientation != Horizontal );
	}
	dockArea = 0;
	move( currRect.topLeft() );
    }
    bool doAdjustSize = curPlace != state && state == OutsideDock;
    curPlace = state;
    updateGui();
    emit orientationChanged( orientation() );
    emit placeChanged( curPlace );
    tmpDockArea = 0;
    if ( doAdjustSize ) {
	QApplication::sendPostedEvents( this, QEvent::LayoutHint );
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

void QDockWindow::startRectDraw( const QPoint &so )
{
    state = place();
    if ( unclippedPainter )
	endRectDraw();
    bool unclipped = QApplication::desktop()->testWFlags( WPaintUnclipped );
    ( (QDockWindow*)QApplication::desktop() )->setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( QApplication::desktop() );
    if ( !unclipped )
	( (QDockWindow*)QApplication::desktop() )->clearWFlags( WPaintUnclipped );
    unclippedPainter->setPen( QPen( gray, 3 ) );
    unclippedPainter->setRasterOp( XorROP );

    currRect = QRect( realWidgetPos( this ), size() );
    unclippedPainter->drawRect( currRect );
    startRect = currRect;
    startOrientation = orientation();
    startOffset = so;
}

void QDockWindow::endRectDraw()
{
    if ( !unclippedPainter )
	return;
    unclippedPainter->drawRect( currRect );
    delete unclippedPainter;
    unclippedPainter = 0;
}

/*! If \a b is TRUE, this QDockWindow becomes resizeable, else not. A
  resizeable QDockWindow can be resized using splitter-like handles
  insode a QDockArea and like every other toplevel window when
  floating.
*/

void QDockWindow::setResizeEnabled( bool b )
{
    resizeEnabled = b;
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
    return closeMode();
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
    return sh;
}

/*! \reimp
 */

QSize QDockWindow::minimumSize() const
{
    QSize ms( QFrame::minimumSize() );
    ms = ms.expandedTo( fixedExtent() );
    ms = ms.expandedTo( QSize( 16, 16 ) );
    return ms;
}

/*! \reimp
 */

QSize QDockWindow::minimumSizeHint() const
{
    QSize msh( QFrame::minimumSize() );
    msh = msh.expandedTo( fixedExtent() );
    msh = msh.expandedTo( QSize( 16, 16 ) );
    return msh;
}

/*! Undocks the QDockWindow from its current QDockArea, if it is
  docked in one. The \a w paramenter is for internal use only.
*/

void QDockWindow::undock( QWidget *w )
{
    if ( place() == OutsideDock && !w )
	return;

    QPoint p( 50, 50 );
    if ( topLevelWidget() )
	p = topLevelWidget()->pos() + QPoint( 20, 20 );
    if ( dockArea ) {
	delete (QDockArea::DockWindowData*)dockWidgetData;
	dockWidgetData = dockArea->dockWidgetData( this );
	dockArea->removeDockWindow( this, TRUE, orientation() != Horizontal );
    }
    dockArea = 0;
    if ( lastPos != QPoint( -1, -1 ) )
	move( lastPos );
    else
	move( p );
    curPlace = OutsideDock;
    updateGui();
    emit orientationChanged( orientation() );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
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

void QDockWindow::removeFromDock()
{
    if ( dockArea )
	dockArea->removeDockWindow( this, FALSE, FALSE );
}

/*! Docks this QDockWindow back into the QDockArea, in which it has
  been docked last, if it was docked in one and this QDockArea still
  exists.
*/

void QDockWindow::dock()
{
    if ( !(QDockArea::DockWindowData*)dockWidgetData ||
	 !( (QDockArea::DockWindowData*)dockWidgetData )->area )
	return;
    curPlace = InDock;
    lastPos = pos();
    ( (QDockArea::DockWindowData*)dockWidgetData )->
	area->dockWidget( this, (QDockArea::DockWindowData*)dockWidgetData );
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

#include "qdockwindow.moc"

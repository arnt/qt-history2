/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsizegrip.cpp#11 $
**
** Implementation of QSizeGrip class
**
** Created : 980119
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qsizegrip.h"
#include "qpainter.h"
#include "qapplication.h"

#if defined(_WS_X11_)
#include "qt_x11.h"
extern Atom qt_sizegrip;			// defined in qapplication_x11.cpp
#elif defined (_WS_WIN_ )
#include "qobjectlist.h"
#include "qt_windows.h"
#endif


static QWidget *qt_sizegrip_topLevelWidget( QWidget* w)
{
    QWidget *p = w->parentWidget();
    while ( !w->testWFlags(Qt::WType_TopLevel) && p && 
	    ( !p->inherits("QWorkspace") || !p->inherits( "QFileDialog" ) ) ) {
	w = p;
	p = p->parentWidget();
    }
    return w;
}

static QWidget* qt_sizegrip_workspace( QWidget* w )
{
    while ( w && ( !w->inherits("QWorkspace") || !w->inherits( "QFileDialog" ) ) ) {
	w = w->parentWidget();
    }
    return w;
}


// NOT REVISED
/*! \class QSizeGrip qsizegrip.h

  \brief The QSizeGrip class provides corner-grip for resizeing a top level
	    window.

  \ingroup realwidgets
  \ingroup application

  This widget works like the standard Windows resize handle.  In the
  X11 version this resize handle generally works differently than the
  one provided by the system; we hope to reduce this difference in the
  future.

  Put this widget anywhere in a tree and the user can use it to resize
  the top-level window.  Generally this should be in the lower right-hand
  corner.  Note that QStatusBar already uses this widget, so if you have
  a status bar (eg. you are using QMainWindow), then you don't need to
  use this widget explicitly.

  <img src=qsizegrip-m.png> <img src=qsizegrip-w.png>

  \sa QStatusBar
*/


/*!
  Construct a resize corner as a child widget of \a parent.
*/
QSizeGrip::QSizeGrip( QWidget * parent, const char* name )
    : QWidget( parent, name )
{
    setCursor( sizeFDiagCursor );

#if defined(_WS_X11_)
    WId id = winId();
    XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
		    qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
		    (unsigned char *)&id, 1);
#endif
}


/*!
  Destructor
 */
QSizeGrip::~QSizeGrip()
{
#if defined(_WS_X11_)
    if ( !QApplication::closingDown() && parentWidget() ) {
	WId id = None;
 	XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
 			qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
 			(unsigned char *)&id, 1);
    }
#endif
}

/*!
  Returns a small size.
*/
QSize QSizeGrip::sizeHint() const
{
    return QSize( 13, 13 );
}

/*!
  Paints the resize grip - small diagonal textured lines in the
  lower righthand corner.
*/
void QSizeGrip::paintEvent( QPaintEvent *e )
{
    QPainter painter( this );
    painter.setClipRegion(e->region());
    painter.translate( width()-13, height()-13 ); // paint in the corner
    QPointArray a;
    a.setPoints( 3, 1,12, 12,1, 12,12 );
    painter.setPen( colorGroup().dark() );
    painter.setBrush( colorGroup().dark() );
    painter.drawPolygon( a );
    painter.setPen( colorGroup().light() );
    painter.drawLine(  0, 12, 12,  0 );
    painter.drawLine(  5, 12, 12,  5 );
    painter.drawLine( 10, 12, 12, 10 );
    painter.setPen( colorGroup().background() );
    painter.drawLine(  4, 12, 12,  4 );
    painter.drawLine(  9, 12, 12,  9 );
}

/*!
  Primes the resize operation.
*/
void QSizeGrip::mousePressEvent( QMouseEvent * e )
{
    p = e->globalPos();
    s = qt_sizegrip_topLevelWidget(this)->size();
}


/*!
  Resizes the top-level widget containing this widget.
*/
void QSizeGrip::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() != LeftButton )
	return;

    QWidget* tlw = qt_sizegrip_topLevelWidget(this);
    if ( tlw->testWState(WState_ConfigPending) )
	return;

    QPoint np( e->globalPos() );

    QWidget* ws = qt_sizegrip_workspace( this );
    if ( ws ) {
	QPoint tmp( ws->mapFromGlobal( np ) );
	if ( tmp.x() > ws->width() )
	    tmp.setX( ws->width() );
	if ( tmp.y() > ws->height() )
	    tmp.setY( ws->height() );
	np = ws->mapToGlobal( tmp );
    }

    int w = np.x() - p.x() + s.width();
    int h = np.y() - p.y() + s.height();
    if ( w < 1 )
	w = 1;
    if ( h < 1 )
	h = 1;
    tlw->resize( w, h );
#ifdef _WS_WIN_
    MSG msg;
    while( PeekMessage( &msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
      ;
#endif
    QApplication::syncX();
}



/*!
  The size grip has a fixed height, but can grow horizontally.
*/

QSizePolicy QSizeGrip::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

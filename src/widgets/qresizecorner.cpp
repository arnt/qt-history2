/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qresizecorner.cpp#2 $
**
** Implementation of QResizeCorner class
**
** Created : 980119
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qresizecorner.h"
#include "qpainter.h"
#include "qapplication.h"

/*! \class QResizeCorner qresizecorner.h

  \brief The QResizeCorner class provides corner-grip for resizeing a top level
	    window.

  \ingroup realwidgets
  \ingroup application

  This widgets works like the standard Windows resize handle.  In the
  X11 version this resize handle generally works differently than the
  one provided by the system; we hope to reduce this difference in the
  future.  

  Put this widget anywhere in a tree and the user can use it to resize
  the top-level window.  Generally this should be in the lower right-hand
  corner.  Note that QStatusBar already uses this widget, so if you have
  a status bar (eg. you are using QMainWindow), then you don't need to
  use this widget explicitly.

  <img src=qresizecorner-m.gif> <img src=qresizecorner-w.gif>

  \sa QStatusBar
*/


/*!
  Construct a resize corner as a child widget of \a parent.
*/
QResizeCorner::QResizeCorner( QWidget * parent, const char* name )
    : QWidget( parent, name )
{
    setCursor( sizeFDiagCursor );
    setSizeGrip( TRUE );
}

/*!
  Returns a small size.
*/
QSize QResizeCorner::sizeHint() const
{
    return QSize( 13, 13 );
}

/*!
  Paints the resize grip - small diagonal textured lines in the
  lower righthand corner.
*/
void QResizeCorner::paintEvent( QPaintEvent *e )
{
    QPainter painter( this );
    e->setClipRegion(e->region());
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
void QResizeCorner::mousePressEvent( QMouseEvent * e )
{
    p = e->globalPos();
    s = topLevelWidget()->size();
}


/*!
  Resizes the top-level widget containing this widget.
*/
void QResizeCorner::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() != LeftButton )
	return;

    if ( topLevelWidget()->testWState(WState_ConfigPending) )
	return;

    QPoint np( e->globalPos() );

    int w = np.x() - p.x() + s.width();
    int h = np.y() - p.y() + s.height();
    if ( w < 1 )
	w = 1;
    if ( h < 1 )
	h = 1;
    topLevelWidget()->resize( w, h );

    QApplication::syncX();
}


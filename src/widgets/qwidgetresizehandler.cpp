/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Implementation of the QWidgetResizeHandler class
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

#include "qwidgetresizehandler.h"
#include "qframe.h"
#include "qapplication.h"
#include "qcursor.h"
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#define RANGE 4

static bool resizeHorizontalDirectionFixed = FALSE;
static bool resizeVerticalDirectionFixed = FALSE;

QWidgetResizeHandler::QWidgetResizeHandler( QWidget *parent, QWidget *cw, const char *name )
    : QObject( parent, name ), widget( parent ), childWidget( cw ? cw : parent ),
      buttonDown( FALSE ), moveResizeMode( FALSE ), extrahei( 0 ), moving( TRUE )
{
    mode = Nowhere;
    widget->setMouseTracking( TRUE );
    active = TRUE;
    qApp->installEventFilter( this );
}

static QWidget *childOf( QWidget *w, QWidget *child )
{
    while ( child ) {
	if ( child == w )
	    return child;
	child = child->parentWidget();
    }
    return 0;
}

bool QWidgetResizeHandler::eventFilter( QObject *o, QEvent *ee )
{
    if ( !o || !ee || !active || !o->isWidgetType() )
	return FALSE;

    QWidget *w = childOf( widget, (QWidget*)o );
    if ( !w )
	return FALSE;


    QMouseEvent *e = (QMouseEvent*)ee;
    switch ( e->type() ) {
    case QEvent::MouseButtonPress: {
	if ( e->button() == LeftButton ) {
	    emit activate();
	    bool me = isMovingEnabled();
	    setMovingEnabled( me && o == widget );
	    mouseMoveEvent( e );
	    setMovingEnabled( me );
	    buttonDown = TRUE;
	    moveOffset = e->pos();
	    invertedMoveOffset = widget->rect().bottomRight() - e->pos();
	}
    } break;
    case QEvent::MouseButtonRelease:
	if ( e->button() == LeftButton ) {
	    buttonDown = FALSE;
	    widget->releaseMouse();
	    widget->releaseKeyboard();
	    widget->setCursor( arrowCursor );
	}
	break;
    case QEvent::MouseMove: {
	bool me = isMovingEnabled();
	setMovingEnabled( me && o == widget );
	mouseMoveEvent( e );
	setMovingEnabled( me );
	if ( buttonDown && mode != Center )
	    return TRUE;
    } break;
    case QEvent::KeyPress:
	keyPressEvent( (QKeyEvent*)e );
	break;
    default:
	break;
    }
    return FALSE;
}

void QWidgetResizeHandler::mouseMoveEvent( QMouseEvent *e )
{
    QPoint pos = widget->mapFromGlobal( e->globalPos() );
    if ( !buttonDown || e->state() == NoButton ) {
	if ( pos.y() <= RANGE && pos.x() <= RANGE)
	    mode = TopLeft;
	else if ( pos.y() >= widget->height()-RANGE && pos.x() >= widget->width()-RANGE)
	    mode = BottomRight;
	else if ( pos.y() >= widget->height()-RANGE && pos.x() <= RANGE)
	    mode = BottomLeft;
	else if ( pos.y() <= RANGE && pos.x() >= widget->width()-RANGE)
	    mode = TopRight;
	else if ( pos.y() <= RANGE )
	    mode = Top;
	else if ( pos.y() >= widget->height()-RANGE )
	    mode = Bottom;
	else if ( pos.x() <= RANGE )
	    mode = Left;
	else if (  pos.x() >= widget->width()-RANGE )
	    mode = Right;
	else
	    mode = Center;
#ifndef QT_NO_CURSOR
	setMouseCursor( mode );
#endif
	return;
    }

    if ( widget->testWState( WState_ConfigPending ) )
 	return;

    QPoint globalPos = widget->parentWidget( TRUE ) ?
		       widget->parentWidget( TRUE ) ->mapFromGlobal( e->globalPos() ) : e->globalPos();
    if ( widget->parentWidget( TRUE ) && !widget->parentWidget( TRUE )->rect().contains( globalPos ) ) {
	if ( globalPos.x() < 0 )
	    globalPos.rx() = 0;
	if ( globalPos.y() < 0 )
	    globalPos.ry() = 0;
	if ( globalPos.x() > widget->parentWidget()->width() )
	    globalPos.rx() = widget->parentWidget()->width();
	if ( globalPos.y() > widget->parentWidget()->height() )
	    globalPos.ry() = widget->parentWidget()->height();
    }

    QPoint p = globalPos + invertedMoveOffset;
    QPoint pp = globalPos - moveOffset;

    int fw = widget->inherits( "QFrame" ) ? ( (QFrame*)widget )->frameWidth() : 0;
    int mw = QMAX( childWidget->minimumSizeHint().width(),
		  childWidget->minimumWidth()) + 2 * fw;
    int mh = QMAX( childWidget->minimumSizeHint().height(),
		   childWidget->minimumHeight()) +  2 * fw + extrahei + 1;

    QSize mpsize( widget->geometry().right() - pp.x() + 1,
		  widget->geometry().bottom() - pp.y() + 1 );
    mpsize = mpsize.expandedTo( widget->minimumSize() ).expandedTo( QSize(mw, mh) );
    QPoint mp( widget->geometry().right() - mpsize.width() + 1,
	       widget->geometry().bottom() - mpsize.height() + 1 );

    QRect geom = widget->geometry();

    switch ( mode ) {
    case TopLeft:
	geom =  QRect( mp, widget->geometry().bottomRight() ) ;
	break;
    case BottomRight:
	geom =  QRect( widget->geometry().topLeft(), p ) ;
	break;
    case BottomLeft:
	geom =  QRect( QPoint(mp.x(), widget->geometry().y() ), QPoint( widget->geometry().right(), p.y()) ) ;
	break;
    case TopRight:
	geom =  QRect( QPoint( widget->geometry().x(), mp.y() ), QPoint( p.x(), widget->geometry().bottom()) ) ;
	break;
    case Top:
	geom =  QRect( QPoint( widget->geometry().left(), mp.y() ), widget->geometry().bottomRight() ) ;
	break;
    case Bottom:
	geom =  QRect( widget->geometry().topLeft(), QPoint( widget->geometry().right(), p.y() ) ) ;
	break;
    case Left:
	geom =  QRect( QPoint( mp.x(), widget->geometry().top() ), widget->geometry().bottomRight() ) ;
	break;
    case Right:
	geom =  QRect( widget->geometry().topLeft(), QPoint( p.x(), widget->geometry().bottom() ) ) ;
	break;
    case Center:
	if ( isMovingEnabled() )
	    geom.moveTopLeft( pp );
	break;
    default:
	break;
    }

    geom = QRect( geom.topLeft(), geom.size().expandedTo( widget->minimumSize() ).expandedTo( QSize(mw,mh) ).
		  boundedTo( childWidget->maximumSize() + QSize( 2*fw, 2*fw + extrahei +1 ) ) );

    if ( geom != widget->geometry() && !widget->parentWidget() || widget->parentWidget()->rect().intersects( geom ) )
	widget->setGeometry( geom );

#if defined(Q_WS_WIN)
    MSG msg;
    while(PeekMessage( &msg, widget->winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ))
	;
#endif
    QApplication::syncX();
}

void QWidgetResizeHandler::setMouseCursor( MousePosition m )
{
    switch ( m ) {
    case TopLeft:
    case BottomRight:
	widget->setCursor( sizeFDiagCursor );
	break;
    case BottomLeft:
    case TopRight:
	widget->setCursor( sizeBDiagCursor );
	break;
    case Top:
    case Bottom:
	widget->setCursor( sizeVerCursor );
	break;
    case Left:
    case Right:
	widget->setCursor( sizeHorCursor );
	break;
    default:
	widget->setCursor( arrowCursor );
	break;
    }
}

void QWidgetResizeHandler::keyPressEvent( QKeyEvent * e )
{
    if ( !isMove() && !isResize() )
	return;
    bool is_control = e->state() & ControlButton;
    int delta = is_control?1:8;
    QPoint pos = QCursor::pos();
    switch ( e->key() ) {
    case Key_Left:
	pos.rx() -= delta;
	if ( pos.x() <= QApplication::desktop()->geometry().left() ) {
	    if ( mode == TopLeft || mode == BottomLeft ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomRight )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = TopLeft;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Right:
	pos.rx() += delta;
	if ( pos.x() >= QApplication::desktop()->geometry().right() ) {
	    if ( mode == TopRight || mode == BottomRight ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = BottomRight;
	    else if ( mode == TopLeft )
		mode = TopRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Up:
	pos.ry() -= delta;
	if ( pos.y() <= QApplication::desktop()->geometry().top() ) {
	    if ( mode == TopLeft || mode == TopRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = TopLeft;
	    else if ( mode == BottomRight )
		mode = TopRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Down:
	pos.ry() += delta;
	if ( pos.y() >= QApplication::desktop()->geometry().bottom() ) {
	    if ( mode == BottomLeft || mode == BottomRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == TopLeft )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = BottomRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Space:
    case Key_Return:
    case Key_Enter:
	moveResizeMode = FALSE;
	widget->releaseMouse();
	widget->releaseKeyboard();
	buttonDown = FALSE;
	break;
    default:
	return;
    }
    QCursor::setPos( pos );
}


void QWidgetResizeHandler::doResize()
{
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = widget->mapFromGlobal( QCursor::pos() );
    if ( moveOffset.x() < widget->width()/2) {
	if ( moveOffset.y() < widget->height()/2)
	    mode = TopLeft;
	else
	    mode = BottomLeft;
    } else {
	if ( moveOffset.y() < widget->height()/2)
	    mode = TopRight;
	else
	    mode = BottomRight;
    }
    invertedMoveOffset = widget->rect().bottomRight() - moveOffset;
#ifndef QT_NO_CURSOR
    setMouseCursor( mode );
    widget->grabMouse( widget->cursor()  );
#else
    widget->grabMouse();
#endif
    widget->grabKeyboard();
    resizeHorizontalDirectionFixed = FALSE;
    resizeVerticalDirectionFixed = FALSE;
}

void QWidgetResizeHandler::doMove()
{
    mode = Center;
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = widget->mapFromGlobal( QCursor::pos() );
    invertedMoveOffset = widget->rect().bottomRight() - moveOffset;
#ifndef QT_NO_CURSOR
    widget->grabMouse( SizeAllCursor );
#else
    widget->grabMouse();
#endif
    widget->grabKeyboard();
}


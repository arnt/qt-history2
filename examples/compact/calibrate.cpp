/****************************************************************************
** $Id: //depot/qt/main/examples/pdasingle/launcher.cpp $
**
** Qt/Embedded single application "launcher" demo for small devices.
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qwindowsystem_qws.h>
#include "calibrate.h"


Calibrate::Calibrate() :
    QWidget( 0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop | WDestructiveClose )
{
    const int offset = 30;
    QRect desk = qApp->desktop()->geometry();
    setGeometry( 0, 0, desk.width(), desk.height() );
    logo = new QPixmap( "/usr/local/qt-embedded/etc/images/qtlogo.png" );
    cd.screenPoints[QWSPointerCalibrationData::TopLeft] = QPoint( offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomLeft] = QPoint( offset, desk.height() - offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomRight] = QPoint( desk.width() - offset, desk.height() - offset );
    cd.screenPoints[QWSPointerCalibrationData::TopRight] = QPoint( desk.width() - offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::Center] = QPoint( desk.width()/2, desk.height()/2 );
    crossPos = cd.screenPoints[QWSPointerCalibrationData::TopLeft];
    location = QWSPointerCalibrationData::TopLeft;

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    QWSServer::mouseHandler()->clearCalibration();
    grabMouse();
}

Calibrate::~Calibrate()
{
    delete logo;
}

void Calibrate::moveCrosshair( QPoint pt )
{
    QPainter p( this );
    p.drawPixmap( crossPos.x()-8, crossPos.y()-8, saveUnder );
    saveUnder = QPixmap::grabWindow( winId(), pt.x()-8, pt.y()-8, 16, 16 );
    p.drawRect( pt.x()-1, pt.y()-8, 2, 7 );
    p.drawRect( pt.x()-1, pt.y()+1, 2, 7 );
    p.drawRect( pt.x()-8, pt.y()-1, 7, 2 );
    p.drawRect( pt.x()+1, pt.y()-1, 7, 2 );
    crossPos = pt;
}

void Calibrate::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int y;

    if ( !logo->isNull() ) {
	y = height() / 2 - logo->height() - 15;
	p.drawPixmap( (width() - logo->width())/2, y, *logo );
    }

    y = height() / 2 + 15;

    QString text( "Welcome to the Qt/Embedded PDA demo" );
    p.setFont( QFont( "helvetica", 10, QFont::Bold ) );
    p.drawText( 0, y, width(), height() - y, AlignHCenter, text );

    y += 40;
    text = "Touch the crosshairs to\ncalibrate your screen.";
    p.setFont( QFont( "helvetica", 10 ) );
    p.drawText( 0, y, width(), height() - y, AlignHCenter, text );

    saveUnder = QPixmap::grabWindow( winId(), crossPos.x()-8, crossPos.y()-8,
				     16, 16 );
    moveCrosshair( crossPos );
}

void Calibrate::mousePressEvent( QMouseEvent *e )
{
    if ( penPos.isNull() )
	penPos = e->pos();
    else
	penPos = QPoint( (penPos.x() + e->pos().x())/2,
			 (penPos.y() + e->pos().y())/2 );
}

void Calibrate::mouseReleaseEvent( QMouseEvent * )
{
    if ( timer->isActive() )
	return;

    cd.devPoints[location] = penPos;
    if ( location < QWSPointerCalibrationData::LastLocation ) {
	location += 1;
	QPoint target = cd.screenPoints[location];
	dx = (target.x() - crossPos.x())/10;
	dy = (target.y() - crossPos.y())/10;
	timer->start( 30 );
    } else {
	QWSServer::mouseHandler()->calibrate( &cd );
	releaseMouse();
	hide();
	close();
    }
}

void Calibrate::timeout()
{
    QPoint target = cd.screenPoints[location];

    bool doneX = FALSE;
    bool doneY = FALSE;
    QPoint newPos( crossPos.x() + dx, crossPos.y() + dy );

    if ( QABS(crossPos.x() - target.x()) <= QABS(dx) ) {
	newPos.setX( target.x() );
	doneX = TRUE;
    }

    if ( QABS(crossPos.y() - target.y()) <= QABS(dy) ) {
	newPos.setY(target.y());
	doneY = TRUE;
    }

    if ( doneX && doneY ) {
	penPos = QPoint();
	timer->stop();
    }

    moveCrosshair( newPos );
}


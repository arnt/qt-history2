/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.cpp#1 $
**
** Implementation of QButton class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qdict.h"
#include "qpixmap.h"
declare(QDictM,QPixMap);			// internal pixmap dict
#define	 QPixMapDict QDictM(QPixMap)
#include "qbutton.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbutton.cpp#1 $";
#endif


QPixMapDict *QButton::pmdict = 0;		// pixmap dict
long QButton::pmsize = 0;			// size of all pixmaps


QButton::QButton( QView *parent ) : QWidget( parent )
{
    initMetaObject();
    onOffButton = FALSE;			// button is not on/off
    buttonDown = FALSE;				// button is up
    buttonOn = FALSE;				// button is off
}


bool QButton::acceptPixmap( int w, int h )	// accept pixmap
{
    long size = w*h;
    if ( size > 5000 )				// will be slow
	return FALSE;
    if ( pmsize + size > 80000 ) {
	debug("QButton: Pixmap cache not recommended");
	return FALSE;
    }
    return TRUE;
}

QPixMap *QButton::findPixmap( const char *key ) // lookup saved pixmap
{
    return pmdict ? pmdict->find( key ) : 0;
}

void QButton::savePixmap( const char *key, const QPixMap *pm )
{
    if ( !pmdict ) {				// create pixmap dict
	pmdict = new QPixMapDict( 31 );
	CHECK_PTR( pmdict );
	qAddCleanupRoutine( delPixmaps );
    }
    debug( "QButton: Insert pixmap %s", key );
    pmdict->insert( key, pm );			// insert into dict
    pmsize += pm->size().width()*pm->size().height();
}

void QButton::delPixmaps()			// delete all pixmaps
{
    debug( "QButton: Deleting pixmaps, total size = %ld", pmsize );
    pmdict->statistics();
    pmdict->setAutoDelete( TRUE );
    pmdict->clear();
    delete pmdict;
}

void QButton::switchOn()			// switch button on
{
#if defined(CHECK_STATE)
    if ( !onOffButton )
	warning( "QButton::switchOn: Only on/off buttons should be switched" );
#endif
    bool lastOn = buttonOn;
    buttonOn = TRUE;
    if ( !lastOn )				// changed state
	paintEvent( 0 );			// redraw
}

void QButton::switchOff()			// switch button off
{
#if defined(CHECK_STATE)
    if ( !onOffButton )
	warning( "QButton::switchOff: Only on/off buttons should be switched");
#endif
    bool lastOn = buttonOn;
    buttonOn = FALSE;
    if ( lastOn )				// changed state
	paintEvent( 0 );			// redraw
}


void QButton::setOnOffButton( bool onOff )	// set to on/off button
{
    onOffButton = onOff;
}


bool QButton::hitButton( const QPoint &pos ) const
{
    return clientRect().contains( pos );
}

void QButton::drawButton( QPainter * )
{
    return;
}


void QButton::mousePressEvent( QMouseEvent *e ) // mouse press
{
    if ( e->button() != LeftButton )
	return;
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse press on button
	buttonDown = TRUE;
	emit pressed();
	paintEvent(0);
    }
}

void QButton::mouseReleaseEvent( QMouseEvent *e)// mouse release
{
    if ( e->button() != LeftButton )
	return;
    bool hit = hitButton( e->pos() );
    buttonDown = FALSE;
    emit released();
    if ( hit ) {				// mouse release on button
	if ( onOffButton )
	    buttonOn = !buttonOn;
	emit clicked();
    }
    paintEvent( 0 );
}

void QButton::mouseMoveEvent( QMouseEvent *e )	// mouse move event
{
    if ( !(e->state() & LeftButton) )		// left button is up
	return;
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse move in button
	if ( !buttonDown ) {
	    buttonDown = TRUE;
	    emit pressed();
	    paintEvent(0);
	}
    }
    else {					// mouse move outside button
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    emit released();
	    paintEvent(0);
	}
    }
}

void QButton::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawButton( &paint );			// ask subclass to draw button
    paint.end();
}

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.cpp#11 $
**
** Implementation of QButton class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdict.h"
#include "qpixmap.h"
declare(QDictM,QPixMap);			// internal pixmap dict
#define	 QPixMapDict QDictM(QPixMap)
#include "qbutton.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbutton.cpp#11 $";
#endif


QPixMapDict *QButton::pmdict = 0;		// pixmap dict
long QButton::pmsize = 0;			// size of all pixmaps


QButton::QButton( QWidget *parent, const char *name ) : QWidget( parent, name )
{
    initMetaObject();
    onOffButton = FALSE;			// button is not on/off
    buttonDown = FALSE;				// button is up
    buttonOn = FALSE;				// button is off
    mlbDown = FALSE;				// mouse left button up
}


bool QButton::acceptPixmap( int w, int h )	// accept pixmap
{
    long size = w*h;
    if ( size > 5000 )				// will be slow
	return FALSE;
    if ( pmsize + size > 80000 )		// limit reached
	return FALSE;
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
	qAddPostRoutine( delPixmaps );
    }
    pmdict->insert( key, pm );			// insert into dict
    pmsize += pm->size().width()*pm->size().height();
}

void QButton::delPixmaps()			// delete all pixmaps
{
    pmdict->setAutoDelete( TRUE );
    pmdict->clear();
    delete pmdict;
}


const char *QButton::label() const		// get button label
{
    return btext;
}

void QButton::setLabel( const char *label, bool resize )
{						// set button label
    btext = label;
    if ( resize )
	resizeFitLabel();
    update();
}


void QButton::resizeFitLabel()			// do nothing
{
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
	repaint( FALSE );			// redraw
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
	repaint( FALSE );			// redraw
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
    if ( e->button() != LeftButton || mlbDown )
	return;
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse press on button
	mlbDown = TRUE;				// left mouse button down
	buttonDown = TRUE;
	repaint( FALSE );
	emit pressed();
    }
}

void QButton::mouseReleaseEvent( QMouseEvent *e)// mouse release
{
    if ( e->button() != LeftButton || !mlbDown )
	return;
    mlbDown = FALSE;				// left mouse button up
    bool hit = hitButton( e->pos() );
    buttonDown = FALSE;
    if ( hit ) {				// mouse release on button
	if ( onOffButton )
	    buttonOn = !buttonOn;
	repaint( FALSE );
	emit released();
	emit clicked();
    }
    else {
	repaint( FALSE );
	emit released();
    }
}

void QButton::mouseMoveEvent( QMouseEvent *e )	// mouse move event
{
    if ( !((e->state() & LeftButton) && mlbDown) )
	return;					// left mouse button is up
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse move in button
	if ( !buttonDown ) {
	    buttonDown = TRUE;
	    repaint( FALSE );
	    emit pressed();
	}
    }
    else {					// mouse move outside button
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    repaint( FALSE );
	    emit released();
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


bool QButton::focusInEvent( QEvent * )
{
    return TRUE;
}

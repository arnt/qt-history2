/****************************************************************************
** $Id$
**
** Implementation of QInputContext class
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qinputcontext_p.h"
#include "qstring.h"
#include "qwindowsystem_qws.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qapplication.h"
#include "qevent.h"

#ifndef QT_NO_QWS_IM
QWidget* QInputContext::activeWidget = 0;
QString* QInputContext::composition = 0;
bool QInputContext::composeMode = FALSE;

static bool sendEndToPrev = FALSE;
static QWidget* prevFocusW = 0;


bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event); //in qapplication_qws.cpp

#include <qmultilineedit.h>
#include <qlineedit.h>

void QInputContext::retrieveMarkedText( QWidget *w )
{
    QString s;
    //Only lineedit and multilineedit are IM-enabled anyway, so
    //we might as well do it all here instead of sending events
/*
#ifndef QT_NO_LINEEDIT
    if ( w->inherits( "QLineEdit" ) ) {
	s = ((QLineEdit*)w)->markedText();
    }
# ifndef QT_NO_MULTILINEEDIT
    else
# endif
#endif
#ifndef QT_NO_MULTILINEEDIT
    if ( w->inherits( "QMultiLineEdit" ) ) {
	s = ((QMultiLineEdit*)w)->markedText();
    }
#endif
*/
    QByteArray ba;
    int len =  s.length()*sizeof(QChar);
    ba.duplicate( (const char*)s.unicode(), len );
    QPaintDevice::qwsDisplay()->
	setProperty( 0, QT_QWS_PROPERTY_MARKEDTEXT, 
		     QWSPropertyManager::PropReplace, ba );
}

void QInputContext::translateIMEvent( QWSIMEvent *e, QWidget *keywidget )
{
    if ( e->simpleData.type == QWSServer::IMMarkedText ) {
	retrieveMarkedText( keywidget );
	return;
    }
    
    //generate end event for previous widget if focus has changed
    //### should not happen
    if ( composeMode && activeWidget != keywidget && !sendEndToPrev ) {
	cleanup();
    }

    QString txt( e->text, e->simpleData.textLen );


    if ( e->simpleData.type == QWSServer::IMCompose ) {
	//generate start event if we haven't done so already
	if ( !composeMode ) {
	    QIMEvent out( QEvent::IMStart, "", -1 );
	    qt_sendSpontaneousEvent( keywidget, &out );
	    activeWidget = keywidget;
	    composeMode = TRUE;
	    if ( !composition )
		composition = new QString;
	}

	const int cpos = qMax(0, qMin(e->simpleData.cpos, int(txt.length())));
	const int selLen = qMin( e->simpleData.selLen, int(txt.length())-cpos);

	QIMEvent out( QEvent::IMCompose, txt, 
			     cpos, 
			     selLen );
	qt_sendSpontaneousEvent( keywidget, &out );

	*composition = txt;
	//qDebug( "IMCompose on widget %p", keywidget );
    } else if ( e->simpleData.type == QWSServer::IMEnd ) {
	//IMEnd also known as IMInput
	//Allow multiple IMEnd events: 
	//generate start event if we haven't seen one
	//but only if we actually need to send something.

	
	//qDebug("IM_END sendEndToPrev %d, preFocusW %p, focusW %p", sendEndToPrev, prevFocusW, activeWidget );


	if ( composeMode ) {
	    QWidget  *target = ( sendEndToPrev && prevFocusW ) ? prevFocusW : activeWidget;
	    QIMEvent out( QEvent::IMEnd, txt, e->simpleData.cpos );
	    qt_sendSpontaneousEvent( target, &out );
	} else if ( !txt.isEmpty() ) {
	    if ( sendEndToPrev && prevFocusW ) 
		keywidget = prevFocusW;
	    QIMEvent start( QEvent::IMStart, "", -1 );
	    qt_sendSpontaneousEvent( keywidget, &start );
	    QIMEvent end( QEvent::IMEnd, txt, e->simpleData.cpos );
	    qt_sendSpontaneousEvent( keywidget, &end );
	}
	composeMode = FALSE;
	if ( composition )
	    *composition = QString::null;

    } 
    sendEndToPrev = FALSE;
}

void QInputContext::reset( QWidget *f )
{
    if ( f ) {
	prevFocusW = f;
	sendEndToPrev = TRUE;
    } else {
	prevFocusW = 0;
    	sendEndToPrev = FALSE;
	composeMode = FALSE;
    }
    
    activeWidget = 0;
    
    //server is obliged to send an IMEnd event in response to this call
    QPaintDevice::qwsDisplay()->resetIM();
}

void QInputContext::setMicroFocusWidget(QWidget *w)
{
    if ( activeWidget && w != activeWidget )
	reset();

    activeWidget = w;
}


void QInputContext::notifyWidgetDeletion( QWidget *w )
{
    if ( w == activeWidget ) {
	reset();
    } else if ( sendEndToPrev && w == prevFocusW ) {
	sendEndToPrev =  FALSE;
	prevFocusW = 0;
    }
}



//Cleaning up if the IM hasn't done so
void QInputContext::cleanup()
{
    qDebug( "============= QInputContext::cleanup =========" );
    //send appropriate IMEnd event if necessary
    if ( composeMode ) {
	QIMEvent out( QEvent::IMEnd, *composition, -1 );
	qt_sendSpontaneousEvent( activeWidget, &out );
	activeWidget = 0;
	composeMode = FALSE;
	*composition = QString::null;
    }
    
    reset();
}

#endif //QT_NO_QWS_IM

/****************************************************************************
**
** Implementation of QInputContext class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qinputcontext_p.h"
#include "qstring.h"
#include "qwindowsystem_qws.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"


#ifndef QT_NO_QWS_IM
QWidget* QInputContext::focusWidget = 0;
QString* QInputContext::composition = 0;

void QInputContext::translateIMEvent( QWSIMEvent *e, QWidget *keywidget )
{
    //generate end event for previous widget if focus has changed
    if ( focusWidget && focusWidget != keywidget )
	reset();

    QString txt( e->text, e->simpleData.textLen );


    if ( e->simpleData.type == QWSServer::IMStart ) {
	//We may already have generated a start event for this widget
	//(could happen if the IM is buggy)
	if ( !focusWidget ) {
	    QIMEvent out( QEvent::IMStart, txt, e->simpleData.cpos );
	    QApplication::sendSpontaneousEvent( keywidget, &out );
	    focusWidget = keywidget;
	    if ( !composition )
		composition = new QString;
	}
    } else if ( e->simpleData.type == QWSServer::IMCompose ) {
	//generate start event if we haven't done so already
	//(focus change or careless IM)
	if ( !focusWidget ) {
	    QIMEvent out( QEvent::IMStart, "", -1 );
	    QApplication::sendSpontaneousEvent( keywidget, &out );
	    focusWidget = keywidget;
	    if ( !composition )
		composition = new QString;
	}

	QIMEvent out( QEvent::IMCompose, txt, e->simpleData.cpos, e->simpleData.selLen );
	QApplication::sendSpontaneousEvent( keywidget, &out );

	*composition = txt;
    } else if ( e->simpleData.type == QWSServer::IMEnd ) {
	//Make sure we don't send multiple end events (guard against buggy IM)
	if ( focusWidget ) {
	    QIMEvent out( QEvent::IMEnd, txt, e->simpleData.cpos );
	    QApplication::sendSpontaneousEvent( keywidget, &out );
	    focusWidget = 0;
	    *composition = QString::null;
	}
    }
}

void QInputContext::reset()
{
    //send appropriate IMEnd event if necessary
    if ( focusWidget ) {
	QIMEvent out( QEvent::IMEnd, *composition, -1 );
	QApplication::sendSpontaneousEvent( focusWidget, &out );
	focusWidget = 0;
	*composition = QString::null;
    }

    QPaintDevice::qwsDisplay()->resetIM();
}
#endif //QT_NO_QWS_IM

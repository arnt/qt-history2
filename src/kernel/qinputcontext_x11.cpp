/****************************************************************************
** $Id$
**
** Implementation of QInputContext class
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qplatformdefs.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#include <stdlib.h>
#include <limits.h>


int qt_compose_keycode = 0;

#if !defined(QT_NO_XIM)

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// from qapplication_x11.cpp
extern XIM	qt_xim;
extern XIMStyle	qt_xim_style;


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic) {
	    // qDebug("compose start: no qic");
	    return 0;
	}

	qic->composing = TRUE;
	qic->lastcompose = qic->text = QString::null;
	qic->focusWidget = 0;

	if ( qic->selectedChars.size() < 128 )
	    qic->selectedChars.resize( 128 );
	qic->selectedChars.fill( 0 );

	// qDebug("compose start");
	return 0;
    }

    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic) {
	    // qDebug("compose event: invalid compose event %p", qic);
	    return 0;
	}

	if (qApp->focusWidget() != qic->focusWidget && qic->text.isEmpty()) {
	    if (qic->focusWidget) {
		QIMEvent endevent(QEvent::IMEnd, qic->lastcompose, -1);
		QApplication::sendEvent(qic->focusWidget, &endevent);
	    }

	    qic->text = qic->lastcompose = QString::null;
	    qic->focusWidget = qApp->focusWidget();
	    qic->composing = FALSE;

	    if ( qic->selectedChars.size() < 128 )
		qic->selectedChars.resize( 128 );
	    qic->selectedChars.fill( 0 );

	    if (qic->focusWidget) {
		qic->composing = TRUE;
		QIMEvent startevent(QEvent::IMStart, QString::null, -1);
		QApplication::sendEvent(qic->focusWidget, &startevent);
	    }
	}

	if (! qic->composing || ! qic->focusWidget) {
	    // qDebug("compose event: invalid compose event %d %p",
	    // qic->composing, qic->focusWidget);
	    return 0;
	}

	XIMPreeditDrawCallbackStruct *drawstruct =
	    (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;
	int cursor = drawstruct->caret, sellen = 0;

	if (text) {
	    char *str = 0;
	    if (text->encoding_is_wchar) {
		int l = wcstombs(NULL, text->string.wide_char, text->length);
		if (l != -1) {
		    str = new char[l + 1];
		    wcstombs(str, text->string.wide_char, l);
		    str[l] = 0;
		}
	    } else
		str = text->string.multi_byte;

	    if (! str)
		return 0;

	    QString s = QString::fromLocal8Bit(str);

	    if (text->encoding_is_wchar)
		delete [] str;

	    if (drawstruct->chg_length < 0)
		qic->text.replace(drawstruct->chg_first, UINT_MAX, s);
	    else
		qic->text.replace(drawstruct->chg_first, drawstruct->chg_length, s);

	    if ( qic->selectedChars.size() < qic->text.length() ) {
		// expand the selectedChars array if the compose string is longer
		uint from = qic->selectedChars.size();
		qic->selectedChars.resize( qic->text.length() );
		for ( uint x = from; from < qic->selectedChars.size(); ++x )
		    qic->selectedChars[x] = 0;
	    }

	    uint x;
	    bool *p = qic->selectedChars.data() + drawstruct->chg_first;
	    // determine if the changed chars are selected based on text->feedback
	    for ( x = 0; x < s.length(); ++x )
		*p++ = ( text->feedback ? ( text->feedback[x] & XIMReverse ) : 0 );

	    // figure out where the selection starts, and how long it is
	    p = qic->selectedChars.data();
	    bool started = FALSE;
	    for ( x = 0; x < qic->selectedChars.size(); ++x ) {
		if ( started ) {
		    if ( *p ) ++sellen;
		    else break;
		} else {
		    if ( *p ) {
			cursor = x;
			started = TRUE;
			sellen = 1;
		    }
		}
		++p;
	    }
	} else {
	    if (drawstruct->chg_length == 0)
		drawstruct->chg_length = -1;

	    qic->text.remove(drawstruct->chg_first, drawstruct->chg_length);

	    KeySym sym = XKeycodeToKeysym(QPaintDevice::x11AppDisplay(),
					  qt_compose_keycode, 0);

	    if ( qic->text.isEmpty() ) {
		if ( sym == XK_Return ) {
		    // qDebug("user pressed return, send an IMEnd...");
		    QIMEvent endevent(QEvent::IMEnd, qic->lastcompose, -1);
		    QApplication::sendEvent(qic->focusWidget, &endevent);
		    qic->focusWidget = 0;
		    return 0;
		} else {
		    // qDebug("last char deleted, send an IMEnd with null string...");
		    QIMEvent endevent(QEvent::IMEnd, QString::null, -1);
		    QApplication::sendEvent(qic->focusWidget, &endevent);
		    qic->focusWidget = 0;
		    return 0;
		}
	    }
	}

	qic->lastcompose = qic->text;

	QIMComposeEvent event( QEvent::IMCompose, qic->text, cursor, sellen );
	QApplication::sendEvent(qic->focusWidget, &event);
	return 0;
    }

    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	if (qic->composing && qic->focusWidget) {
       	    QIMEvent event(QEvent::IMEnd, qic->lastcompose, -1);
	    QApplication::sendEvent(qic->focusWidget, &event);
	}

 	qic->lastcompose = QString::null;
	qic->composing = FALSE;
	qic->focusWidget = 0;

	if ( qic->selectedChars.size() < 128 )
	    qic->selectedChars.resize( 128 );
	qic->selectedChars.fill( 0 );

	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // !QT_NO_XIM



QInputContext::QInputContext(QWidget *widget)
    : ic(0), focusWidget(0), composing(FALSE), fontset(0)
{
#if !defined(QT_NO_XIM)
    if (! qt_xim) {
	qWarning("QInputContext: no input method context available");
	return;
    }

    if (! widget->isTopLevel()) {
	qWarning("QInputContext: cannot create input context for non-toplevel widgets");
	return;
    }

    XPoint spot;
    XRectangle rect;
    XVaNestedList preedit_attr = 0;
    XIMCallback startcallback, drawcallback, donecallback;

    int missCount;
    char** missList;
    char* defStr;
    fontset = XCreateFontSet(QPaintDevice::x11AppDisplay(), "-*-fixed-*--14-*",
			     &missList, &missCount, &defStr);
    if(missCount > 0)
	XFreeStringList(missList);
    font = QApplication::font();

    if (qt_xim_style & XIMPreeditArea) {
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->width();
	rect.height = widget->height();

	preedit_attr = XVaCreateNestedList(0,
					   XNArea, &rect,
					   XNFontSet, fontset,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditPosition) {
	spot.x = 1;
	spot.y = 1;

	preedit_attr = XVaCreateNestedList(0,
					   XNSpotLocation, &spot,
					   XNFontSet, fontset,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditCallbacks) {
	startcallback.client_data = (XPointer) this;
	startcallback.callback = (XIMProc) xic_start_callback;
	drawcallback.client_data = (XPointer) this;
	drawcallback.callback = (XIMProc)xic_draw_callback;
	donecallback.client_data = (XPointer) this;
	donecallback.callback = (XIMProc) xic_done_callback;

	preedit_attr = XVaCreateNestedList(0,
					   XNPreeditStartCallback, &startcallback,
					   XNPreeditDrawCallback, &drawcallback,
					   XNPreeditDoneCallback, &donecallback,
					   (char *) 0);
    }

    if (preedit_attr) {
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       XNPreeditAttributes, preedit_attr,
		       (char *) 0);
	XFree(preedit_attr);
    } else
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       (char *) 0);

    if (! ic)
	qFatal("Failed to create XIM input context!");

    // when resetting the input context, preserve the input state
    (void) XSetICValues((XIC) ic, XNResetState, XIMPreserveState, (char *) 0);
#endif // !QT_NO_XIM
}


QInputContext::~QInputContext()
{

#if !defined(QT_NO_XIM)
    if (ic)
	XDestroyIC((XIC) ic);

    if ( (qt_xim_style & XIMPreeditPosition) && fontset )
	XFreeFontSet( QPaintDevice::x11AppDisplay(), fontset );
#endif // !QT_NO_XIM

    ic = 0;
    focusWidget = 0;
    composing = FALSE;
}


void QInputContext::reset()
{
#if !defined(QT_NO_XIM)
    if (focusWidget && composing && ! text.isNull()) {
	// qDebug("QInputContext::reset: composing - sending IMEnd");

	QIMEvent endevent(QEvent::IMEnd, lastcompose, -1);
	QApplication::sendEvent(focusWidget, &endevent);
	focusWidget = 0;
	lastcompose = text = QString::null;
	if ( selectedChars.size() < 128 )
	    selectedChars.resize( 128 );
	selectedChars.fill( 0 );

	char *mb = XmbResetIC((XIC) ic);
	if (mb)
	    XFree(mb);
    }
#endif // !QT_NO_XIM
}


void QInputContext::setComposePosition(int x, int y)
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	XPoint point;
	point.x = x;
	point.y = y;

	XVaNestedList preedit_attr =
	    XVaCreateNestedList(0,
				XNSpotLocation, &point,
				XNFontSet, fontset,
				(char *) 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
	XFree(preedit_attr);
    }
#endif // !QT_NO_XIM
}


void QInputContext::setComposeArea(int x, int y, int w, int h)
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;

	XVaNestedList preedit_attr = XVaCreateNestedList(0,
							 XNArea, &rect,
							 XNFontSet, fontset,
							 (char *) 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
	XFree(preedit_attr);
    }
#endif
}


int QInputContext::lookupString(XKeyEvent *event, QCString &chars,
				KeySym *key, Status *status) const
{
    int count = 0;

#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	count = XmbLookupString((XIC) ic, event, chars.data(),
				chars.size(), key, status);

	if ((*status) == XBufferOverflow ) {
	    chars.resize(count + 1);
	    count = XmbLookupString((XIC) ic, event, chars.data(),
				    chars.size(), key, status);
	}
    }

#endif // QT_NO_XIM

    return count;
}

void QInputContext::setFocus()
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic)
	XSetICFocus((XIC) ic);
#endif // !QT_NO_XIM
}

void QInputContext::setXFontSet(QFont *f)
{
#if !defined(QT_NO_XIM)
    Display* dpy = QPaintDevice::x11AppDisplay();
    int missCount;
    char** missList;
    char* defStr;

    if ( ( f && font == *f) || ( font == QApplication::font() ) ) // nothing to do
	return;

    if (!f && fontset) // save the server roundtrip
        return;

    if (fontset) {
	XFreeFontSet(QPaintDevice::x11AppDisplay(),
		     fontset);
    }

    if (f) {
#if defined(QT_NO_XFTFREETYPE)
	fontset = XCreateFontSet(dpy, f->rawName().latin1(),
				 &missList, &missCount, &defStr);
#else // !QT_NO_XFTFREETYPE
	QString wght, slant;

	if (f->bold())
	    wght = QString::fromLatin1("bold");
	else
	    wght = QString::fromLatin1("medium");

	if (f->italic())
	    slant = QString::fromLatin1("i");
	else
	    slant = QString::fromLatin1("r");

	QString rawName = QString("-*-fixed-%1-%2-*-*-14-*").arg(wght).arg(slant);
	fontset = XCreateFontSet(dpy, rawName.latin1(),
				 &missList, &missCount, &defStr);
#endif // !QT_NO_XFTFREETYPE

	font = *f;
    } else {
	fontset = XCreateFontSet(dpy, "-*-fixed-medim-r-*-*-14-*",
				 &missList, &missCount, &defStr);
	font = QApplication::font();
    }

    if(missCount > 0)
	XFreeStringList(missList);
#endif // !QT_NO_XIM
}

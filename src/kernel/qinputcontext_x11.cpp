// Get the system specific includes and defines
#include "qplatformdefs.h"


#include "qapplication.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#include <stdlib.h>

#if !defined(QT_NO_XIM)

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

	qic->focusWidget = qApp->focusWidget();
	if (! qic->focusWidget) {
	    // qDebug("compose start: no focus widget");
	    return 0;
	}

	qic->composing = TRUE;
	qic->lastcompose = qic->text = QString::null;

	// qDebug("compose start: %p", qic->focusWidget);

	QIMEvent event(QEvent::IMStart, qic->text, -1);
	QApplication::sendEvent(qic->focusWidget, &event);
	return 0;
    }


    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic) {
	    // qDebug("compose event: invalid compose event %p", qic);
	    return 0;
	}

	if (qApp->focusWidget() != qic->focusWidget) {
	    if (qic->focusWidget) {
		QIMEvent endevent(QEvent::IMEnd, qic->lastcompose, -1);
		QApplication::sendEvent(qic->focusWidget, &endevent);
	    }

	    qic->text = qic->lastcompose = QString::null;
	    qic->focusWidget = qApp->focusWidget();

	    if (qic->focusWidget) {
		qic->composing = TRUE;
		QIMEvent startevent(QEvent::IMStart, qic->text, -1);
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

	    qic->lastcompose = qic->text;
	} else
	    qic->text.remove(drawstruct->chg_first, drawstruct->chg_length);

	QIMEvent event(QEvent::IMCompose, qic->text, drawstruct->caret);
	QApplication::sendEvent(qic->focusWidget, &event);
	return 0;
    }


    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic || ! qic->composing || ! qic->focusWidget)
	    return 0;

	// qDebug("compose done");

	QIMEvent event(QEvent::IMEnd, qic->lastcompose, -1);
	QApplication::sendEvent(qic->focusWidget, &event);

 	qic->lastcompose = QString::null;
	qic->composing = FALSE;
	qic->focusWidget = 0;

	return 0;
    }


    static int xic_caret_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic || ! qic->composing || ! qic->focusWidget)
	    return 0;

	XIMPreeditCaretCallbackStruct *caretstruct =
	    (XIMPreeditCaretCallbackStruct *) call_data;

	if (! caretstruct)
	    return 0;

	// qDebug("compose position: %d", caretstruct->position);

	// this is probably wrong
	QIMEvent event(QEvent::IMCompose, qic->text, caretstruct->position);
	QApplication::sendEvent(qic->focusWidget, &event);
	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // !QT_NO_XIM



QInputContext::QInputContext(QWidget *widget)
    : ic(0), focusWidget(0), composing(FALSE)
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
    XIMCallback startcallback, drawcallback, donecallback, caretcallback;

    if (qt_xim_style & XIMPreeditArea) {
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->width();
	rect.height = widget->height();

	preedit_attr = XVaCreateNestedList(0,
					   XNArea, &rect,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditPosition) {
	spot.x = 1;
	spot.y = 1;

	preedit_attr = XVaCreateNestedList(0,
					   XNSpotLocation, &spot,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditCallbacks) {
	startcallback.client_data = (XPointer) this;
	startcallback.callback = (XIMProc) xic_start_callback;
	drawcallback.client_data = (XPointer) this;
	drawcallback.callback = (XIMProc)xic_draw_callback;
	donecallback.client_data = (XPointer) this;
	donecallback.callback = (XIMProc) xic_done_callback;
	caretcallback.client_data = (XPointer) this;
	caretcallback.callback = (XIMProc) xic_caret_callback;

	preedit_attr = XVaCreateNestedList(0,
					   XNPreeditStartCallback, &startcallback,
					   XNPreeditDrawCallback, &drawcallback,
					   XNPreeditDoneCallback, &donecallback,
					   XNPreeditCaretCallback, &caretcallback,
					   (char *) 0);
    }

    if (preedit_attr)
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       XNPreeditAttributes, preedit_attr,
		       (char *) 0);
    else
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       (char *) 0);
#endif // !QT_NO_XIM
}


QInputContext::~QInputContext()
{
#if !defined(QT_NO_XIM)
    if (ic)
	XDestroyIC((XIC) ic);
#endif // !QT_NO_XIM
    ic = 0;
    focusWidget = 0;
    composing = FALSE;
}


void QInputContext::reset()
{
#if !defined(QT_NO_XIM)
    if (focusWidget && composing) {
	// qDebug("QInputContext::reset: composing - sending IMEnd");

	QIMEvent event(QEvent::IMEnd, lastcompose, -1);
	QApplication::sendEvent(focusWidget, &event);

	focusWidget = 0;
	composing = FALSE;
    }

    (void) XmbResetIC((XIC) ic);
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
	    XVaCreateNestedList(0, XNSpotLocation, &point, (char *) 0);
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

	XVaNestedList preedit_attr = XVaCreateNestedList(0, XNArea, &rect, (char *) 0);
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

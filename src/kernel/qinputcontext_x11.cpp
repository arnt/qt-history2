#include "qplatformdefs.h"

#include <qapplication.h>
#include <qwidget.h>
#include "qinputcontext_p.h"


#if !defined(NO_XIM)

// from qapplication_x11.cpp
extern XIM	qt_xim;
extern XIMStyle	qt_xim_style;


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qic->composing = TRUE;
	qic->lastcompose = qic->text = QString::null;

	QIMEvent event(QEvent::IMStart, qic->text, -1);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	XIMPreeditDrawCallbackStruct *drawstruct =
	    (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;

	// qDebug("xic_draw_callback: f %d l %d c %d t %p",
	// drawstruct->chg_first,
	// drawstruct->chg_length,
	// drawstruct->caret,
	// drawstruct->text);

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

	// qDebug("xic_draw_callback - string length %d, caret %d, data:",
	// qic->text.length(), drawstruct->caret);

	QIMEvent event(QEvent::IMCompose, qic->text, drawstruct->caret);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qic->composing = FALSE;

	// qDebug("xic_done_callback - string length %d, data:",
	// qic->lastcompose.length());

	QIMEvent event(QEvent::IMEnd, qic->lastcompose, -1);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_caret_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	XIMPreeditCaretCallbackStruct *caretstruct =
	    (XIMPreeditCaretCallbackStruct *) call_data;

	if (! caretstruct)
	    return 0;

	// this is probably wrong
	QIMEvent event(QEvent::IMCompose, qic->text, caretstruct->position);
	QApplication::sendEvent(widget, &event);
	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // !NO_XIM



QInputContext::QInputContext(QWidget *widget)
{
    ic = 0;

#if !defined(NO_XIM)
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
    XICCallback startcallback, drawcallback, donecallback, caretcallback;

    if (qt_xim_style & XIMPreeditArea) {
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->width();
	rect.height = widget->height();

	preedit_attr = XVaCreateNestedList(0,
					   XNArea, &rect,
					   0);
    } else if (qt_xim_style & XIMPreeditPosition) {
	spot.x = 1;
	spot.y = 1;

	preedit_attr = XVaCreateNestedList(0,
					   XNSpotLocation, &spot,
					   0);
    } else if (qt_xim_style & XIMPreeditCallbacks) {
	startcallback.client_data = (XPointer) this;
	startcallback.callback = xic_start_callback;
	drawcallback.client_data = (XPointer) this;
	drawcallback.callback = xic_draw_callback;
	donecallback.client_data = (XPointer) this;
	donecallback.callback = xic_done_callback;
	caretcallback.client_data = (XPointer) this;
	caretcallback.callback = xic_caret_callback;

	preedit_attr = XVaCreateNestedList(0,
					   XNPreeditStartCallback, &startcallback,
					   XNPreeditDrawCallback, &drawcallback,
					   XNPreeditDoneCallback, &donecallback,
					   XNPreeditCaretCallback, &caretcallback,
					   0);
    }

    if (preedit_attr)
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       XNPreeditAttributes, preedit_attr,
		       0);
    else
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       0);
#endif // Q_WS_X11 && !NO_XIM
}


QInputContext::~QInputContext()
{
#if !defined(NO_XIM)
    if (ic)
	XDestroyIC((XIC) ic);
#endif // Q_WS_X11 && !NO_XIM
    ic = 0;
}


void QInputContext::reset()
{
#if !defined(NO_XIM)
    if (composing)
	return;

    text.truncate(0);

    XIMPreeditState state = XIMPreeditUnKnown;
    XVaNestedList attr = XVaCreateNestedList(0, XNPreeditState, &state, NULL);
    bool st = FALSE;
    if (! XGetICValues((XIC) ic, XNPreeditAttributes, attr, NULL))
	st = TRUE;
    XFree(attr);

    (void) XmbResetIC((XIC) ic);

    attr = XVaCreateNestedList(0, XNPreeditState, st, 0);
    if (st)
	XSetICValues((XIC) ic, XNPreeditAttributes, attr, NULL);
    XFree(attr);
#endif // Q_WS_X11 && !NO_XIM
}


void QInputContext::setComposePosition(int x, int y)
{
#if !defined(NO_XIM)
    if (qt_xim && ic) {
	XPoint point;
	point.x = x;
	point.y = y;

	XVaNestedList preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &point, 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, 0);
	XFree(preedit_attr);
    }
#endif // !NO_XIM
}


void QInputContext::setComposeArea(int x, int y, int w, int h)
{
#if !defined(NO_XIM)
    if (qt_xim && ic) {
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;

	XVaNestedList preedit_attr = XVaCreateNestedList(0, XNArea, &rect, 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, 0);
	XFree(preedit_attr);
    }
#endif
}


int QInputContext::lookupString(XKeyEvent *event, QCString &chars,
				KeySym *key, Status *status) const
{
    int count = 0;

#if !defined(NO_XIM)
    if (qt_xim && ic) {
	count = XmbLookupString((XIC) ic, event, chars.data(),
				chars.size(), key, status);

	if ((*status) == XBufferOverflow ) {
	    chars.resize(count+1);
	    count = XmbLookupString((XIC) ic, event, chars.data(),
				    chars.size(), key, status);
	}
    }

#endif // NO_XIM

    return count;
}


void QInputContext::setFocus()
{
#if !defined(NO_XIM)
    if (qt_xim && ic)
	XSetICFocus((XIC) ic);
#endif // !NO_XIM
}

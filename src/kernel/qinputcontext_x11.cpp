#include <qapplication.h>
#include <qwidget.h>

#include "qinputcontext_p.h"

#include <stdlib.h> // for wcstombs


#if defined(Q_WS_X11) && !defined(NO_XIM)

// from qapplication_x11.cpp
extern XIM	qt_xim;
extern XIMStyle	qt_xim_style;


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS


    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	qDebug("xic_start_callback");

	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qDebug("xic_start_callback - ok %p %p", qic, widget);

	qic->setText(QString::null);

	QIMEvent event(QEvent::IMStart, QString::null, -1);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	qDebug("xic_draw_callback");

	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qDebug("xic_draw_callback - ok %p %p", qic, widget);

	XIMPreeditDrawCallbackStruct *drawstruct =
	    (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;

	int len = 0; // TODO: get length of reset string

	if (len && len == drawstruct->chg_length) {
	    qic->setText(QString::null);
	    return 0;
	}

	QString string;
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

	    if (str) {
		string = QString::fromLocal8Bit(str);

		if (text->encoding_is_wchar)
		    delete [] str;
	    }
	}

	qic->setText(string);

	QIMEvent event(QEvent::IMCompose, string, drawstruct->caret);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	qDebug("xic_done_callback");

	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qDebug("xic_done_callback - ok %p %p", qic, widget);

	QIMEvent event(QEvent::IMEnd, qic->text(), -1);
	QApplication::sendEvent(widget, &event);
	return 0;
    }


    static int xic_caret_callback(XIC, XPointer client_data, XPointer) {
	qDebug("xic_caret_callback");

	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	QWidget *widget = qApp->focusWidget();
	if (! widget)
	    return 0;

	qDebug("xic_caret_callback - ok %p %p", qic, widget);
	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // Q_WS_X11 && !NO_XIM


// We create an XFontSet for use with input method stuff - since QFont is
// now a logical font composed of several real XFonts, we need a way to
// tell the input method server what font to use
static XFontSet fixed_fontset = 0; // leaked once


static void cleanup_ffs()
{
    if ( fixed_fontset )
	XFreeFontSet(QPaintDevice::x11AppDisplay(), fixed_fontset);
    fixed_fontset = 0;
}


static XFontSet xic_fontset(int pt)
{
    if ( !fixed_fontset ) {
	QCString n;
	char **missing = 0;
	int nmissing = 0;
	n.sprintf( "-*-fixed-*-*-normal-*-*-%d-*-*-*-*-*-*,"
		   "-*-*-*-*-normal-*-*-%d-*-*-*-*-*-*,"
		   "-*-*-*-*-*-*-*-%d-*-*-*-*-*-*",
		   pt*10,
		   pt*10,
		   pt*10 );
	fixed_fontset = XCreateFontSet( QPaintDevice::x11AppDisplay(), n,
					&missing, &nmissing, 0 );
	qAddPostRoutine(cleanup_ffs);
    }
    return fixed_fontset;
}


QInputContext::QInputContext(QWidget *widget)
{
    ic = 0;

#if defined(Q_WS_X11) && !defined(NO_XIM)
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
    XFontSet fontset;
    XICCallback startcallback, drawcallback, donecallback, caretcallback;

    fontset = xic_fontset(QApplication::font().pointSize());

    if (qt_xim_style & XIMPreeditArea) {
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->width();
	rect.height = widget->height();

	preedit_attr = XVaCreateNestedList(0,
					   XNArea, &rect,
					   XNFontSet, fontset, 0);
	// qDebug("    preedit area");
    } else if (qt_xim_style & XIMPreeditPosition) {
	spot.x = 1;
	spot.y = 1;

	preedit_attr = XVaCreateNestedList(0,
					   XNSpotLocation, &spot,
					   XNFontSet, fontset,
					   0);
	// qDebug("    preedit position");
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
					   XNFontSet, fontset,
					   XNPreeditStartCallback, &startcallback,
					   XNPreeditDrawCallback, &drawcallback,
					   XNPreeditDoneCallback, &donecallback,
					   XNPreeditCaretCallback, &caretcallback,
					   0);

	/*
	  qDebug("    preedit callbacks %p %p %p %p",
	  startcallback.callback,
	  drawcallback.callback,
	  donecallback.callback,
	  caretcallback.callback);
	*/
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

    // qDebug("QInputContext: created %p %p", this, ic);

    // QKeyPressEventList = 0;
#endif // Q_WS_X11 && !NO_XIM
}


QInputContext::~QInputContext()
{
    // qDebug("QInputContext::~QInputContext");

#if defined(Q_WS_X11) && !defined(NO_XIM)
    if (ic)
	XDestroyIC(ic);
    ic = 0;
#endif // Q_WS_X11 && !NO_XIM

}


void QInputContext::reset()
{
    // qDebug("QInputContext::reset");

#if defined(Q_WS_X11) && !defined(NO_XIM)
    if (isComposing())
	return;

    setText(QString::null);

    XIMPreeditState state = XIMPreeditUnKnown;
    XVaNestedList attr = XVaCreateNestedList(0, XNPreeditState, &state, NULL);
    bool st = FALSE;
    if (! XGetICValues(ic, XNPreeditAttributes, attr, NULL))
	st = TRUE;
    XFree(attr);

    QString uncommitted = QString::fromLocal8Bit(XmbResetIC(ic));
    // TODO: save length

    attr = XVaCreateNestedList(0, XNPreeditState, st, 0);
    if (st)
	XSetICValues(ic, XNPreeditAttributes, attr, NULL);
    XFree(attr);
#endif // Q_WS_X11 && !NO_XIM
}


bool QInputContext::isComposing() const
{
    bool ret = FALSE;

#if defined(Q_WS_X11) && !defined(NO_XIM)
    if (qt_xim && ic) {
	if (qt_xim_style & XIMPreeditCallbacks) {
	    if (txt.length() > 0)
		ret = TRUE;
	} else {
	    XIMPreeditState state = XIMPreeditUnKnown;
	    XVaNestedList attr;

	    attr = XVaCreateNestedList(0, XNPreeditAttributes, state, 0);
	    if (! XGetICValues(ic, XNPreeditAttributes, attr, NULL)) {
		if (state == XIMPreeditEnable)
		    ret = TRUE;
	    } else
		ret = TRUE;
	    XFree(attr);
	}
    }
#endif // Q_WS_X11 && !NO_XIM

    return ret;
}


void QInputContext::setXFontSet(XFontSet fontset)
{
    if (qt_xim && ic) {
	XVaNestedList attr = XVaCreateNestedList(0, XNFontSet, fontset, NULL);
	XSetICValues(ic, XNPreeditAttributes, attr, 0);
	XFree(attr);
    }
}


void QInputContext::setComposePosition(int x, int y)
{
    if (qt_xim && ic) {
	XPoint point;
	point.x = x;
	point.y = y;

	// XSetICValues(ic, XNSpotLocation, &point, 0);
    }
}


void QInputContext::setComposeArea(int x, int y, int w, int h)
{
    if (qt_xim && ic) {
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;

	// XSetICValues(ic, XNArea, &rect, 0);
    }
}


int QInputContext::lookupString(XKeyEvent *event, QCString &chars,
				KeySym *key, Status *status) const
{
    int count = 0;

    if (qt_xim && ic) {
	count = XmbLookupString(ic, event, chars.data(), chars.size(), key, status);

	if ((*status) == XBufferOverflow ) {
	    chars.resize(count+1);
	    count = XmbLookupString(ic, event, chars.data(), chars.size(), key, status);
	}
    }

    return count;
}


void QInputContext::setFocus()
{
    if (qt_xim && ic)
	XSetICFocus(ic);
}

/****************************************************************************
** $Id$
**
** Implementation of QXIMInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

/****************************************************************************
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the input method module of the Qt Toolkit.
**
** Licensees holding valid Qt Preview licenses may use this file in
** accordance with the Qt Preview License Agreement provided with the
** Software.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qximinputcontext_p.h"

#if !defined(QT_NO_IM)

#include "qplatformdefs.h"

#include "qapplication.h"
#include "qwidget.h"
#include "qstring.h"
#include "qlist.h"
#include "qtextcodec.h"

#include <stdlib.h>
#include <limits.h>
#include <qdebug.h>

#include "qx11info_x11.h"

bool QXIMInputContext::isInitialized = false;
XIM QXIMInputContext::xim = 0;
XIMStyle QXIMInputContext::xim_style = 0;
QList<QXIMInputContext *> *QXIMInputContext::ximContexts = 0;

#if !defined(QT_NO_XIM)

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// #define QT_XIM_DEBUG

// moved from qapplication_x11.cpp
static const XIMStyle xim_default_style = XIMPreeditCallbacks | XIMStatusNothing;

// refers to qapplication_x11.cpp
extern XIMStyle	qt_xim_preferred_style;
extern char    *qt_ximServer;
#endif
extern int qt_ximComposingKeycode;
extern QTextCodec * qt_input_mapper;


#if !defined(QT_NO_XIM)

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif // Q_C_CALLBACKS

#ifdef USE_X11R6_XIM
    static void xim_create_callback(XIM /*im*/,
                                    XPointer /*client_data*/,
                                    XPointer /*call_data*/)
    {
        // qDebug("xim_create_callback");
        QXIMInputContext::create_xim();
    }

    static void xim_destroy_callback(XIM /*im*/,
                                     XPointer /*client_data*/,
                                     XPointer /*call_data*/)
    {
        // qDebug("xim_destroy_callback");
        QXIMInputContext::close_xim();
        XRegisterIMInstantiateCallback(X11->display, 0, 0, 0,
                                       (XIMProc) xim_create_callback, 0);
    }

#endif // USE_X11R6_XIM

#if defined(Q_C_CALLBACKS)
}
#endif // Q_C_CALLBACKS

#endif // QT_NO_XIM

#ifndef QT_NO_XIM

/* The cache here is needed, as X11 leaks a few kb for every
   XFreeFontSet call, so we avoid creating and deletion of fontsets as
   much as possible
*/
static XFontSet fontsetCache[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int fontsetRefCount = 0;

static const char * const fontsetnames[] = {
    "-*-fixed-medium-r-*-*-16-*,-*-*-medium-r-*-*-16-*",
    "-*-fixed-medium-i-*-*-16-*,-*-*-medium-i-*-*-16-*",
    "-*-fixed-bold-r-*-*-16-*,-*-*-bold-r-*-*-16-*",
    "-*-fixed-bold-i-*-*-16-*,-*-*-bold-i-*-*-16-*",
    "-*-fixed-medium-r-*-*-24-*,-*-*-medium-r-*-*-24-*",
    "-*-fixed-medium-i-*-*-24-*,-*-*-medium-i-*-*-24-*",
    "-*-fixed-bold-r-*-*-24-*,-*-*-bold-r-*-*-24-*",
    "-*-fixed-bold-i-*-*-24-*,-*-*-bold-i-*-*-24-*"
};

static XFontSet getFontSet(const QFont &f)
{
    int i = 0;
    if (f.italic())
        i |= 1;
    if (f.bold())
        i |= 2;

    if (f.pointSize() > 20)
        i += 4;

    if (!fontsetCache[i]) {
        Display* dpy = X11->display;
        int missCount;
        char** missList;
        fontsetCache[i] = XCreateFontSet(dpy, fontsetnames[i], &missList, &missCount, 0);
        if(missCount > 0)
            XFreeStringList(missList);
        if (!fontsetCache[i]) {
            fontsetCache[i] = XCreateFontSet(dpy,  "-*-fixed-*-*-*-*-16-*", &missList, &missCount, 0);
            if(missCount > 0)
                XFreeStringList(missList);
            if (!fontsetCache[i])
                fontsetCache[i] = (XFontSet)-1;
        }
    }
    return (fontsetCache[i] == (XFontSet)-1) ? 0 : fontsetCache[i];
}


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

    // These static functions should be rewritten as member of
    // QXIMInputContext

    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (! qic) {
#ifdef QT_XIM_DEBUG
	    qDebug("compose start: no qic");
#endif // QT_XIM_DEBUG

	    return 0;
	}

	qic->resetClientState();
	qic->sendIMEvent( QEvent::InputMethodStart );

#ifdef QT_XIM_DEBUG
	qDebug("compose start");
#endif // QT_XIM_DEBUG

	return 0;
    }

    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (! qic) {
#ifdef QT_XIM_DEBUG
	    qDebug("compose event: invalid compose event %p", qic);
#endif // QT_XIM_DEBUG

	    return 0;
	}

	bool send_imstart = FALSE;
	if( ! qic->isComposing() && qic->hasFocus() ) {
	    qic->resetClientState();
	    send_imstart = TRUE;
	} else if ( ! qic->isComposing() || ! qic->hasFocus() ) {
#ifdef QT_XIM_DEBUG
	    qDebug( "compose event: invalid compose event composing=%d hasFocus=%d",
		    qic->isComposing(), qic->hasFocus() );
#endif // QT_XIM_DEBUG

	    return 0;
	}

	if ( send_imstart )
	    qic->sendIMEvent( QEvent::InputMethodStart );

	XIMPreeditDrawCallbackStruct *drawstruct =
	    (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;
	int cursor = drawstruct->caret, sellen = 0;

	if ( ! drawstruct->caret && ! drawstruct->chg_first &&
	     ! drawstruct->chg_length && ! text ) {
	    if( qic->composingText.isEmpty() ) {
#ifdef QT_XIM_DEBUG
		qDebug( "compose emptied" );
#endif // QT_XIM_DEBUG
		// if the composition string has been emptied, we need
		// to send an InputMethodEnd event
		qic->sendIMEvent( QEvent::InputMethodEnd );
		qic->resetClientState();
		// if the commit string has coming after here, InputMethodStart
		// will be sent dynamically
	    }
	    return 0;
	}

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
		qic->composingText.replace(drawstruct->chg_first, UINT_MAX, s);
	    else
		qic->composingText.replace(drawstruct->chg_first, drawstruct->chg_length, s);

	    if ( qic->selectedChars.size() < qic->composingText.length() ) {
		// expand the selectedChars array if the compose string is longer
		int from = qic->selectedChars.size();
		qic->selectedChars.resize( qic->composingText.length() );
                for (int x = from; from < qic->selectedChars.size(); ++x)
                    qic->selectedChars.clearBit(x);
	    }

            // determine if the changed chars are selected based on text->feedback
            for (int x = 0; x < s.length(); ++x)
                qic->selectedChars.setBit(x, (text->feedback ?
                                              (text->feedback[x] & XIMReverse) : 0));

            // figure out where the selection starts, and how long it is
            bool started = false;
            for (int x = 0; x < qic->selectedChars.size(); ++x) {
                if (started) {
                    if (qic->selectedChars.testBit(x)) ++sellen;
                    else break;
                } else {
                    if (qic->selectedChars.testBit(x)) {
                        cursor = x;
                        started = true;
                        sellen = 1;
                    }
                }
            }
	} else {
	    if (drawstruct->chg_length == 0)
		drawstruct->chg_length = -1;

	    qic->composingText.remove(drawstruct->chg_first, drawstruct->chg_length);
	    bool qt_compose_emptied = qic->composingText.isEmpty();
	    if ( qt_compose_emptied ) {
#ifdef QT_XIM_DEBUG
		qDebug( "compose emptied" );
#endif // QT_XIM_DEBUG
		// if the composition string has been emptied, we need
		// to send an InputMethodEnd event
		qic->sendIMEvent( QEvent::InputMethodEnd );
		qic->resetClientState();
		// if the commit string has coming after here, InputMethodStart
		// will be sent dynamically
		return 0;
	    }
	}

	qic->sendIMEvent( QEvent::InputMethodCompose, qic->composingText, cursor, sellen );

	return 0;
    }

    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (! qic)
	    return 0;

	// Don't send InputMethodEnd here. QXIMInputContext::x11FilterEvent()
	// handles InputMethodEnd with commit string.
#if 0
	if ( qic->isComposing() )
	    qic->sendIMEvent( QEvent::InputMethodEnd );
	qic->resetClientState();
#endif

	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // !QT_NO_XIM



QXIMInputContext::QXIMInputContext()
    : QInputContext(), ic(0), fontset(0)
{
    if(!isInitialized)
	QXIMInputContext::init_xim();
}


void QXIMInputContext::setHolderWidget( QWidget *widget )
{
    if ( ! widget )
	return;

    QInputContext::setHolderWidget( widget );

#if !defined(QT_NO_XIM)
    fontsetRefCount++;
    if (! xim) {
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

    font = widget->font();
    fontset = getFontSet(font);

    if (xim_style & XIMPreeditArea) {
        rect.x = 0;
        rect.y = 0;
        rect.width = widget->width();
        rect.height = widget->height();

        preedit_attr = XVaCreateNestedList(0,
                                           XNArea, &rect,
                                           XNFontSet, fontset,
                                           (char *) 0);
    } else if (xim_style & XIMPreeditPosition) {
        spot.x = 1;
        spot.y = 1;

        preedit_attr = XVaCreateNestedList(0,
                                           XNSpotLocation, &spot,
                                           XNFontSet, fontset,
                                           (char *) 0);
    } else if (xim_style & XIMPreeditCallbacks) {
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
        ic = XCreateIC(xim,
                       XNInputStyle, xim_style,
                       XNClientWindow, widget->winId(),
                       XNPreeditAttributes, preedit_attr,
                       (char *) 0);
        XFree(preedit_attr);
    } else
        ic = XCreateIC(xim,
                       XNInputStyle, xim_style,
                       XNClientWindow, widget->winId(),
                       (char *) 0);

    if (! ic)
        qFatal("Failed to create XIM input context!");

    // when resetting the input context, preserve the input state
    (void) XSetICValues((XIC) ic, XNResetState, XIMPreserveState, (char *) 0);

    if(! ximContexts)
        ximContexts = new QList<QXIMInputContext *>;
    ximContexts->append(this);
#endif // !QT_NO_XIM
}


QXIMInputContext::~QXIMInputContext()
{
#if !defined(QT_NO_XIM)
    if (ic)
        XDestroyIC((XIC) ic);

    if (--fontsetRefCount == 0) {
        Display *dpy = X11->display;
        for (int i = 0; i < 8; i++) {
            if (fontsetCache[i] && fontsetCache[i] != (XFontSet)-1) {
                XFreeFontSet(dpy, fontsetCache[i]);
                fontsetCache[i] = 0;
            }
        }
    }

    if( ximContexts ) {
        ximContexts->removeAll( this );
        if(ximContexts->isEmpty()) {
            // Calling XCloseIM gives a Purify FMR error
            // XCloseIM(xim);
            // We prefer a less serious memory leak
            if(xim) {
                xim = 0;
                isInitialized = false;
            }

            delete ximContexts;
            ximContexts = 0;
        }
    }
#endif // !QT_NO_XIM

    ic = 0;
}

void QXIMInputContext::init_xim()
{
#ifndef QT_NO_XIM
        if (! qt_xim_preferred_style) // no configured input style, use the default
            qt_xim_preferred_style = xim_default_style;

        xim = 0;
        QString ximServerName(qt_ximServer);
        if (qt_ximServer)
            ximServerName.prepend("@im=");
        else
            ximServerName = "";

        if (!XSupportsLocale())
            qWarning("Qt: Locales not supported on X server");

#ifdef USE_X11R6_XIM
        else if (XSetLocaleModifiers (ximServerName.ascii()) == 0)
            qWarning("Qt: Cannot set locale modifiers: %s",
                      ximServerName.ascii());
        else /* if (! noxim) */
            XRegisterIMInstantiateCallback(X11->display, 0, 0, 0,
                                           (XIMProc) xim_create_callback, 0);
#else // !USE_X11R6_XIM
        else if (XSetLocaleModifiers ("") == 0)
            qWarning("Qt: Cannot set locale modifiers");
        else /* if (! noxim) */
            QXIMInputContext::create_xim();
#endif // USE_X11R6_XIM

	isInitialized = true;
#endif // QT_NO_XIM
}


/*! \internal
  Creates the application input method.
 */
void QXIMInputContext::create_xim()
{
#ifndef QT_NO_XIM
    xim = XOpenIM(X11->display, 0, 0, 0);
    if (xim) {

#ifdef USE_X11R6_XIM
        XIMCallback destroy;
        destroy.callback = (XIMProc) xim_destroy_callback;
        destroy.client_data = 0;
        if (XSetIMValues(xim, XNDestroyCallback, &destroy, (char *) 0) != 0)
            qWarning("Xlib dosn't support destroy callback");
#endif // USE_X11R6_XIM

        XIMStyles *styles = 0;
        XGetIMValues(xim, XNQueryInputStyle, &styles, (char *) 0, (char *) 0);
        if (styles) {
            int i;
            for (i = 0; !xim_style && i < styles->count_styles; i++) {
                if (styles->supported_styles[i] == qt_xim_preferred_style) {
                    xim_style = qt_xim_preferred_style;
                    break;
                }
            }
            // if the preferred input style couldn't be found, look for
            // Nothing
            for (i = 0; !xim_style && i < styles->count_styles; i++) {
                if (styles->supported_styles[i] == (XIMPreeditNothing |
                                                     XIMStatusNothing)) {
                    xim_style = XIMPreeditNothing | XIMStatusNothing;
                    break;
                }
            }
            // ... and failing that, None.
            for (i = 0; !xim_style && i < styles->count_styles; i++) {
                if (styles->supported_styles[i] == (XIMPreeditNone |
                                                     XIMStatusNone)) {
                    xim_style = XIMPreeditNone | XIMStatusNone;
                    break;
                }
            }

            // qDebug("QApplication: using im style %lx", xim_style);
            XFree((char *)styles);
        }

        if (xim_style) {

#ifdef USE_X11R6_XIM
            XUnregisterIMInstantiateCallback(X11->display, 0, 0, 0,
                                             (XIMProc) xim_create_callback, 0);
#endif // USE_X11R6_XIM

	    // following code fragment is not required for immodule
	    // version of XIM
#if 0
            QWidgetList list = qApp->topLevelWidgets();
            for (int i = 0; i < list.size(); ++i) {
                QWidget *w = list.at(i);
                w->d->createTLSysExtra();
            }
#endif
        } else {
            // Give up
            qWarning("No supported input style found."
                      "  See InputMethod documentation.");
            close_xim();
        }
    }
#endif // QT_NO_XIM
}


/*! \internal
  Closes the application input method.
*/
void QXIMInputContext::close_xim()
{
#ifndef QT_NO_XIM
    QString errMsg( "QXIMInputContext::close_xim() has been called" );

    // Calling XCloseIM gives a Purify FMR error
    // XCloseIM(xim);
    // We prefer a less serious memory leak

    xim = 0;
    // following code fragment is not required for immodule
    // version of XIM
#if 0
    QWidgetList list = qApp->topLevelWidgets();
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->d->destroyInputContext();
#endif
    if(ximContexts) {
        QList<QXIMInputContext *> contexts(*ximContexts);
        QList<QXIMInputContext *>::Iterator it = contexts.begin();
        while(it != contexts.end()) {
            (*it)->close(errMsg);
            ++it;
        }
        // ximContexts will be deleted in ~QXIMInputContext
    }
#endif // QT_NO_XIM
}


bool QXIMInputContext::x11FilterEvent( QWidget *keywidget, XEvent *event )
{
#ifndef QT_NO_XIM
    int xkey_keycode = event->xkey.keycode;
    if (XFilterEvent(event, keywidget->topLevelWidget()->winId())) {
        qt_ximComposingKeycode = xkey_keycode; // ### not documented in xlib

        return true;
    } else if ( focusWidget() ) {
        if ( event->type == XKeyPress && event->xkey.keycode == 0 ) {
            // input method has sent us a commit string
            QByteArray data;
            data.resize(513);
            KeySym sym;    // unused
            Status status; // unused
            QString text;
            int count = lookupString(&(event->xkey), data,
                                     &sym, &status);
            if (count > 0)
                text = qt_input_mapper->toUnicode(data.constData() , count);

            if (!(xim_style & XIMPreeditCallbacks) || !isComposing()) {
                // ############### send a regular key event here!
                // there is no composing state
                sendIMEvent(QEvent::InputMethodStart);
            }

            sendIMEvent(QEvent::InputMethodEnd, text);
            resetClientState();

            return true;
        }
    }
#endif // !QT_NO_XIM

    return false;
}


void QXIMInputContext::sendIMEvent( QEvent::Type type, const QString &text,
				    int cursorPosition, int selLength )
{
    QInputContext::sendIMEvent( type, text, cursorPosition, selLength );
    if ( type == QEvent::InputMethodCompose )
	composingText = text;
}


void QXIMInputContext::reset()
{
#if !defined(QT_NO_XIM)
    if (focusWidget() && isComposing() && ! composingText.isNull()) {
#ifdef QT_XIM_DEBUG
	qDebug("QXIMInputContext::reset: composing - sending InputMethodEnd (empty) to %p",
	       focusWidget());
#endif // QT_XIM_DEBUG

	QInputContext::reset();
	resetClientState();

	char *mb = XmbResetIC((XIC) ic);
	if (mb)
	    XFree(mb);
    }
#endif // !QT_NO_XIM
}


void QXIMInputContext::resetClientState()
{
#if !defined(QT_NO_XIM)
    composingText = QString::null;
    if (selectedChars.size() < 128)
        selectedChars.resize(128);
    selectedChars.fill(0);
#endif // !QT_NO_XIM
}


void QXIMInputContext::close( const QString &errMsg )
{
    qDebug( errMsg.utf8() );
    delete this;
}


bool QXIMInputContext::hasFocus() const
{
    return ( focusWidget() != 0 );
}


void QXIMInputContext::setMicroFocus(const QRect &r, const QFont &f)
{
    QWidget *widget = focusWidget();
    if ( xim && widget ) {
	QPoint p( r.x(), r.y() );
	QPoint p2 = widget->mapTo( widget->topLevelWidget(), QPoint( 0, 0 ) );
	p = widget->topLevelWidget()->mapFromGlobal( p );
	setXFontSet(f);
	setComposePosition(p.x(), p.y() + r.height());
	setComposeArea(p2.x(), p2.y(), widget->width(), widget->height());
    }

}

void QXIMInputContext::mouseHandler( int , QMouseEvent *event)
{
    if ( event->type() == QEvent::MouseButtonPress ||
	 event->type() == QEvent::MouseButtonDblClick ) {
	// Don't reset Japanese input context here. Japanese input
	// context sometimes contains a whole paragraph and has
	// minutes of lifetime different to ephemeral one in other
	// languages. The input context should be survived until
	// focused again.
	if ( !isPreeditPreservationEnabled() )
	    reset();
    }
}

void QXIMInputContext::setComposePosition(int x, int y)
{
#if !defined(QT_NO_XIM)
    if (xim && ic) {
        XPoint point;
        point.x = x;
        point.y = y;

        XVaNestedList preedit_attr =
            XVaCreateNestedList(0,
                                XNSpotLocation, &point,

                                (char *) 0);
        XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
        XFree(preedit_attr);
    }
#endif // !QT_NO_XIM
}


void QXIMInputContext::setComposeArea(int x, int y, int w, int h)
{
#if !defined(QT_NO_XIM)
    if (xim && ic) {
        XRectangle rect;
        rect.x = x;
        rect.y = y;
        rect.width = w;
        rect.height = h;

        XVaNestedList preedit_attr = XVaCreateNestedList(0,
                                                         XNArea, &rect,

                                                         (char *) 0);
        XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
        XFree(preedit_attr);
    }
#endif
}


void QXIMInputContext::setXFontSet(const QFont &f)
{
#if !defined(QT_NO_XIM)
    if (font == f) return; // nothing to do
    font = f;

    XFontSet fs = getFontSet(font);
    if (fontset == fs) return; // nothing to do
    fontset = fs;

    XVaNestedList preedit_attr = XVaCreateNestedList(0, XNFontSet, fontset, (char *) 0);
    XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
    XFree(preedit_attr);
#else
    Q_UNUSED(f);
#endif
}


int QXIMInputContext::lookupString(XKeyEvent *event, QByteArray &chars,
				KeySym *key, Status *status) const
{
    int count = 0;

#if !defined(QT_NO_XIM)
    if (xim && ic) {
        count = XmbLookupString((XIC) ic, event, chars.data(),
                                chars.size(), key, status);

        if ((*status) == XBufferOverflow) {
            chars.resize(count + 1);
            count = XmbLookupString((XIC) ic, event, chars.data(),
                                    chars.size(), key, status);
        }
    }

#endif // QT_NO_XIM

    return count;
}

void QXIMInputContext::setFocus()
{
#if !defined(QT_NO_XIM)
    if (xim && ic)
        XSetICFocus((XIC) ic);
#endif // !QT_NO_XIM
}

void QXIMInputContext::unsetFocus()
{
#if !defined(QT_NO_XIM)
    if (xim && ic)
	XUnsetICFocus((XIC) ic);
#endif // !QT_NO_XIM

    // Don't reset Japanese input context here. Japanese input context
    // sometimes contains a whole paragraph and has minutes of
    // lifetime different to ephemeral one in other languages. The
    // input context should be survived until focused again.
    if ( ! isPreeditPreservationEnabled() )
	reset();
}


bool QXIMInputContext::isPreeditRelocationEnabled()
{
    return ( language() == "ja" );
}


bool QXIMInputContext::isPreeditPreservationEnabled()
{
    return ( language() == "ja" );
}


QString QXIMInputContext::identifierName()
{
    // the name should be "xim" rather than "XIM" to be consistent
    // with corresponding immodule of GTK+
    return "xim";
}


QString QXIMInputContext::language()
{
#if !defined(QT_NO_XIM)
    if ( xim ) {
	QString locale( XLocaleOfIM( xim ) );

	if ( locale.startsWith( "zh" ) ) {
	    // Chinese language should be formed as "zh_CN", "zh_TW", "zh_HK"
	    _language = locale.left( 5 );
	} else {
	    // other languages should be two-letter ISO 639 language code
	    _language = locale.left( 2 );
	}
    }
#endif
    return _language;
}

#endif //QT_NO_IM

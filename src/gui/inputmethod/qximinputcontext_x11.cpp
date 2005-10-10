/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
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

#include "qdebug.h"
#include "qximinputcontext_p.h"

#if !defined(QT_NO_IM)

#if !defined(QT_NO_XIM)

#include "qplatformdefs.h"

#include "qapplication.h"
#include "qwidget.h"
#include "qstring.h"
#include "qlist.h"
#include "qtextcodec.h"
#include "qevent.h"
#include "qtextformat.h"

#include "qx11info_x11.h"

#include <stdlib.h>
#include <limits.h>

// #define QT_XIM_DEBUG
#ifdef QT_XIM_DEBUG
#define XIM_DEBUG qDebug
#else
#define XIM_DEBUG if (0) qDebug
#endif

// from qapplication_x11.cpp
// #### move to X11 struct
extern XIMStyle	qt_xim_preferred_style;
extern char    *qt_ximServer;
extern int qt_ximComposingKeycode;
extern QTextCodec * qt_input_mapper;

XIMStyle QXIMInputContext::xim_style = 0;
// moved from qapplication_x11.cpp
static const XIMStyle xim_default_style = XIMPreeditCallbacks | XIMStatusNothing;


extern "C" {
#ifdef USE_X11R6_XIM
    static void xim_create_callback(XIM /*im*/,
                                    XPointer client_data,
                                    XPointer /*call_data*/)
    {
        QXIMInputContext *qic = reinterpret_cast<QXIMInputContext *>(client_data);
        // qDebug("xim_create_callback");
        qic->create_xim();
    }

    static void xim_destroy_callback(XIM /*im*/,
                                     XPointer client_data,
                                     XPointer /*call_data*/)
    {
        QXIMInputContext *qic = reinterpret_cast<QXIMInputContext *>(client_data);
        // qDebug("xim_destroy_callback");
        qic->close_xim();
        XRegisterIMInstantiateCallback(X11->display, 0, 0, 0,
                                       (XIMProc) xim_create_callback, reinterpret_cast<char *>(qic));
    }
#endif // USE_X11R6_XIM

    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (!qic) {
	    XIM_DEBUG("xic_start_callback: no qic");
	    return 0;
	}
        QXIMInputContext::ICData *data = qic->icData();
        if (!data) {
            XIM_DEBUG("xic_start_callback: no ic data");
            return 0;
        }
	XIM_DEBUG("xic_start_callback");

	data->clear();
        data->composing = true;

	return 0;
    }

    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (!qic) {
	    XIM_DEBUG("xic_draw_callback: no qic");
	    return 0;
	}
        QXIMInputContext::ICData *data = qic->icData();
	if (!data) {
	    XIM_DEBUG("xic_draw_callback: no ic data");
	    return 0;
	}
        XIM_DEBUG("xic_draw_callback");


	if(!data->composing) {
	    data->clear();
            data->composing = true;
        }

	XIMPreeditDrawCallbackStruct *drawstruct = (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;
	int cursor = drawstruct->caret, sellen = 0;

	if (!drawstruct->caret && !drawstruct->chg_first && !drawstruct->chg_length && !text) {
	    if(data->text.isEmpty()) {
		XIM_DEBUG("compose emptied");
		// if the composition string has been emptied, we need
		// to send an InputMethodEnd event
                QInputMethodEvent e;
		qic->sendEvent(e);
		data->clear();

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

	    if (!str)
		return 0;

	    QString s = QString::fromLocal8Bit(str);

	    if (text->encoding_is_wchar)
		delete [] str;

	    if (drawstruct->chg_length < 0)
		data->text.replace(drawstruct->chg_first, UINT_MAX, s);
	    else
		data->text.replace(drawstruct->chg_first, drawstruct->chg_length, s);

	    if (data->selectedChars.size() < data->text.length()) {
		// expand the selectedChars array if the compose string is longer
		int from = data->selectedChars.size();
		data->selectedChars.resize(data->text.length());
                for (int x = from; x < data->selectedChars.size(); ++x)
                    data->selectedChars.clearBit(x);
	    }

            // determine if the changed chars are selected based on text->feedback
            for (int x = 0; x < text->length; ++x)
                data->selectedChars.setBit(x + drawstruct->chg_first,
                                           (text->feedback ? (text->feedback[x] & XIMReverse) : 0));

            // figure out where the selection starts, and how long it is
            bool started = false;
            for (int x = 0; x < qMin(data->selectedChars.size(), data->text.length()); ++x) {
                if (started) {
                    if (data->selectedChars.testBit(x)) ++sellen;
                    else break;
                } else {
                    if (data->selectedChars.testBit(x)) {
                        cursor = x;
                        started = true;
                        sellen = 1;
                    }
                }
            }
	} else {
	    if (drawstruct->chg_length == 0)
		drawstruct->chg_length = -1;

	    data->text.remove(drawstruct->chg_first, drawstruct->chg_length);
	    bool qt_compose_emptied = data->text.isEmpty();
	    if (qt_compose_emptied) {
		XIM_DEBUG("compose emptied 2 text=%s", data->text.toUtf8().constData());
		// if the composition string has been emptied, we need
		// to send an InputMethodEnd event
                QInputMethodEvent e;
		qic->sendEvent(e);
		data->clear();
		// if the commit string has coming after here, InputMethodStart
		// will be sent dynamically
		return 0;
	    }
	}

        if (!sellen)
            cursor = data->text.length();
        XIM_DEBUG("sending compose: '%s', cursor=%d, sellen=%d",
                  data->text.toUtf8().constData(), cursor, sellen);
        QList<QInputMethodEvent::Attribute> attrs;
        if (cursor > 0)
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, cursor,
                               qic->standardFormat(QInputContext::PreeditFormat));
        if (sellen)
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, cursor, sellen,
                               qic->standardFormat(QInputContext::SelectionFormat));
        if (cursor + sellen < data->text.length())
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                                     cursor + sellen, data->text.length() - cursor - sellen,
                                     qic->standardFormat(QInputContext::PreeditFormat));
        QInputMethodEvent e(data->text, attrs);
	qic->sendEvent(e);

	return 0;
    }

    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QXIMInputContext *qic = (QXIMInputContext *) client_data;
	if (!qic)
	    return 0;

        XIM_DEBUG("xic_done_callback");
	// Don't send InputMethodEnd here. QXIMInputContext::x11FilterEvent()
	// handles InputMethodEnd with commit string.
	return 0;
    }
}

void QXIMInputContext::ICData::clear()
{
    text = QString();
    selectedChars.clear();
    composing = false;
}

QXIMInputContext::ICData *QXIMInputContext::icData() const
{
    return ximData.value(focusWidget());
}
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
            fontsetCache[i] = XCreateFontSet(dpy, "-*-fixed-*-*-*-*-16-*", &missList, &missCount, 0);
            if(missCount > 0)
                XFreeStringList(missList);
            if (!fontsetCache[i])
                fontsetCache[i] = (XFontSet)-1;
        }
    }
    return (fontsetCache[i] == (XFontSet)-1) ? 0 : fontsetCache[i];
}



QXIMInputContext::QXIMInputContext()
{
    if (!qt_xim_preferred_style) // no configured input style, use the default
        qt_xim_preferred_style = xim_default_style;

    xim = 0;
    QByteArray ximServerName(qt_ximServer);
    if (qt_ximServer)
        ximServerName.prepend("@im=");
    else
        ximServerName = "";

#ifndef QT_NO_DEBUG
    if (!XSupportsLocale())
        qWarning("Qt: Locale not supported on X server");
#endif
    
#ifdef USE_X11R6_XIM
    else if (XSetLocaleModifiers (ximServerName.constData()) == 0)
        qWarning("Qt: Cannot set locale modifiers: %s", ximServerName.constData());
    else /* if (!noxim) */
        XRegisterIMInstantiateCallback(X11->display, 0, 0, 0,
                                       (XIMProc) xim_create_callback, reinterpret_cast<char *>(this));
#else // !USE_X11R6_XIM
    else if (XSetLocaleModifiers ("") == 0)
        qWarning("Qt: Cannot set locale modifiers");
    else /* if (!noxim) */
        QXIMInputContext::create_xim();
#endif // USE_X11R6_XIM
}


/*!\internal
  Creates the application input method.
*/
void QXIMInputContext::create_xim()
{
    ++fontsetRefCount;
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
                if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
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
                                             (XIMProc) xim_create_callback, reinterpret_cast<char *>(this));
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

/*!\internal
  Closes the application input method.
*/
void QXIMInputContext::close_xim()
{
    QString errMsg("QXIMInputContext::close_xim() has been called");

    // ###clean up ximData!
    ximData.clear();

    // ############## check this!
    // Calling XCloseIM gives a Purify FMR error
    // XCloseIM(xim);
    // We prefer a less serious memory leak
    xim = 0;

    if ( --fontsetRefCount == 0 ) {
	Display *dpy = X11->display;
	for ( int i = 0; i < 8; i++ ) {
	    if ( fontsetCache[i] && fontsetCache[i] != (XFontSet)-1 ) {
		XFreeFontSet(dpy, fontsetCache[i]);
		fontsetCache[i] = 0;
	    }
	}
    }
}



QXIMInputContext::~QXIMInputContext()
{
    close_xim();
}


QString QXIMInputContext::identifierName()
{
    // the name should be "xim" rather than "XIM" to be consistent
    // with corresponding immodule of GTK+
    return "xim";
}


QString QXIMInputContext::language()
{
    QString language;
    if (xim) {
        QByteArray locale(XLocaleOfIM(xim));

        if (locale.startsWith("zh")) {
            // Chinese language should be formed as "zh_CN", "zh_TW", "zh_HK"
            language = locale.left(5);
        } else {
            // other languages should be two-letter ISO 639 language code
            language = locale.left(2);
        }
    }
    return language;
}

void QXIMInputContext::reset()
{
    QWidget *w = focusWidget();
    if (!w)
        return;

    ICData *data = ximData.value(w);
    if (!data)
        return;

    if (data->ic) {
        char *mb = XmbResetIC(data->ic);
        if (mb) {
            QInputMethodEvent e;
            e.setCommitString(QString::fromLocal8Bit(mb));
            sendEvent(e);
            XFree(mb);
        }
    }
    data->clear();
}

void QXIMInputContext::widgetDestroyed(QWidget *w)
{
    QInputContext::widgetDestroyed(w);
    ICData *data = ximData.take(w);
    if (!data)
        return;

    data->clear();
    if (data->ic)
        XDestroyIC(data->ic);
    delete data;
}

void QXIMInputContext::mouseHandler(int pos, QMouseEvent *)
{
    XIM_DEBUG("QXIMInputContext::mouseHandler pos=%d", pos);
    ICData *data = ximData.value(focusWidget());
    if (!data)
        return;
    if (pos < 0 || pos > data->text.length())
        reset();
    // ##### handle mouse position
}

bool QXIMInputContext::isComposing() const
{
    QWidget *w = focusWidget();
    if (!w)
        return false;

    ICData *data = ximData.value(w);
    if (!data)
        return false;
    return data->composing;
}

void QXIMInputContext::setFocusWidget(QWidget *w)
{
    if (!xim)
        return;
    QWidget *oldFocus = focusWidget();
    if (oldFocus == w)
        return;

    if (language() != QLatin1String("ja"))
        reset();

    if (oldFocus) {
        ICData *data = ximData.value(oldFocus);
        if (data && data->ic)
            XUnsetICFocus(data->ic);
    }

    QInputContext::setFocusWidget(w);

    if (!w)
        return;

    ICData *data = ximData.value(w);
    if (!data)
        data = createICData(w);

    if (data->ic)
        XSetICFocus(data->ic);

    update();
}


bool QXIMInputContext::x11FilterEvent(QWidget *keywidget, XEvent *event)
{
    int xkey_keycode = event->xkey.keycode;
    if (XFilterEvent(event, keywidget->winId())) {
        qt_ximComposingKeycode = xkey_keycode; // ### not documented in xlib

        return true;
    }
    if (event->type != XKeyPress || event->xkey.keycode != 0)
        return false;

    QWidget *w = focusWidget();
    if (keywidget != w)
        return false;
    ICData *data = ximData.value(w);
    if (!data)
        return false;

    // input method has sent us a commit string
    QByteArray string;
    string.resize(513);
    KeySym key;    // unused
    Status status; // unused
    QString text;
    int count = XmbLookupString(data->ic, &event->xkey, string.data(), string.size(),
                                &key, &status);

    if (status == XBufferOverflow) {
        string.resize(count + 1);
        count = XmbLookupString(data->ic, &event->xkey, string.data(), string.size(),
                                &key, &status);
    }
    if (count > 0)
        text = qt_input_mapper->toUnicode(string.constData() , count);

#if 0
    if (!(xim_style & XIMPreeditCallbacks) || !isComposing()) {
        // ############### send a regular key event here!
        ;
    }
#endif

    QInputMethodEvent e;
    e.setCommitString(text);
    sendEvent(e);
    data->clear();
    return true;
}


QXIMInputContext::ICData *QXIMInputContext::createICData(QWidget *w)
{
    ICData *data = new ICData;
    data->widget = w;

    XVaNestedList preedit_attr = 0;
    XIMCallback startcallback, drawcallback, donecallback;

    QFont font = w->font();
    data->fontset = getFontSet(font);

    if (xim_style & XIMPreeditArea) {
        XRectangle rect;
        rect.x = 0;
        rect.y = 0;
        rect.width = w->width();
        rect.height = w->height();

        preedit_attr = XVaCreateNestedList(0,
                                           XNArea, &rect,
                                           XNFontSet, data->fontset,
                                           (char *) 0);
    } else if (xim_style & XIMPreeditPosition) {
        XPoint spot;
        spot.x = 1;
        spot.y = 1;

        preedit_attr = XVaCreateNestedList(0,
                                           XNSpotLocation, &spot,
                                           XNFontSet, data->fontset,
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
        data->ic = XCreateIC(xim,
                             XNInputStyle, xim_style,
                             XNClientWindow, w->winId(),
                             XNPreeditAttributes, preedit_attr,
                             (char *) 0);
        XFree(preedit_attr);
    } else {
        data->ic = XCreateIC(xim,
                             XNInputStyle, xim_style,
                             XNClientWindow, w->winId(),
                             (char *) 0);
    }

    if (data->ic) {
        // when resetting the input context, preserve the input state
        (void) XSetICValues(data->ic, XNResetState, XIMPreserveState, (char *) 0);
    } else {
        qWarning("Failed to create XIC");
    }

    ximData[w] = data;
    return data;
}

void QXIMInputContext::update()
{
    QWidget *w = focusWidget();
    if (!w)
        return;

    ICData *data = ximData.value(w);
    if (!data || !data->ic)
        return;

    QRect r = w->inputMethodQuery(Qt::ImMicroFocus).toRect();
    QPoint p = QPoint((r.left() + r.right() + 1)/2, r.bottom());
    XPoint spot;
    spot.x = p.x();
    spot.y = p.y();

    r = w->rect();
    XRectangle area;
    area.x = r.x();
    area.y = r.y();
    area.width = r.width();
    area.height = r.height();

    XFontSet fontset = getFontSet(qvariant_cast<QFont>(w->inputMethodQuery(Qt::ImFont)));
    if (data->fontset == fontset)
        fontset = 0;
    else
        data->fontset = fontset;

    XVaNestedList preedit_attr;
    if (fontset)
        preedit_attr = XVaCreateNestedList(0,
                                           XNSpotLocation, &spot,
                                           XNArea, &area,
                                           XNFontSet, fontset,
                                           (char *) 0);
    else
        preedit_attr = XVaCreateNestedList(0,
                                           XNSpotLocation, &spot,
                                           XNArea, &area,
                                           (char *) 0);

    XSetICValues(data->ic, XNPreeditAttributes, preedit_attr, (char *) 0);
    XFree(preedit_attr);
}


#else
/*
    When QT_NO_XIM is defined, we provide a dummy implementation for
    this class. The reason for this is that the header file is moc'ed
    regardless of QT_NO_XIM. The best would be to remove the file
    completely from the pri file is QT_NO_XIM was defined, or for moc
    to understand this preprocessor directive. Since the header does
    not declare this class when QT_NO_XIM is defined, this is dead
    code.
*/
bool QXIMInputContext::isComposing() const { return false; }
QString QXIMInputContext::identifierName() { return QString(); }
void QXIMInputContext::mouseHandler(int, QMouseEvent *) {}
void QXIMInputContext::setFocusWidget(QWidget *) {}
void QXIMInputContext::reset() {}
void QXIMInputContext::update() {}
QXIMInputContext::~QXIMInputContext() {}
void QXIMInputContext::widgetDestroyed(QWidget *) {}
QString QXIMInputContext::language() { return QString(); }
bool QXIMInputContext::x11FilterEvent(QWidget *, XEvent *) { return true; }
#endif //QT_NO_XIM

#endif //QT_NO_IM

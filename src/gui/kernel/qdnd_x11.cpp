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

#include "qplatformdefs.h"

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qdatetime.h"
#include "qiodevice.h"
#include "qpointer.h"
#include "qcursor.h"
#include "qvariant.h"
#include "qvector.h"
#include "qurl.h"
#include "qdebug.h"
#include "qimagewriter.h"
#include "qbuffer.h"

#include "qdnd_p.h"
#include "qt_x11_p.h"
#include "qx11info_x11.h"

#include "qwidget_p.h"
#include "qcursor_p.h"

// #define DND_DEBUG
#ifdef DND_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if(0) qDebug
#endif

#ifdef DND_DEBUG
#define DNDDEBUG qDebug()
#else
#define DNDDEBUG if(0) qDebug()
#endif

// and all this stuff is copied -into- qapp_x11.cpp

static void handle_xdnd_position(QWidget *, const XEvent *, bool);
static void handle_xdnd_status(QWidget * w, const XEvent * xe, bool /*passive*/);

const int xdnd_version = 5;

static Qt::DropAction xdndaction_to_qtaction(Atom atom)
{
    if (atom == ATOM(XdndActionCopy) || atom == 0)
        return Qt::CopyAction;
    if (atom == ATOM(XdndActionLink))
        return Qt::LinkAction;
    if (atom == ATOM(XdndActionMove))
        return Qt::MoveAction;
    return Qt::CopyAction;
}

static int qtaction_to_xdndaction(Qt::DropAction a)
{
    switch (a) {
    case Qt::CopyAction:
        return ATOM(XdndActionCopy);
    case Qt::LinkAction:
        return ATOM(XdndActionLink);
    case Qt::MoveAction:
    case Qt::TargetMoveAction:
        return ATOM(XdndActionMove);
    case Qt::IgnoreAction:
        return XNone;
    default:
        return ATOM(XdndActionCopy);
    }
}

// clean up the stuff used.
static void qt_xdnd_cleanup();

static void qt_xdnd_send_leave();

// real variables:
// xid of current drag source
static Atom qt_xdnd_dragsource_xid = 0;

// the types in this drop. 100 is no good, but at least it's big.
const int qt_xdnd_max_type = 100;
static Atom qt_xdnd_types[qt_xdnd_max_type];

// timer used when target wants "continuous" move messages (eg. scroll)
static int heartbeat = -1;
// rectangle in which the answer will be the same
static QRect qt_xdnd_source_sameanswer;
// top-level window we sent position to last.
static Window qt_xdnd_current_target;
// window to send events to (always valid if qt_xdnd_current_target)
static Window qt_xdnd_current_proxy_target;
// widget we forwarded position to last, and local position
static QPointer<QWidget> qt_xdnd_current_widget;
static QPoint qt_xdnd_current_position;
// timestamp from the XdndPosition and XdndDrop
static Time qt_xdnd_target_current_time;
// screen number containing the pointer... -1 means default
static int qt_xdnd_current_screen = -1;
// state of dragging... true if dragging, false if not
bool qt_xdnd_dragging = false;

static bool waiting_for_status = false;

// used to preset each new QDragMoveEvent
static Qt::DropAction last_target_accepted_action = Qt::IgnoreAction;

// Shift/Ctrl handling, and final drop status
static Qt::DropAction global_accepted_action = Qt::CopyAction;
static Qt::DropActions possible_actions = Qt::IgnoreAction;

// for embedding only
static QWidget* current_embedding_widget  = 0;
static XEvent last_enter_event;

// cursors
static QCursor *noDropCursor = 0;
static QCursor *moveCursor = 0;
static QCursor *copyCursor = 0;
static QCursor *linkCursor = 0;

static QPixmap *defaultPm = 0;

static const int default_pm_hotx = -2;
static const int default_pm_hoty = -16;
static const char* const default_pm[] = {
"13 9 3 1",
".      c None",
"       c #000000",
"X      c #FFFFFF",
"X X X X X X X",
" X X X X X X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X X X X X X ",
"X X X X X X X"
};

class QShapedPixmapWidget : public QWidget {

public:
    QShapedPixmapWidget(int screen = -1) :
        QWidget(QApplication::desktop()->screen(screen),
                Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
    {
    }

    void setPixmap(const QPixmap &pm)
    {
        QBitmap mask = pm.mask();
        if (!mask.isNull()) {
            setMask(mask);
        } else {
            clearMask();
        }
        resize(pm.width(),pm.height());
	QPalette p = palette();
	p.setBrush(backgroundRole(), QBrush(pm));
	setPalette(p);
        update();
    }
    QPoint pm_hot;
};

struct XdndData {
    QShapedPixmapWidget *deco;
    QWidget* desktop_proxy;
};

static XdndData xdnd_data = { 0, 0 };

class QExtraWidget : public QWidget
{
    Q_DECLARE_PRIVATE(QWidget)
public:
    inline QWExtra* extraData();
    inline QTLWExtra* topData();
};

inline QWExtra* QExtraWidget::extraData() { return d_func()->extraData(); }
inline QTLWExtra* QExtraWidget::topData() { return d_func()->topData(); }


static WId xdndProxy(WId w)
{
    Atom type = XNone;
    int f;
    unsigned long n, a;
    WId *proxy_id_ptr;
    XGetWindowProperty(X11->display, w, ATOM(XdndProxy), 0, 1, False,
                       XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id_ptr);
    WId proxy_id = 0;
    if (type == XA_WINDOW && proxy_id_ptr) {
        proxy_id = *proxy_id_ptr;
        XFree(proxy_id_ptr);
        proxy_id_ptr = 0;
        // Already exists. Real?
        X11->ignoreBadwindow();
        XGetWindowProperty(X11->display, proxy_id, ATOM(XdndProxy), 0, 1, False,
                           XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id_ptr);
        if (X11->badwindow() || type != XA_WINDOW || !proxy_id_ptr || *proxy_id_ptr != proxy_id)
            // Bogus - we will overwrite.
            proxy_id = 0;
    }
    if (proxy_id_ptr)
        XFree(proxy_id_ptr);
    return proxy_id;
}

static bool xdndEnable(QWidget* w, bool on)
{
    DNDDEBUG << "xdndEnable" << w << on;
    if (on) {
        QWidget * xdnd_widget = 0;
        if ((w->windowType() == Qt::Desktop)) {
            if (xdnd_data.desktop_proxy) // *WE* already have one.
                return false;

            // As per Xdnd4, use XdndProxy
            XGrabServer(X11->display);
            WId proxy_id = xdndProxy(w->winId());

            if (!proxy_id) {
                xdnd_widget = xdnd_data.desktop_proxy = new QWidget;
                proxy_id = xdnd_data.desktop_proxy->winId();
                XChangeProperty (X11->display, w->winId(), ATOM(XdndProxy),
                                 XA_WINDOW, 32, PropModeReplace, (unsigned char *)&proxy_id, 1);
                XChangeProperty (X11->display, proxy_id, ATOM(XdndProxy),
                                 XA_WINDOW, 32, PropModeReplace, (unsigned char *)&proxy_id, 1);
            }

            XUngrabServer(X11->display);
        } else {
            xdnd_widget = w->window();
        }
        if (xdnd_widget) {
            DNDDEBUG << "setting XdndAware for" << xdnd_widget << xdnd_widget->winId();
            Atom atm = (Atom)xdnd_version;
            XChangeProperty(X11->display, xdnd_widget->winId(), ATOM(XdndAware),
                             XA_ATOM, 32, PropModeReplace, (unsigned char *)&atm, 1);
            return true;
        } else {
            return false;
        }
    } else {
        if ((w->windowType() == Qt::Desktop)) {
            XDeleteProperty(X11->display, w->winId(), ATOM(XdndProxy));
            delete xdnd_data.desktop_proxy;
            xdnd_data.desktop_proxy = 0;
        } else {
            DNDDEBUG << "not deleteing XDndAware";
        }
        return true;
    }
}

QByteArray QX11Data::xdndAtomToString(Atom a)
{
    if (!a) return 0;

    if (a == XA_STRING || a == ATOM(UTF8_STRING)) {
        return "text/plain"; // some Xdnd clients are dumb
    }
    char *atom = XGetAtomName(display, a);
    QByteArray result = atom;
    XFree(atom);
    return result;
}

Atom QX11Data::xdndStringToAtom(const char *mimeType)
{
    if (!mimeType || !*mimeType)
        return 0;
    return XInternAtom(display, mimeType, False);
}

//$$$
QString QX11Data::xdndMimeAtomToString(Atom a)
{
    QString atomName;
    if (a) {
        char *atom = XGetAtomName(display, a);
        atomName = QString::fromLatin1(atom);
        XFree(atom);
    }
    return atomName;
}

//$$$
Atom QX11Data::xdndMimeStringToAtom(const QString &mimeType)
{
    if (mimeType.isEmpty())
        return 0;
    return XInternAtom(display, mimeType.toLatin1().constData(), False);
}

//$$$ replace ccxdndAtomToString()
QStringList QX11Data::xdndMimeFormatsForAtom(Atom a)
{
    QStringList formats;
    if (a) {
        QString atomName = xdndMimeAtomToString(a);
        formats.append(atomName);
        
        // special cases for string type
        if (a == ATOM(UTF8_STRING) || a == XA_STRING
            || a == ATOM(TEXT) || a == ATOM(COMPOUND_TEXT))
            formats.append(QLatin1String("text/plain"));
        
        // special cases for uris
        if (atomName == QLatin1String("text/x-moz-url"))
            formats.append(QLatin1String("text/uri-list"));

        // special case for images
        if (a == XA_PIXMAP)
            formats.append(QLatin1String("image/ppm"));
    }
    return formats;
}

//$$$ 
bool QX11Data::xdndMimeDataForAtom(Atom a, QMimeData *mimeData, QByteArray *data, Atom *atomFormat, int *dataFormat)
{
    bool ret = false;
    *atomFormat = a;
    *dataFormat = 8;
    QString atomName = xdndMimeAtomToString(a);
    if (QInternalMimeData::hasFormatHelper(atomName, mimeData)) {
        *data = QInternalMimeData::renderDataHelper(atomName, mimeData);
        if (atomName == QLatin1String("application/x-color"))
            *dataFormat = 16;
        ret = true;
    } else {
        if ((a == ATOM(UTF8_STRING) || a == XA_STRING 
            || a == ATOM(TEXT) || a == ATOM(COMPOUND_TEXT))
            && QInternalMimeData::hasFormatHelper(QLatin1String("text/plain"), mimeData)) {
            if (a == ATOM(UTF8_STRING)){
                *data = QInternalMimeData::renderDataHelper(QLatin1String("text/plain"), mimeData);
                ret = true;
            } else if (a == XA_STRING) {
                *data = QString::fromUtf8(QInternalMimeData::renderDataHelper(
                        QLatin1String("text/plain"), mimeData)).toLocal8Bit();
                ret = true;
            } else if (a == ATOM(TEXT) || a == ATOM(COMPOUND_TEXT)) {
                // the ICCCM states that TEXT and COMPOUND_TEXT are in the
                // encoding of choice, so we choose the encoding of the locale
                QByteArray strData = QString::fromUtf8(QInternalMimeData::renderDataHelper(
                                     QLatin1String("text/plain"), mimeData)).toLocal8Bit();
                char *list[] = { strData.data(), NULL };
    
                XICCEncodingStyle style = (a == ATOM(COMPOUND_TEXT)) 
                                        ? XCompoundTextStyle : XStdICCTextStyle;
                XTextProperty textprop;
                if (list[0] != NULL
                    && XmbTextListToTextProperty(X11->display, list, 1, style,
                                                 &textprop) == Success) {
                    *atomFormat = textprop.encoding;
                    *dataFormat = textprop.format;
                    *data = QByteArray((const char *) textprop.value, textprop.nitems * textprop.format / 8);

                    DEBUG("    textprop type %lx\n"
                    "    textprop name '%s'\n"
                    "    format %d\n"
                    "    %ld items\n"
                    "    %d bytes\n",
                    textprop.encoding, 
                    X11->xdndMimeAtomToString(textprop.encoding).toLatin1().data(),
                    textprop.format, textprop.nitems, data->size());

                    XFree(textprop.value);
                }
            }
        } else if (atomName == QLatin1String("text/x-moz-url") && 
                   QInternalMimeData::hasFormatHelper(QLatin1String("text/uri-list"), mimeData)) {
            QByteArray uri = QInternalMimeData::renderDataHelper(
                             QLatin1String("text/uri-list"), mimeData).split('\n').first();
            QString mozUri = QString::fromLatin1(uri, uri.size());
            mozUri += QLatin1Char('\n');
            *data = QByteArray(reinterpret_cast<const char *>(mozUri.utf16()), mozUri.length() * 2);
            ret = true;
        } else if ((a == XA_PIXMAP || a == XA_BITMAP) && mimeData->hasImage()) {
            QPixmap pm = qvariant_cast<QPixmap>(mimeData->imageData());
            if (a == XA_BITMAP && pm.depth() != 1) {
                QImage img = pm.toImage();
                img = img.convertToFormat(QImage::Format_MonoLSB);
                pm = QPixmap::fromImage(img);
            }
            QDragManager *dm = QDragManager::self();
            if (dm) {
                Pixmap handle = pm.handle();
                *data = QByteArray((const char *) &handle, sizeof(Pixmap));
                dm->xdndMimeTransferedPixmap[dm->xdndMimeTransferedPixmapIndex] = pm;
                dm->xdndMimeTransferedPixmapIndex = 
                            (dm->xdndMimeTransferedPixmapIndex + 1) % 2;
            }
        }
    }
    return data;
}

//$$$
QList<Atom> QX11Data::xdndMimeAtomsForFormat(const QString &format)
{
    QList<Atom> atoms;
    atoms.append(xdndMimeStringToAtom(format));

    // special cases for strings
    if (format == QLatin1String("text/plain")) {
        atoms.append(ATOM(UTF8_STRING));
        atoms.append(XA_STRING);
        atoms.append(ATOM(TEXT));
        atoms.append(ATOM(COMPOUND_TEXT));
    }

    // special cases for uris
    if (format == QLatin1String("text/uri-list")) {
        atoms.append(xdndMimeStringToAtom(QLatin1String("text/x-moz-url")));
    }

    //special cases for images
    if (format == QLatin1String("image/ppm"))
        atoms.append(XA_PIXMAP);
    if (format == QLatin1String("image/pbm"))
        atoms.append(XA_BITMAP);

    return atoms;
}

//$$$
QByteArray QX11Data::xdndMimeConvertToFormat(Atom a, const QByteArray &data, const QString &format)
{
    QString atomName = xdndMimeAtomToString(a);
    if (atomName == format)
        return data;
 
    // special cases for string types
    if (format == QLatin1String("text/plain")) {
        if (a == ATOM(UTF8_STRING))
            return data;
        if (a == XA_STRING)
            return QString::fromLatin1(data).toUtf8();
        if (a == ATOM(TEXT) || a == ATOM(COMPOUND_TEXT))
            // #### might be wrong for COMPUND_TEXT
            return QString::fromLocal8Bit(data, data.size()).toUtf8();
    }
    
    // special case for uri types
    if (format == QLatin1String("text/uri-list")) {
        if (atomName == QLatin1String("text/x-moz-url")) {
            // we expect this as utf16 <url><space><title>
            // the first part is a url that should only contain ascci char
            // so it should be safe to check that the second char is 0
            // to verify that it is utf16
            if (data.size() > 1 && data.at(1) == 0)
                return QString::fromUtf16(reinterpret_cast<const ushort *>(data.constData()),
                                data.size() / 2).split(QLatin1Char('\n')).first().toLatin1();
        }
    }

    // special cas for images
    if (format == QLatin1String("image/ppm")) {
        if (a == XA_PIXMAP && data.size() == sizeof(Pixmap)) {
            Pixmap xpm = *((Pixmap*)data.data());
            Display *dpy = display;
            Window r;
            int x,y;
            uint w,h,bw,d;
            if (!xpm)
                return QByteArray();
            XGetGeometry(dpy,xpm, &r,&x,&y,&w,&h,&bw,&d);
            QImageWriter imageWriter;
            GC gc = XCreateGC(dpy, xpm, 0, 0);
            QImage imageToWrite;
            if (d == 1) {
                QBitmap qbm(w,h);
                XCopyArea(dpy,xpm,qbm.handle(),gc,0,0,w,h,0,0);
                imageWriter.setFormat("PBMRAW");
                imageToWrite = qbm.toImage();
            } else {
                QPixmap qpm(w,h);
                XCopyArea(dpy,xpm,qpm.handle(),gc,0,0,w,h,0,0);
                imageWriter.setFormat("PPMRAW");
                imageToWrite = qpm.toImage();
            }
            XFreeGC(dpy,gc);
            QBuffer buf;
            buf.open(QIODevice::WriteOnly);
            imageWriter.setDevice(&buf);
            imageWriter.write(imageToWrite);
            return buf.buffer();
        }
    }
    return QByteArray();        
}

//$$$ middle of xdndObtainData
Atom QX11Data::xdndMimeAtomForFormat(const QString &format, const QList<Atom> &atoms)
{
    Atom a = xdndMimeStringToAtom(format);
    if (a && atoms.contains(a))
        return a;
    
    // find matches for string types
    if (format == QLatin1String("text/plain")) {
        if (atoms.contains(ATOM(UTF8_STRING)))
            return ATOM(UTF8_STRING);
        if (atoms.contains(ATOM(COMPOUND_TEXT)))
            return XA_STRING;
        if (atoms.contains(ATOM(TEXT)))
            return XA_STRING;
        if (atoms.contains(XA_STRING))
            return XA_STRING;
    }

    // find mathes for uri types
    if (format == QLatin1String("text/uri-list")) {
        Atom a = xdndMimeStringToAtom(QLatin1String("text/x-moz-url"));
        if (a && atoms.contains(a))
            return a;
    }

    // find match for image
    if (format == QLatin1String("image/ppm")) {
        if (atoms.contains(XA_PIXMAP))
            return XA_PIXMAP;
    }

    return 0;
}

void QX11Data::xdndSetup() {
    QCursorData::initialize();
    qAddPostRoutine(qt_xdnd_cleanup);
}


void qt_xdnd_cleanup()
{
    delete noDropCursor;
    noDropCursor = 0;
    delete copyCursor;
    copyCursor = 0;
    delete moveCursor;
    moveCursor = 0;
    delete linkCursor;
    linkCursor = 0;
    delete defaultPm;
    defaultPm = 0;
    delete xdnd_data.desktop_proxy;
    xdnd_data.desktop_proxy = 0;
    delete xdnd_data.deco;
    xdnd_data.deco = 0;
}


static QWidget *find_child(QWidget *tlw, QPoint & p)
{
    QWidget *widget = tlw;

    p = widget->mapFromGlobal(p);
    bool done = false;
    while (!done) {
        done = true;
        if (((QExtraWidget*)widget)->extraData() &&
             ((QExtraWidget*)widget)->extraData()->xDndProxy != 0)
            break; // stop searching for widgets under the mouse cursor if found widget is a proxy.
        QObjectList children = widget->children();
        if (!children.isEmpty()) {
            for(int i = children.size(); i > 0;) {
                --i;
                QWidget *w = qobject_cast<QWidget *>(children.at(i));
                if (!w)
                    continue;
                if (w->isVisible() &&
                     w->geometry().contains(p) &&
                     !w->isWindow()) {
                    widget = w;
                    done = false;
                    p = widget->mapFromParent(p);
                    break;
                }
            }
        }
    }
    return widget;
}


static bool checkEmbedded(QWidget* w, const XEvent* xe)
{
    if (!w)
        return false;

    if (current_embedding_widget != 0 && current_embedding_widget != w) {
        qt_xdnd_current_target = ((QExtraWidget*)current_embedding_widget)->extraData()->xDndProxy;
        qt_xdnd_current_proxy_target = qt_xdnd_current_target;
        qt_xdnd_send_leave();
        qt_xdnd_current_target = 0;
        qt_xdnd_current_proxy_target = 0;
        current_embedding_widget = 0;
    }

    QWExtra* extra = ((QExtraWidget*)w)->extraData();
    if (extra && extra->xDndProxy != 0) {

        if (current_embedding_widget != w) {

            last_enter_event.xany.window = extra->xDndProxy;
            XSendEvent(X11->display, extra->xDndProxy, False, NoEventMask, &last_enter_event);
            current_embedding_widget = w;
        }

        ((XEvent*)xe)->xany.window = extra->xDndProxy;
        XSendEvent(X11->display, extra->xDndProxy, False, NoEventMask, (XEvent*)xe);
        if (qt_xdnd_current_widget != w) {
            qt_xdnd_current_widget = w;
        }
        return true;
    }
    current_embedding_widget = 0;
    return false;
}

void QX11Data::xdndHandleEnter(QWidget *, const XEvent * xe, bool /*passive*/)
{
    motifdnd_active = false;

    last_enter_event.xclient = xe->xclient;

    const long *l = xe->xclient.data.l;
    int version = (int)(((unsigned long)(l[1])) >> 24);

    if (version > xdnd_version)
        return;

    qt_xdnd_dragsource_xid = l[0];

    int j = 0;
    if (l[1] & 1) {
        // get the types from XdndTypeList
        Atom   type = XNone;
        int f;
        unsigned long n, a;
        Atom *data;
        XGetWindowProperty(X11->display, qt_xdnd_dragsource_xid, ATOM(XdndTypelist), 0,
                           qt_xdnd_max_type, False, XA_ATOM, &type, &f,&n,&a,(uchar**)&data);
        for (; j<qt_xdnd_max_type && j < (int)n; j++) {
            qt_xdnd_types[j] = data[j];
        }
        if (data)
            XFree((uchar*)data);
    } else {
        // get the types from the message
        int i;
        for(i=2; i < 5; i++) {
            qt_xdnd_types[j++] = l[i];
        }
    }
    qt_xdnd_types[j] = 0;
}

static void handle_xdnd_position(QWidget *w, const XEvent * xe, bool passive)
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QPoint p((l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff);
    QWidget * c = find_child(w, p); // changes p to to c-local coordinates

    if (!passive && checkEmbedded(c, xe))
        return;

    if (!c || !c->acceptDrops() && (c->windowType() == Qt::Desktop))
        return;

    if (l[0] != qt_xdnd_dragsource_xid) {
        DEBUG("xdnd drag position from unexpected source (%08lx not %08lx)", l[0], qt_xdnd_dragsource_xid);
        return;
    }

    if (l[3] != 0) {
        // timestamp from the source
        qt_xdnd_target_current_time = X11->userTime = l[3];
    }

    QDragManager *manager = QDragManager::self();
    QMimeData *dropData = manager->object ? manager->dragPrivate()->data : manager->dropData;

    XClientMessageEvent response;
    response.type = ClientMessage;
    response.window = qt_xdnd_dragsource_xid;
    response.format = 32;
    response.message_type = ATOM(XdndStatus);
    response.data.l[0] = w->winId();
    response.data.l[1] = 0; // flags
    response.data.l[2] = 0; // x, y
    response.data.l[3] = 0; // w, h
    response.data.l[4] = 0; // action

    if (!passive) { // otherwise just reject
        while (c && !c->acceptDrops() && !c->isWindow()) {
            p = c->mapToParent(p);
            c = c->parentWidget();
        }
        QWidget *target_widget = c->acceptDrops() ? c : 0;

        QRect answerRect(c->mapToGlobal(p), QSize(1,1));

        if (manager->object) {
            possible_actions = manager->dragPrivate()->possible_actions;
        } else {
            possible_actions = Qt::DropActions(xdndaction_to_qtaction(l[4]));
//             possible_actions |= Qt::CopyAction;
        }
        QDragMoveEvent me(p, possible_actions, dropData, QApplication::mouseButtons(), QApplication::keyboardModifiers());

        Qt::DropAction accepted_action = Qt::IgnoreAction;


        if (target_widget != qt_xdnd_current_widget) {
            if (qt_xdnd_current_widget) {
                QDragLeaveEvent e;
                QApplication::sendEvent(qt_xdnd_current_widget, &e);
            }
            if (qt_xdnd_current_widget != target_widget) {
                qt_xdnd_current_widget = target_widget;
            }
            if (target_widget) {
                qt_xdnd_current_position = p;

                last_target_accepted_action = Qt::IgnoreAction;
                QDragEnterEvent de(p, possible_actions, dropData, QApplication::mouseButtons(), QApplication::keyboardModifiers());
                QApplication::sendEvent(target_widget, &de);
                if (de.isAccepted() && de.dropAction() != Qt::IgnoreAction)
                    last_target_accepted_action = de.dropAction();
            }
        }

        DEBUG() << "qt_handle_xdnd_position action=" << X11->xdndAtomToString(l[4]);
        if (!target_widget) {
            answerRect = QRect(p, QSize(1, 1));
        } else {
            qt_xdnd_current_widget = c;
            qt_xdnd_current_position = p;

            if (last_target_accepted_action != Qt::IgnoreAction) {
                me.setDropAction(last_target_accepted_action);
                me.accept();
            }
            QApplication::sendEvent(c, &me);
            if (me.isAccepted()) {
                response.data.l[1] = 1; // yes
                accepted_action = me.dropAction();
                last_target_accepted_action = accepted_action;
            } else {
                response.data.l[0] = 0;
                last_target_accepted_action = Qt::IgnoreAction;
            }
            answerRect = me.answerRect().intersect(c->rect());
        }
        answerRect = QRect(c->mapToGlobal(answerRect.topLeft()), answerRect.size());

        if (answerRect.left() < 0)
            answerRect.setLeft(0);
        if (answerRect.right() > 4096)
            answerRect.setRight(4096);
        if (answerRect.top() < 0)
            answerRect.setTop(0);
        if (answerRect.bottom() > 4096)
            answerRect.setBottom(4096);
        if (answerRect.width() < 0)
            answerRect.setWidth(0);
        if (answerRect.height() < 0)
            answerRect.setHeight(0);

        response.data.l[2] = (answerRect.x() << 16) + answerRect.y();
        response.data.l[3] = (answerRect.width() << 16) + answerRect.height();
        response.data.l[4] = qtaction_to_xdndaction(accepted_action);
    }

    // reset
    qt_xdnd_target_current_time = CurrentTime;

    QWidget * source = QWidget::find(qt_xdnd_dragsource_xid);
    if (source && (source->windowType() == Qt::Desktop) && !source->acceptDrops())
        source = 0;

    DEBUG() << "sending XdndStatus";
    if (source)
        handle_xdnd_status(source, (const XEvent *)&response, passive);
    else
        XSendEvent(X11->display, qt_xdnd_dragsource_xid, False, NoEventMask, (XEvent*)&response);
}

static Bool xdnd_position_scanner(Display *, XEvent *event, XPointer)
{
    if (event->type != ClientMessage)
        return false;
    XClientMessageEvent *ev = &event->xclient;

    if (ev->message_type == ATOM(XdndPosition))
        return true;

    return false;
}

void QX11Data::xdndHandlePosition(QWidget * w, const XEvent * xe, bool passive)
{
    DEBUG("xdndHandlePosition");
    while (XCheckIfEvent(X11->display, (XEvent *)xe, xdnd_position_scanner, 0))
        ;

    handle_xdnd_position(w, xe, passive);
}


static void handle_xdnd_status(QWidget *, const XEvent * xe, bool)
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;
    // ignore late status messages
    if (l[0] && l[0] != qt_xdnd_current_proxy_target)
        return;
    Qt::DropAction newAction = (l[1] & 0x1) ? xdndaction_to_qtaction(l[4]) : Qt::IgnoreAction;

    if ((int)(l[1] & 2) == 0) {
        QPoint p((l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff);
        QSize s((l[3] & 0xffff0000) >> 16, l[3] & 0x0000ffff);
        qt_xdnd_source_sameanswer = QRect(p, s);
    } else {
        qt_xdnd_source_sameanswer = QRect();
    }
    QDragManager *manager = QDragManager::self();
    manager->willDrop = (l[1] & 0x1);
    if (global_accepted_action != newAction)
        manager->emitActionChanged(newAction);
    global_accepted_action = newAction;
    manager->updateCursor();
    waiting_for_status = false;
}

static Bool xdnd_status_scanner(Display *, XEvent *event, XPointer)
{
    if (event->type != ClientMessage)
        return false;
    XClientMessageEvent *ev = &event->xclient;

    if (ev->message_type == ATOM(XdndStatus))
        return true;

    return false;
}

void QX11Data::xdndHandleStatus(QWidget * w, const XEvent * xe, bool passive)
{
    DEBUG("xdndHandleStatus");
    while (XCheckIfEvent(X11->display, (XEvent *)xe, xdnd_status_scanner, 0))
        ;

    handle_xdnd_status(w, xe, passive);
    DEBUG("xdndHandleStatus end");
}

void QX11Data::xdndHandleLeave(QWidget *w, const XEvent * xe, bool /*passive*/)
{
    DEBUG("xdnd leave");
    if (!qt_xdnd_current_widget ||
         w->window() != qt_xdnd_current_widget->window()) {
        return; // sanity
    }

    if (checkEmbedded(current_embedding_widget, xe)) {
        current_embedding_widget = 0;
        qt_xdnd_current_widget = 0;
        return;
    }

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QDragLeaveEvent e;
    QApplication::sendEvent(qt_xdnd_current_widget, &e);

    if (l[0] != qt_xdnd_dragsource_xid) {
        // This often happens - leave other-process window quickly
        DEBUG("xdnd drag leave from unexpected source (%08lx not %08lx", l[0], qt_xdnd_dragsource_xid);
        qt_xdnd_current_widget = 0;
        return;
    }

    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_types[0] = 0;
    qt_xdnd_current_widget = 0;
}


void qt_xdnd_send_leave()
{
    if (!qt_xdnd_current_target)
        return;

    XClientMessageEvent leave;
    leave.type = ClientMessage;
    leave.window = qt_xdnd_current_target;
    leave.format = 32;
    leave.message_type = ATOM(XdndLeave);
    leave.data.l[0] = qt_xdnd_dragsource_xid;
    leave.data.l[1] = 0; // flags
    leave.data.l[2] = 0; // x, y
    leave.data.l[3] = 0; // w, h
    leave.data.l[4] = 0; // just null

    QWidget * w = QWidget::find(qt_xdnd_current_proxy_target);

    if (w && (w->windowType() == Qt::Desktop) && !w->acceptDrops())
        w = 0;

    if (w)
        X11->xdndHandleLeave(w, (const XEvent *)&leave, false);
    else
        XSendEvent(X11->display, qt_xdnd_current_proxy_target, False,
                    NoEventMask, (XEvent*)&leave);
    // reset the drag manager state
    QDragManager *manager = QDragManager::self();
    manager->willDrop = false;
    if (global_accepted_action != Qt::IgnoreAction)
        manager->emitActionChanged(Qt::IgnoreAction);
    global_accepted_action = Qt::IgnoreAction;
    manager->updateCursor();
    qt_xdnd_current_target = 0;
    qt_xdnd_current_proxy_target = 0;
    waiting_for_status = false;
}



void QX11Data::xdndHandleDrop(QWidget *, const XEvent * xe, bool passive)
{
    DEBUG("xdndHandleDrop");
    if (!qt_xdnd_current_widget) {
        qt_xdnd_dragsource_xid = 0;
        return; // sanity
    }

    if (!passive && checkEmbedded(qt_xdnd_current_widget, xe)){
        current_embedding_widget = 0;
        qt_xdnd_dragsource_xid = 0;
        qt_xdnd_current_widget = 0;
        return;
    }
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QDragManager *manager = QDragManager::self();
    DEBUG("xdnd drop");

    if (l[0] != qt_xdnd_dragsource_xid) {
        DEBUG("xdnd drop from unexpected source (%08lx not %08lx", l[0], qt_xdnd_dragsource_xid);
        return;
    }

    if (l[2] != 0) {
        // update the "user time" from the timestamp in the event.
        qt_xdnd_target_current_time = X11->userTime = l[2];
    }

    if (!passive) {
        QMimeData *dropData = (manager->object) ? manager->dragPrivate()->data : manager->dropData;
        QDropEvent de(qt_xdnd_current_position, possible_actions, dropData,
                      QApplication::mouseButtons(), QApplication::keyboardModifiers());
        QApplication::sendEvent(qt_xdnd_current_widget, &de);
        if (!de.isAccepted()) {
            // Ignore a failed drag
            global_accepted_action = Qt::IgnoreAction;
        } else {
            global_accepted_action = de.dropAction();
        }
        XClientMessageEvent finished;
        finished.type = ClientMessage;
        finished.window = qt_xdnd_dragsource_xid;
        finished.format = 32;
        finished.message_type = ATOM(XdndFinished);
        DNDDEBUG << "xdndHandleDrop"
             << "qt_xdnd_current_widget" << qt_xdnd_current_widget
             << (qt_xdnd_current_widget ? qt_xdnd_current_widget->winId() : 0)
             << "t_xdnd_current_widget->window()"
             << (qt_xdnd_current_widget ? qt_xdnd_current_widget->window() : 0)
             << (qt_xdnd_current_widget ? qt_xdnd_current_widget->window()->winId() : 0);
        finished.data.l[0] = qt_xdnd_current_widget?qt_xdnd_current_widget->window()->winId():0;
        finished.data.l[1] = de.isAccepted() ? 1 : 0; // flags
        finished.data.l[2] = qtaction_to_xdndaction(global_accepted_action);
        XSendEvent(X11->display, qt_xdnd_dragsource_xid, False,
                    NoEventMask, (XEvent*)&finished);
    } else {
        QDragLeaveEvent e;
        QApplication::sendEvent(qt_xdnd_current_widget, &e);
    }
    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_current_widget = 0;
    waiting_for_status = false;

    // reset
    qt_xdnd_target_current_time = CurrentTime;
}


void QX11Data::xdndHandleFinished(QWidget *, const XEvent * xe, bool passive)
{
    DEBUG("xdndHandleFinished");
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    DNDDEBUG << "xdndHandleFinished, l[0]" << l[0]
             << "qt_xdnd_current_target" << qt_xdnd_current_target
             << "qt_xdnd_current_proxy_targe" << qt_xdnd_current_proxy_target;

    if (l[0] && (l[0] == qt_xdnd_current_target
            || l[0] == qt_xdnd_current_proxy_target)) {
        //
        if (!passive)
            (void) checkEmbedded(qt_xdnd_current_widget, xe);
        current_embedding_widget = 0;
        qt_xdnd_current_target = 0;
        qt_xdnd_current_proxy_target = 0;
        QDragManager *manager = QDragManager::self();
        if (manager->object)
            manager->object->deleteLater();
        manager->object = 0;
    }
    waiting_for_status = false;
}


void QDragManager::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == heartbeat && qt_xdnd_source_sameanswer.isNull())
        move(QCursor::pos());
}

bool QDragManager::eventFilter(QObject * o, QEvent * e)
{
    if (beingCancelled) {
        if (e->type() == QEvent::KeyRelease && ((QKeyEvent*)e)->key() == Qt::Key_Escape) {
            qApp->removeEventFilter(this);
            Q_ASSERT(object == 0);
            beingCancelled = false;
            eventLoop->exit();
            return true; // block the key release
        }
        return false;
    }

    Q_ASSERT(object != 0);

    if (!o->isWidgetType())
        return false;

    if (e->type() == QEvent::MouseMove) {
        QMouseEvent* me = (QMouseEvent *)e;
        move(me->globalPos());
        return true;
    } else if (e->type() == QEvent::MouseButtonRelease) {
        DEBUG("pre drop");
        qApp->removeEventFilter(this);
        if (willDrop)
            drop();
        else
            cancel();
        DEBUG("drop, resetting object");
        beingCancelled = false;
        eventLoop->exit();
        return true;
    }

    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = ((QKeyEvent*)e);
        if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress) {
            cancel();
            qApp->removeEventFilter(this);
            beingCancelled = false;
            eventLoop->exit();
        } else {
            qt_xdnd_source_sameanswer = QRect(); // force move
            move(QCursor::pos());
        }
        return true; // Eat all key events
    }

    // ### We bind modality to widgets, so we have to do this
    // ###  "manually".
    // DnD is modal - eat all other interactive events
    switch (e->type()) {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::Wheel:
      case QEvent::ShortcutOverride:
#ifdef QT3_SUPPORT
      case QEvent::Accel:
      case QEvent::AccelAvailable:
#endif
        return true;
      default:
        return false;
    }
}

void QDragManager::updateCursor()
{
    if (!noDropCursor) {
        noDropCursor = new QCursor(Qt::ForbiddenCursor);
        moveCursor = new QCursor(dragCursor(Qt::MoveAction), 0,0);
        copyCursor = new QCursor(dragCursor(Qt::CopyAction), 0,0);
        linkCursor = new QCursor(dragCursor(Qt::LinkAction), 0,0);
    }

    QCursor *c;
    if (willDrop) {
        if (global_accepted_action == Qt::CopyAction) {
            c = copyCursor;
        } else if (global_accepted_action == Qt::LinkAction) {
            c = linkCursor;
        } else {
            c = moveCursor;
        }
        if (xdnd_data.deco) {
            xdnd_data.deco->show();
            xdnd_data.deco->raise();
        }
    } else {
        c = noDropCursor;
        //if (qt_xdnd_deco)
        //    qt_xdnd_deco->hide();
    }
#ifndef QT_NO_CURSOR
    if (c)
        qApp->changeOverrideCursor(*c);
#endif
}


void QDragManager::cancel(bool deleteSource)
{
    DEBUG("QDragManager::cancel");
    Q_ASSERT(heartbeat != -1);
    killTimer(heartbeat);
    heartbeat = -1;
    beingCancelled = true;

    if (qt_xdnd_current_target)
        qt_xdnd_send_leave();

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif

    if (deleteSource && object)
        object->deleteLater();
    object = 0;
    delete xdnd_data.deco;
    xdnd_data.deco = 0;

    global_accepted_action = Qt::IgnoreAction;
}

static
Window findRealWindow(const QPoint & pos, Window w, int md)
{
    if (xdnd_data.deco && w == xdnd_data.deco->winId())
        return 0;

    if (md) {
        X11->ignoreBadwindow();
        XWindowAttributes attr;
        XGetWindowAttributes(X11->display, w, &attr);
        if (X11->badwindow())
            return 0;

        if (attr.map_state == IsViewable
            && QRect(attr.x,attr.y,attr.width,attr.height).contains(pos)) {
            {
                Atom   type = XNone;
                int f;
                unsigned long n, a;
                unsigned char *data;

                XGetWindowProperty(X11->display, w, ATOM(XdndAware), 0, 0, False,
                                   AnyPropertyType, &type, &f,&n,&a,&data);
                if (data) XFree(data);
                if (type)
                    return w;
            }

            Window r, p;
            Window* c;
            uint nc;
            if (XQueryTree(X11->display, w, &r, &p, &c, &nc)) {
                r=0;
                for (uint i=nc; !r && i--;) {
                    r = findRealWindow(pos-QPoint(attr.x,attr.y),
                                        c[i], md-1);
                }
                XFree(c);
                if (r)
                    return r;

                // We didn't find a client window!  Just use the
                // innermost window.
            }

            // No children!
            return w;
        }
    }
    return 0;
}

void QDragManager::move(const QPoint & globalPos)
{
    DEBUG() << "QDragManager::move enter";
    if (!object) {
        // perhaps the target crashed?
        return;
    }

    int screen = QCursor::x11Screen();
    if ((qt_xdnd_current_screen == -1 && screen != X11->defaultScreen) || (screen != qt_xdnd_current_screen)) {
        // recreate the pixmap on the new screen...
        delete xdnd_data.deco;
        xdnd_data.deco = new QShapedPixmapWidget(screen);
        if (!QWidget::mouseGrabber()) {
            updatePixmap();
            xdnd_data.deco->grabMouse();
        }
    }
    xdnd_data.deco->move(QCursor::pos() - xdnd_data.deco->pm_hot);

    if (qt_xdnd_source_sameanswer.contains(globalPos) && qt_xdnd_source_sameanswer.isValid())
        return;

    qt_xdnd_current_screen = screen;
    Window rootwin = QX11Info::appRootWindow(qt_xdnd_current_screen);
    Window target = 0;
    int lx = 0, ly = 0;
    if (!XTranslateCoordinates(X11->display, rootwin, rootwin, globalPos.x(), globalPos.y(), &lx, &ly, &target))
        // some weird error...
        return;

    if (target == rootwin) {
        // Ok.
    } else if (target) {
        //me
        Window src = rootwin;
        while (target != 0) {
            DNDDEBUG << "checking target for XdndAware" << QWidget::find(target) << target;
            int lx2, ly2;
            Window t;
            // translate coordinates
            if (!XTranslateCoordinates(X11->display, src, target, lx, ly, &lx2, &ly2, &t)) {
                target = 0;
                break;
            }
            lx = lx2;
            ly = ly2;
            src = target;

	    // check if it has XdndAware
	    Atom type = 0;
	    int f;
	    unsigned long n, a;
	    unsigned char *data = 0;
	    XGetWindowProperty(X11->display, target, ATOM(XdndAware), 0, 0, False,
                               AnyPropertyType, &type, &f,&n,&a,&data);
	    if (data)
                XFree(data);
	    if (type) {
                DNDDEBUG << "Found XdndAware on " << QWidget::find(target) << target;
                break;
            }

            // find child at the coordinates
            if (!XTranslateCoordinates(X11->display, src, src, lx, ly, &lx2, &ly2, &target)) {
                target = 0;
                break;
            }
        }
        if (xdnd_data.deco && (!target || target == xdnd_data.deco->winId())) {
            DNDDEBUG << "need to find real window";
            target = findRealWindow(globalPos, rootwin, 6);
            DNDDEBUG << "real window found" << QWidget::find(target) << target;
        }
    }

    QWidget* w;
    if (target) {
        w = QWidget::find((WId)target);
        if (w && (w->windowType() == Qt::Desktop) && !w->acceptDrops())
            w = 0;
    } else {
        w = 0;
        target = rootwin;
    }

    DNDDEBUG << "and the final target is " << QWidget::find(target) << target;
    DNDDEBUG << "the widget w is" << w;

    WId proxy_target = xdndProxy(target);
    if (!proxy_target)
        proxy_target = target;
    int target_version = 1;

    if (proxy_target) {
        Atom   type = XNone;
        int r, f;
        unsigned long n, a;
        int *tv;
        X11->ignoreBadwindow();
        r = XGetWindowProperty(X11->display, proxy_target, ATOM(XdndAware), 0,
                               1, False, AnyPropertyType, &type, &f,&n,&a,(uchar**)&tv);
        if (r != Success || X11->badwindow()) {
            target = 0;
        } else {
            target_version = qMin(xdnd_version,tv ? *tv : 1);
            if (tv)
                XFree(tv);
//             if (!(!X11->badwindow() && type))
//                 target = 0;
        }
    }

    if (target != qt_xdnd_current_target) {
        if (qt_xdnd_current_target)
            qt_xdnd_send_leave();

        qt_xdnd_current_target = target;
        qt_xdnd_current_proxy_target = proxy_target;
        if (target) {
            QVector<Atom> types;
            int flags = target_version << 24;
            QStringList fmts = QInternalMimeData::formatsHelper(dragPrivate()->data);
            for (int i = 0; i < fmts.size(); ++i) {
                QList<Atom> atoms = X11->xdndMimeAtomsForFormat(fmts.at(i));
                for (int j = 0; j < atoms.size(); ++j) {
                    if (!types.contains(atoms.at(j)))
                        types.append(atoms.at(j));
                }
            }
            if (types.size() > 3) {
                XChangeProperty(X11->display,
                                dragPrivate()->source->winId(), ATOM(XdndTypelist),
                                XA_ATOM, 32, PropModeReplace,
                                (unsigned char *)types.data(),
                                types.size());
                flags |= 0x0001;
            }
            XClientMessageEvent enter;
            enter.type = ClientMessage;
            enter.window = target;
            enter.format = 32;
            enter.message_type = ATOM(XdndEnter);
            enter.data.l[0] = dragPrivate()->source->winId();
            enter.data.l[1] = flags;
            enter.data.l[2] = types.size()>0 ? types.at(0) : 0;
            enter.data.l[3] = types.size()>1 ? types.at(1) : 0;
            enter.data.l[4] = types.size()>2 ? types.at(2) : 0;
            // provisionally set the rectangle to 5x5 pixels...
            qt_xdnd_source_sameanswer = QRect(globalPos.x() - 2,
                                              globalPos.y() -2 , 5, 5);

            DEBUG("sending Xdnd enter");
            if (w)
                X11->xdndHandleEnter(w, (const XEvent *)&enter, false);
            else if (target)
                XSendEvent(X11->display, proxy_target, False, NoEventMask, (XEvent*)&enter);
            waiting_for_status = false;
        }
    }
    if (waiting_for_status)
        return;

    if (target) {
        waiting_for_status = true;

        XClientMessageEvent move;
        move.type = ClientMessage;
        move.window = target;
        move.format = 32;
        move.message_type = ATOM(XdndPosition);
        move.window = target;
        move.data.l[0] = dragPrivate()->source->winId();
        move.data.l[1] = 0; // flags
        move.data.l[2] = (globalPos.x() << 16) + globalPos.y();
        move.data.l[3] = X11->time;
        move.data.l[4] = qtaction_to_xdndaction(defaultAction(dragPrivate()->possible_actions, QApplication::keyboardModifiers()));
        DEBUG("sending Xdnd position");

        if (w)
            handle_xdnd_position(w, (const XEvent *)&move, false);
        else
            XSendEvent(X11->display, proxy_target, False, NoEventMask,
                       (XEvent*)&move);
    } else {
        if (willDrop) {
            willDrop = false;
            updateCursor();
        }
    }
    DEBUG() << "QDragManager::move leave";
}


void QDragManager::drop()
{
    Q_ASSERT(heartbeat != -1);
    killTimer(heartbeat);
    heartbeat = -1;
    if (!qt_xdnd_current_target)
        return;

    delete xdnd_data.deco;
    xdnd_data.deco = 0;

    XClientMessageEvent drop;
    drop.type = ClientMessage;
    drop.window = qt_xdnd_current_target;
    drop.format = 32;
    drop.message_type = ATOM(XdndDrop);
    drop.data.l[0] = dragPrivate()->source->winId();
    drop.data.l[1] = 0; // flags
    drop.data.l[2] = X11->time;

    drop.data.l[3] = 0;
    drop.data.l[4] = 0;

    QWidget * w = QWidget::find(qt_xdnd_current_proxy_target);

    if (w && (w->windowType() == Qt::Desktop) && !w->acceptDrops())
        w = 0;

    if (w)
        X11->xdndHandleDrop(w, (const XEvent *)&drop, false);
    else
        XSendEvent(X11->display, qt_xdnd_current_proxy_target, False,
                    NoEventMask, (XEvent*)&drop);

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
}



bool QX11Data::xdndHandleBadwindow()
{
    QDragManager *manager = QDragManager::self();
    if (manager->object && qt_xdnd_current_target) {
        qt_xdnd_current_target = 0;
        qt_xdnd_current_proxy_target = 0;
        manager->object->deleteLater();
        manager->object = 0;
        delete xdnd_data.deco;
        xdnd_data.deco = 0;
        return true;
    }
    if (qt_xdnd_dragsource_xid) {
        qt_xdnd_dragsource_xid = 0;
        if (qt_xdnd_current_widget) {
            QDragLeaveEvent e;
            QApplication::sendEvent(qt_xdnd_current_widget, &e);
            qt_xdnd_current_widget = 0;
        }
        return true;
    }
    return false;
}

void QX11Data::xdndHandleSelectionRequest(const XSelectionRequestEvent * req)
{
    if (!req)
        return;
    XEvent evt;
    evt.xselection.type = SelectionNotify;
    evt.xselection.display = req->display;
    evt.xselection.requestor = req->requestor;
    evt.xselection.selection = req->selection;
    evt.xselection.target = XNone;
    evt.xselection.property = XNone;
    evt.xselection.time = req->time;
    Atom atomFormat = req->target;
    int dataFormat = 0;
    QByteArray data;
    if (X11->xdndMimeDataForAtom(req->target, QDragManager::self()->dragPrivate()->data,
                            &data, &atomFormat, &dataFormat)) {
        int dataSize = data.size() / (dataFormat / 8);
        XChangeProperty (X11->display, req->requestor, req->property,
                         atomFormat, dataFormat, PropModeReplace,
                         (unsigned char *)data.data(), dataSize);
        evt.xselection.property = req->property;
        evt.xselection.target = atomFormat;
    }
    // ### this can die if req->requestor crashes at the wrong
    // ### moment
    XSendEvent(X11->display, req->requestor, False, 0, &evt);
}

static QByteArray xdndObtainData(const char *format)
{
    QByteArray result;

    QWidget* w;
    QDragManager *manager = QDragManager::self();
    if (qt_xdnd_dragsource_xid && manager->object &&
         (w=QWidget::find(qt_xdnd_dragsource_xid))
         && (!(w->windowType() == Qt::Desktop) || w->acceptDrops()))
    {
        QDragPrivate * o = QDragManager::self()->dragPrivate();
        if (o->data->hasFormat(QLatin1String(format)))
            result = o->data->data(QLatin1String(format));
        return result;
    }

    QList<Atom> atoms;
    int i = 0;
    while ((qt_xdnd_types[i])) {
        atoms.append(qt_xdnd_types[i]);
        ++i;
    }
    Atom a = X11->xdndMimeAtomForFormat(format, atoms);
    if (!a)
        return result;

    if (XGetSelectionOwner(X11->display, ATOM(XdndSelection)) == XNone)
        return result; // should never happen?

    QWidget* tw = qt_xdnd_current_widget;
    if (!qt_xdnd_current_widget || (qt_xdnd_current_widget->windowType() == Qt::Desktop))
        tw = new QWidget;

    XConvertSelection(X11->display, ATOM(XdndSelection), a, ATOM(XdndSelection), tw->winId(),
                      qt_xdnd_target_current_time);
    XFlush(X11->display);

    XEvent xevent;
    bool got=X11->clipboardWaitForEvent(tw->winId(), SelectionNotify, &xevent, 5000);
    if (got) {
        Atom type;

        if (X11->clipboardReadProperty(tw->winId(), ATOM(XdndSelection), true, &result, 0, &type, 0, false)) {
            if (type == ATOM(INCR)) {
                int nbytes = result.size() >= 4 ? *((int*)result.data()) : 0;
                result = X11->clipboardReadIncrementalProperty(tw->winId(), ATOM(XdndSelection), nbytes, false);
            } else if (type != a && type != XNone) {
                DEBUG("Qt clipboard: unknown atom %ld", type);
            }
        }
    }
    if (!qt_xdnd_current_widget || (qt_xdnd_current_widget->windowType() == Qt::Desktop))
        delete tw;
   
    return X11->xdndMimeConvertToFormat(a, result, format);
}


/*
  Enable drag and drop for widget w by installing the proper
  properties on w's toplevel widget.
*/
bool QX11Data::dndEnable(QWidget* w, bool on)
{
    w = w->window();

    if (on) {
        if (((QExtraWidget*)w)->topData()->dnd)
            return true; // been there, done that
        ((QExtraWidget*)w)->topData()->dnd  = 1;
    }

    motifdndEnable(w, on);
    return xdndEnable(w, on);
}

Qt::DropAction QDragManager::drag(QDrag * o)
{
    if (object == o || !o || !o->d_func()->source)
        return Qt::IgnoreAction;

    if (object) {
        cancel();
        qApp->removeEventFilter(this);
        beingCancelled = false;
    }

    if (object) {
        // the last drag and drop operation hasn't finished, so we are going to wait
        // for one second to see if it does... if the finish message comes after this,
        // then we could still have problems, but this is highly unlikely
        QApplication::flush();

        QTime started = QTime::currentTime();
        QTime now = started;
        do {
            XEvent event;
            if (XCheckTypedEvent(X11->display, ClientMessage, &event))
                qApp->x11ProcessEvent(&event);

            now = QTime::currentTime();
            if (started > now) // crossed midnight
                started = now;

            // sleep 50ms, so we don't use up CPU cycles all the time.
            struct timeval usleep_tv;
            usleep_tv.tv_sec = 0;
            usleep_tv.tv_usec = 50000;
            select(0, 0, 0, 0, &usleep_tv);
        } while (object && started.msecsTo(now) < 1000);
    }

    object = o;
    object->d_func()->target = 0;
    xdnd_data.deco = new QShapedPixmapWidget();

    willDrop = false;

    updatePixmap();

    qApp->installEventFilter(this);
    XSetSelectionOwner(X11->display, ATOM(XdndSelection), dragPrivate()->source->window()->winId(), X11->time);
    global_accepted_action = Qt::CopyAction;
    qt_xdnd_source_sameanswer = QRect();
    move(QCursor::pos());
    heartbeat = startTimer(200);

#ifndef QT_NO_CURSOR
    qApp->setOverrideCursor(Qt::ArrowCursor);
    restoreCursor = true;
    updateCursor();
#endif

    qt_xdnd_dragging = true;

    if (!QWidget::mouseGrabber())
        xdnd_data.deco->grabMouse();

    eventLoop = new QEventLoop;
    (void) eventLoop->exec();
    delete eventLoop;
    eventLoop = 0;

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        qApp->restoreOverrideCursor();
        restoreCursor = false;
    }
#endif

    // delete cursors as they may be different next drag.
    delete noDropCursor;
    noDropCursor = 0;
    delete copyCursor;
    copyCursor = 0;
    delete moveCursor;
    moveCursor = 0;
    delete linkCursor;
    linkCursor = 0;

    delete xdnd_data.deco;
    xdnd_data.deco = 0;
    if (heartbeat != -1)
        killTimer(heartbeat);
    heartbeat = -1;
    qt_xdnd_current_screen = -1;
    qt_xdnd_dragging = false;

    return global_accepted_action;
    // object persists until we get an xdnd_finish message
}

void QDragManager::updatePixmap()
{
    if (xdnd_data.deco) {
        QPixmap pm;
        QPoint pm_hot(default_pm_hotx,default_pm_hoty);
        if (object) {
            pm = dragPrivate()->pixmap;
            if (!pm.isNull())
                pm_hot = dragPrivate()->hotspot;
        }
        if (pm.isNull()) {
            if (!defaultPm)
                defaultPm = new QPixmap(default_pm);
            pm = *defaultPm;
        }
        xdnd_data.deco->pm_hot = pm_hot;
        xdnd_data.deco->setPixmap(pm);
        xdnd_data.deco->move(QCursor::pos()-pm_hot);
        xdnd_data.deco->show();
    }
}

QVariant QDropData::retrieveData_sys(const QString &mimetype, QVariant::Type) const
{
    QByteArray mime = mimetype.toLatin1();
    QByteArray data = X11->motifdnd_active
                      ? X11->motifdndObtainData(mime)
                      : xdndObtainData(mime);
    return data;
}

bool QDropData::hasFormat_sys(const QString &format) const
{
    return formats().contains(format);
}

QStringList QDropData::formats_sys() const
{
    QStringList formats;
    if (X11->motifdnd_active) {
        int i = 0;
        QByteArray fmt;
        while (!(fmt = X11->motifdndFormat(i)).isEmpty()) {
            formats.append(QLatin1String(fmt));
            ++i;
        }
    } else {
        int i = 0;
        while ((qt_xdnd_types[i])) {
            QStringList formatsForAtom = X11->xdndMimeFormatsForAtom(qt_xdnd_types[i]);
            for (int j = 0; j < formatsForAtom.size(); ++j) {
                if (!formats.contains(formatsForAtom.at(j)))
                    formats.append(formatsForAtom.at(j));
            }
            ++i;
        }
    }
    return formats;
}

#endif // QT_NO_DRAGANDDROP

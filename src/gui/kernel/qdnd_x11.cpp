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
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qdatetime.h"
#include "qiodevice.h"
#include "qpointer.h"
#include "qcursor.h"
#include "qvector.h"

#include "qdnd_p.h"
#include "qt_x11_p.h"
#include "qx11info_x11.h"
#include "qdebug.h"

#include "qwidget_p.h"
#define d d_func()
#define q q_func()

#define DND_DEBUG
#ifdef DND_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if(0) qDebug
#endif

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

extern void qt_enter_modal(QWidget *widget);
extern void qt_leave_modal(QWidget *widget);

#if defined(Q_C_CALLBACKS)
}
#endif

extern Window qt_x11_findClientWindow(Window, Atom, bool);

// this stuff is copied from qclb_x11.cpp

extern bool qt_xclb_wait_for_event(Display *dpy, Window win, int type,
                                    XEvent *event, int timeout);
extern bool qt_xclb_read_property(Display *dpy, Window win, Atom property,
                                   bool deleteProperty,
                                   QByteArray *buffer, int *size, Atom *type,
                                   int *format, bool nullterm);
extern QByteArray qt_xclb_read_incremental_property(Display *dpy, Window win,
                                                     Atom property,
                                                     int nbytes, bool nullterm);
// and all this stuff is copied -into- qapp_x11.cpp

void qt_xdnd_setup();
void qt_handle_xdnd_enter(QWidget *, const XEvent *, bool);
void qt_handle_xdnd_position(QWidget *, const XEvent *, bool);
void qt_handle_xdnd_status(QWidget *, const XEvent *, bool);
void qt_handle_xdnd_leave(QWidget *, const XEvent *, bool);
void qt_handle_xdnd_drop(QWidget *, const XEvent *, bool);
void qt_handle_xdnd_finished(QWidget *, const XEvent *, bool);
void qt_xdnd_handle_selection_request(const XSelectionRequestEvent *);
bool qt_xdnd_handle_badwindow();
// client messages
const int qt_xdnd_version = 4;

// Actions
//
// The Xdnd spec allows for user-defined actions. This could be implemented
// with a registration process in Qt. WE SHOULD do that later.
//

static QDrag::DropAction xdndaction_to_qtaction(Atom atom)
{
    if (atom == ATOM(XdndActionCopy) || atom == 0)
        return QDrag::CopyAction;
    if (atom == ATOM(XdndActionLink))
        return QDrag::LinkAction;
    if (atom == ATOM(XdndActionMove))
        return QDrag::MoveAction;
    return QDrag::CopyAction;
}
static
int qtaction_to_xdndaction(QDrag::DropAction a)
{
    switch (a) {
      case QDrag::CopyAction:
        return ATOM(XdndActionCopy);
      case QDrag::LinkAction:
        return ATOM(XdndActionLink);
      case QDrag::MoveAction:
        return ATOM(XdndActionMove);
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
//static QRect qt_xdnd_target_sameanswer;
static bool qt_xdnd_target_answerwas;
// top-level window we sent position to last.
static Window qt_xdnd_current_target;
// window to send events to (always valid if qt_xdnd_current_target)
static Window qt_xdnd_current_proxy_target;
// widget we forwarded position to last, and local position
static QPointer<QWidget> qt_xdnd_current_widget;
static QPoint qt_xdnd_current_position;
// time of this drop, as type Atom to save on casts
static Atom qt_xdnd_source_current_time;
//NOTUSED static Atom qt_xdnd_target_current_time;
// screen number containing the pointer... -1 means default
static int qt_xdnd_current_screen = -1;
// state of dragging... true if dragging, false if not
bool qt_xdnd_dragging = false;

// first drag object, or 0
static QDrag * qt_xdnd_source_object = 0;

// Motif dnd
extern void qt_motifdnd_enable(QWidget *, bool);
extern QByteArray qt_motifdnd_obtain_data(const char *format);
extern const char *qt_motifdnd_format(int n);

bool qt_motifdnd_active = false;
static bool dndCancelled = false;

// Shift/Ctrl handling, and final drop status
static QDrag::DropAction global_requested_action = QDrag::CopyAction;
static QDrag::DropAction global_accepted_action = QDrag::CopyAction;

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
                Qt::WStyle_Customize | Qt::WStyle_Tool | Qt::WStyle_NoBorder | Qt::WX11BypassWM)
    {
    }

    void setPixmap(QPixmap pm)
    {
        if (pm.mask()) {
            setMask(*pm.mask());
        } else {
            clearMask();
        }
        resize(pm.width(),pm.height());
	QPalette p = palette();
	p.setBrush(backgroundRole(), QBrush(pm));
	setPalette(p);
    }
};

static QShapedPixmapWidget * qt_xdnd_deco = 0;

static QWidget* desktop_proxy = 0;

class QExtraWidget : public QWidget
{
    Q_DECLARE_PRIVATE(QWidget)
public:
    inline QWExtra* extraData();
    inline QTLWExtra* topData();
};

inline QWExtra* QExtraWidget::extraData() { return d->extraData(); }
inline QTLWExtra* QExtraWidget::topData() { return d->topData(); }

static bool qt_xdnd_enable(QWidget* w, bool on)
{
    if (on) {
        QWidget * xdnd_widget = 0;
        if (w->isDesktop()) {
            if (desktop_proxy) // *WE* already have one.
                return false;

            // As per Xdnd4, use XdndProxy
            XGrabServer(w->x11Info().display());
            Atom type = XNone;
            int f;
            unsigned long n, a;
            WId *proxy_id_ptr;
            XGetWindowProperty(w->x11Info().display(), w->winId(),
                                ATOM(XdndProxy), 0, 1, False,
                XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id_ptr);
            WId proxy_id = 0;
            if (type == XA_WINDOW && proxy_id_ptr) {
                proxy_id = *proxy_id_ptr;
                XFree(proxy_id_ptr);
                proxy_id_ptr = 0;
                // Already exists. Real?
                X11->ignoreBadwindow();
                XGetWindowProperty(w->x11Info().display(), proxy_id,
                    ATOM(XdndProxy), 0, 1, False,
                    XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id_ptr);
                if (X11->badwindow() || type != XA_WINDOW || !proxy_id_ptr || *proxy_id_ptr != proxy_id) {
                    // Bogus - we will overwrite.
                    proxy_id = 0;
                }
            }
            if (proxy_id_ptr)
                XFree(proxy_id_ptr);

            if (!proxy_id) {
                xdnd_widget = desktop_proxy = new QWidget;
                proxy_id = desktop_proxy->winId();
                XChangeProperty (w->x11Info().display(),
                    w->winId(), ATOM(XdndProxy),
                    XA_WINDOW, 32, PropModeReplace,
                    (unsigned char *)&proxy_id, 1);
                XChangeProperty (w->x11Info().display(),
                    proxy_id, ATOM(XdndProxy),
                    XA_WINDOW, 32, PropModeReplace,
                    (unsigned char *)&proxy_id, 1);
            }

            XUngrabServer(w->x11Info().display());
        } else {
            xdnd_widget = w->topLevelWidget();
        }
        if (xdnd_widget) {
            Atom atm = (Atom)qt_xdnd_version;
            XChangeProperty (xdnd_widget->x11Info().display(), xdnd_widget->winId(),
                              ATOM(XdndAware), XA_ATOM, 32, PropModeReplace,
                              (unsigned char *)&atm, 1);
            return true;
        } else {
            return false;
        }
    } else {
        if (w->isDesktop()) {
            XDeleteProperty(w->x11Info().display(), w->winId(),
                             ATOM(XdndProxy));
            delete desktop_proxy;
            desktop_proxy = 0;
        }
        return true;
    }
}

const char* qt_xdnd_atom_to_str(Atom a)
{
    if (!a) return 0;

    if (a == XA_STRING)
        return "text/plain"; // some Xdnd clients are dumb

    return XGetAtomName(QX11Info::display(), a);
}

Atom qt_xdnd_str_to_atom(const char *mimeType)
{
    if (!mimeType || !*mimeType)
        return 0;
    return XInternAtom(QX11Info::display(), mimeType, False);
}


void qt_xdnd_setup() {
    QCursor::initialize();
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
    delete desktop_proxy;
    desktop_proxy = 0;
}


static QWidget * find_child(QWidget * tlw, QPoint & p)
{
    QWidget * w = tlw;

    p = w->mapFromGlobal(p);
    bool done = false;
    while (!done) {
        done = true;
        if (((QExtraWidget*)w)->extraData() &&
             ((QExtraWidget*)w)->extraData()->xDndProxy != 0)
            break; // stop searching for widgets under the mouse cursor if found widget is a proxy.
        QObjectList children = w->children();
        if (!children.isEmpty()) {
            for(int i = children.size(); i > 0;) {
                --i;
                QObject * o = children.at(i);
                if (o->isWidgetType() &&
                     ((QWidget*)o)->isVisible() &&
                     ((QWidget*)o)->geometry().contains(p) &&
                     !((QWidget*)o)->isTopLevel()) {
                    w = (QWidget *)o;
                    done = false;
                    p = w->mapFromParent(p);
                    break;
                }
            }
        }
    }
    return w;
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
            XSendEvent(QX11Info::display(), extra->xDndProxy, False, NoEventMask,
                        &last_enter_event);
            current_embedding_widget = w;
        }

        ((XEvent*)xe)->xany.window = extra->xDndProxy;
        XSendEvent(QX11Info::display(), extra->xDndProxy, False, NoEventMask,
                    (XEvent*)xe);
        qt_xdnd_current_widget = w;
        return true;
    }
    current_embedding_widget = 0;
    return false;
}

void qt_handle_xdnd_enter(QWidget *, const XEvent * xe, bool /*passive*/)
{
    //if (!w->neveHadAChildWithDropEventsOn())
        //return; // haven't been set up for dnd

    qt_motifdnd_active = false;

    last_enter_event.xclient = xe->xclient;

    qt_xdnd_target_answerwas = false;

    const long *l = xe->xclient.data.l;
    int version = (int)(((unsigned long)(l[1])) >> 24);

    if (version > qt_xdnd_version)
        return;

    qt_xdnd_dragsource_xid = l[0];

    int j = 0;
    if (l[1] & 1) {
        // get the types from XdndTypeList
        Atom   type = XNone;
        int f;
        unsigned long n, a;
        Atom *data;
        XGetWindowProperty(QX11Info::display(), qt_xdnd_dragsource_xid,
                    ATOM(XdndTypelist), 0,
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



void qt_handle_xdnd_position(QWidget *w, const XEvent * xe, bool passive)
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QPoint p((l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff);
    QWidget * c = find_child(w, p); // changes p to to c-local coordinates

    if (!passive && checkEmbedded(c, xe))
        return;

    if (!c || !c->acceptDrops() && c->isDesktop()) {
        return;
    }

    if (l[0] != qt_xdnd_dragsource_xid) {
        DEBUG("xdnd drag position from unexpected source (%08lx not %08lx)", l[0], qt_xdnd_dragsource_xid);
        return;
    }

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
        while (c && !c->acceptDrops() && !c->isTopLevel()) {
            p = c->mapToParent(p);
            c = c->parentWidget();
        }

        QRect answerRect(c->mapToGlobal(p), QSize(1,1));

        QDrag::DropAction accepted_action = xdndaction_to_qtaction(l[4]);
        QDragMoveEvent me(p, accepted_action, QDragManager::self()->dropData);

        if (c != qt_xdnd_current_widget) {
            qt_xdnd_target_answerwas = false;
            if (qt_xdnd_current_widget) {
                QDragLeaveEvent e;
                QApplication::sendEvent(qt_xdnd_current_widget, &e);
            }
            if (c->acceptDrops()) {
                qt_xdnd_current_widget = c;
                qt_xdnd_current_position = p;
                //NOTUSED qt_xdnd_target_current_time = l[3]; // will be 0 for xdnd1

                QDragEnterEvent de(p, accepted_action, QDragManager::self()->dropData);
                QApplication::sendEvent(c, &de);
                if (de.isAccepted()) {
                    me.accept(de.answerRect());
                    accepted_action = de.dropAction();
                } else {
                    me.ignore(de.answerRect());
                }
            }
        } else {
            if (qt_xdnd_target_answerwas) {
                me.accept();
                me.acceptAction(global_requested_action == global_accepted_action);
            }
        }

         DEBUG() << "qt_handle_xdnd_position action=" << qt_xdnd_atom_to_str(l[4]);
        if (!c->acceptDrops()) {
            qt_xdnd_current_widget = 0;
            answerRect = QRect(p, QSize(1, 1));
        } else {
            qt_xdnd_current_widget = c;
            qt_xdnd_current_position = p;
            //NOTUSED qt_xdnd_target_current_time = l[3]; // will be 0 for xdnd1

            QApplication::sendEvent(c, &me);
            qt_xdnd_target_answerwas = me.isAccepted();
            if (me.isAccepted()) {
                response.data.l[1] = 1; // yes
                accepted_action = me.dropAction();
            } else {
                response.data.l[0] = 0;
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
        global_accepted_action = accepted_action;
    }

    QWidget * source = QWidget::find(qt_xdnd_dragsource_xid);

    if (source && source->isDesktop() && !source->acceptDrops())
        source = 0;

    if (source)
        qt_handle_xdnd_status(source, (const XEvent *)&response, passive);
    else
        XSendEvent(QX11Info::display(), qt_xdnd_dragsource_xid, False,
                    NoEventMask, (XEvent*)&response);
}


void qt_handle_xdnd_status(QWidget * w, const XEvent * xe, bool /*passive*/)
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;
    // Messy:  QDragResponseEvent is just a call to QDragManager function
    global_accepted_action = xdndaction_to_qtaction(l[4]);
    QDragResponseEvent e((int)(l[1] & 1));
    QApplication::sendEvent(w, &e);

    DEBUG() << "qt_handle_xdnd_status accion=" << qt_xdnd_atom_to_str(l[4]);

    if ((int)(l[1] & 2) == 0) {
        QPoint p((l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff);
        QSize s((l[3] & 0xffff0000) >> 16, l[3] & 0x0000ffff);
        qt_xdnd_source_sameanswer = QRect(p, s);
        if (qt_xdnd_source_sameanswer.isNull()) {
            // Application wants "coninutous" move events
        }
    } else {
        qt_xdnd_source_sameanswer = QRect();
    }
}


void qt_handle_xdnd_leave(QWidget *w, const XEvent * xe, bool /*passive*/)
{
    DEBUG("xdnd leave");
    if (!qt_xdnd_current_widget ||
         w->topLevelWidget() != qt_xdnd_current_widget->topLevelWidget()) {
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

    if (w && w->isDesktop() && !w->acceptDrops())
        w = 0;

    if (w)
        qt_handle_xdnd_leave(w, (const XEvent *)&leave, false);
    else
        XSendEvent(QX11Info::display(), qt_xdnd_current_proxy_target, False,
                    NoEventMask, (XEvent*)&leave);
    qt_xdnd_current_target = 0;
    qt_xdnd_current_proxy_target = 0;
}



void qt_handle_xdnd_drop(QWidget *, const XEvent * xe, bool passive)
{
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
    if (qt_xdnd_source_object)
        manager->dragPrivate(qt_xdnd_source_object)->target = qt_xdnd_current_widget;

    if (!passive) {
        QDropEvent de(qt_xdnd_current_position, global_accepted_action, QDragManager::self()->dropData);
        QApplication::sendEvent(qt_xdnd_current_widget, &de);
        if (!de.isAccepted()) {
            // Ignore a failed drag
            global_accepted_action = QDrag::CopyAction;
            dndCancelled = true;
        }
        XClientMessageEvent finished;
        finished.type = ClientMessage;
        finished.window = qt_xdnd_dragsource_xid;
        finished.format = 32;
        finished.message_type = ATOM(XdndFinished);
        finished.data.l[0] = qt_xdnd_current_widget?qt_xdnd_current_widget->topLevelWidget()->winId():0;
        finished.data.l[1] = 0; // flags
        XSendEvent(QX11Info::display(), qt_xdnd_dragsource_xid, False,
                    NoEventMask, (XEvent*)&finished);
    } else {
        QDragLeaveEvent e;
        QApplication::sendEvent(qt_xdnd_current_widget, &e);
    }
    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_current_widget = 0;
}


void qt_handle_xdnd_finished(QWidget *, const XEvent * xe, bool passive)
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    if (l[0] && (l[0] == qt_xdnd_current_target
            || l[0] == qt_xdnd_current_proxy_target)) {
        //
        if (!passive)
            (void) checkEmbedded(qt_xdnd_current_widget, xe);
        current_embedding_widget = 0;
        qt_xdnd_current_target = 0;
        qt_xdnd_current_proxy_target = 0;
        if (qt_xdnd_source_object)
            qt_xdnd_source_object->deleteLater();
        qt_xdnd_source_object = 0;
    }
}


void QDragManager::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == heartbeat && qt_xdnd_source_sameanswer.isNull())
        move(QCursor::pos());
}

bool QDragManager::eventFilter(QObject * o, QEvent * e)
{
    if (beingCancelled) {
        if (e->type() == QEvent::KeyRelease &&
             ((QKeyEvent*)e)->key() == Qt::Key_Escape) {
            qApp->removeEventFilter(this);
            object = 0;
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
        updateMode(me->modifiers());
        move(me->globalPos());
        return true;
    } else if (e->type() == QEvent::MouseButtonRelease) {
        qApp->removeEventFilter(this);
        if (willDrop)
            drop();
        else
            cancel();
        object = 0;
        beingCancelled = false;
        eventLoop->exit();
        return true;
    } else if (e->type() == QEvent::DragResponse) {
        if (((QDragResponseEvent *)e)->dragAccepted()) {
            if (!willDrop) {
                willDrop = true;
            }
        } else {
            if (willDrop) {
                willDrop = false;
            }
        }
        updateCursor();
        return true;
    }

    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = ((QKeyEvent*)e);
        if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress) {
            cancel();
            qApp->removeEventFilter(this);
            object = 0;
            beingCancelled = false;
            eventLoop->exit();
        } else {
            DEBUG() << "keyEvent: modifs=" << ke->modifiers();
            updateMode(ke->modifiers());
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
#ifdef QT_COMPAT
      case QEvent::Accel:
      case QEvent::AccelAvailable:
      case QEvent::AccelOverride:
#endif
        return true;
      default:
        return false;
    }
}


void QDragManager::updateMode(Qt::KeyboardModifiers modifiers)
{
    const int both = Qt::ShiftModifier|Qt::ControlModifier;
    if ((modifiers & both) == both && (object->d->possible_actions & QDrag::LinkAction)) {
        global_requested_action = QDrag::LinkAction;
    } else if ((modifiers & Qt::ShiftButton) && (object->d->possible_actions & QDrag::MoveAction)) {
        global_requested_action = QDrag::MoveAction;
    } else {
        global_requested_action = QDrag::CopyAction;
    }
}


void QDragManager::updateCursor()
{
    if (!noDropCursor) {
        noDropCursor = new QCursor(Qt::ForbiddenCursor);
        if (!pm_cursor[0].isNull())
            moveCursor = new QCursor(pm_cursor[0], 0,0);
        if (!pm_cursor[1].isNull())
            copyCursor = new QCursor(pm_cursor[1], 0,0);
        if (!pm_cursor[2].isNull())
            linkCursor = new QCursor(pm_cursor[2], 0,0);
    }

    QCursor *c;
    if (willDrop) {
        if (global_accepted_action == QDrag::CopyAction) {
            if (global_requested_action == QDrag::MoveAction)
                c = moveCursor; // (source can delete)
            else
                c = copyCursor;
        } else if (global_accepted_action == QDrag::LinkAction) {
            c = linkCursor;
        } else {
            c = moveCursor;
        }
        if (qt_xdnd_deco) {
            qt_xdnd_deco->show();
            qt_xdnd_deco->raise();
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
    killTimer(heartbeat);
    heartbeat = -1;
    if (object) {
        beingCancelled = true;
        object = 0;
    }

    if (qt_xdnd_current_target) {
        qt_xdnd_send_leave();
    }

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif

    if (deleteSource && qt_xdnd_source_object)
        qt_xdnd_source_object->deleteLater();
    qt_xdnd_source_object = 0;
    delete qt_xdnd_deco;
    qt_xdnd_deco = 0;

    dndCancelled = true;
}

static
Window findRealWindow(const QPoint & pos, Window w, int md)
{
    if (qt_xdnd_deco && w == qt_xdnd_deco->winId())
        return 0;

    if (md) {
        XWindowAttributes attr;
        XGetWindowAttributes(QX11Info::display(), w, &attr);

        if (attr.map_state != IsUnmapped
            && QRect(attr.x,attr.y,attr.width,attr.height)
                .contains(pos))
        {
            {
                Atom   type = XNone;
                int f;
                unsigned long n, a;
                unsigned char *data;

                XGetWindowProperty(QX11Info::display(), w, ATOM(WM_STATE), 0,
                    0, False, AnyPropertyType, &type, &f,&n,&a,&data);
                if (data) XFree(data);
                if (type) return w;
            }

            Window r, p;
            Window* c;
            uint nc;
            if (XQueryTree(QX11Info::display(), w, &r, &p, &c, &nc)) {
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
    Q_ASSERT(object != 0);
    int screen = QCursor::x11Screen();
    if ((qt_xdnd_current_screen == -1 && screen != QX11Info::appScreen()) ||
         (screen != qt_xdnd_current_screen)) {
        // recreate the pixmap on the new screen...
        delete qt_xdnd_deco;
        qt_xdnd_deco = new QShapedPixmapWidget(screen);
        if (!QWidget::mouseGrabber()) {
            updatePixmap();
            qt_xdnd_deco->grabMouse();
        }
    }
    updatePixmap();

    if (qt_xdnd_source_sameanswer.contains(globalPos) &&
         qt_xdnd_source_sameanswer.isValid()) {
        return;
    }

    qt_xdnd_current_screen = screen;
    Window rootwin = QX11Info::appRootWindow(qt_xdnd_current_screen);
    Window target = 0;
    int lx = 0, ly = 0;
    if (!XTranslateCoordinates(QX11Info::display(), rootwin, rootwin,
                                 globalPos.x(), globalPos.y(),
                                 &lx, &ly, &target))
        // some weird error...
        return;

    if (target == rootwin) {
        // Ok.
    } else if (target) {
        //me
        Window targetW = qt_x11_findClientWindow(target, ATOM(WM_STATE), true);
        if (targetW)
            target = targetW;
        if (qt_xdnd_deco && (!target || target == qt_xdnd_deco->winId())) {
            target = findRealWindow(globalPos,rootwin,6);
        }
    }

    QWidget* w;
    if (target) {
        w = QWidget::find((WId)target);
        if (w && w->isDesktop() && !w->acceptDrops())
            w = 0;
    } else {
        w = 0;
        target = rootwin;
    }

    WId proxy_target = target;
    int target_version = 1;

    {
        Atom   type = XNone;
        int r, f;
        unsigned long n, a;
        WId *proxy_id;
        X11->ignoreBadwindow();
        r = XGetWindowProperty(qt_xdisplay(), target, ATOM(XdndProxy), 0,
                                1, False, XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id);
        if ((r != Success) || X11->badwindow()) {
            proxy_target = target = 0;
        } else if (type == XA_WINDOW && proxy_id) {
            proxy_target = *proxy_id;
            XFree(proxy_id);
            proxy_id = 0;
            r = XGetWindowProperty(qt_xdisplay(), proxy_target, ATOM(XdndProxy), 0,
                                    1, False, XA_WINDOW, &type, &f,&n,&a,(uchar**)&proxy_id);
            if ((r != Success) || X11->badwindow() || !type || !proxy_id || *proxy_id != proxy_target) {
                // Bogus
                proxy_target = 0;
                target = 0;
            }
            if (proxy_id)
                XFree(proxy_id);
        }
        if (proxy_target) {
            int *tv;
            X11->ignoreBadwindow();
            r = XGetWindowProperty(qt_xdisplay(), proxy_target, ATOM(XdndAware), 0,
                                    1, False, AnyPropertyType, &type, &f,&n,&a,(uchar**)&tv);
            if (r != Success) {
                target = 0;
            } else {
                target_version = qMin(qt_xdnd_version,tv ? *tv : 1);
                if (tv)
                    XFree(tv);
                if (!(!X11->badwindow() && type))
                    target = 0;
            }
        }
    }

    if (target != qt_xdnd_current_target) {
        if (qt_xdnd_current_target)
            qt_xdnd_send_leave();

        qt_xdnd_current_target = target;
        qt_xdnd_current_proxy_target = proxy_target;
        if (target) {
            QVector<Atom> type;
            int flags = target_version << 24;
            QStringList fmts = object->d->data->formats();
            for (int i = 0; i < fmts.size(); ++i)
                type.append(qt_xdnd_str_to_atom(fmts.at(i).latin1()));
            if (type.size() > 3) {
                XChangeProperty(X11->display,
                                object->d->source->winId(), ATOM(XdndTypelist),
                                XA_ATOM, 32, PropModeReplace,
                                (unsigned char *)type.data(),
                                type.size());
                flags |= 0x0001;
            }
            XClientMessageEvent enter;
            enter.type = ClientMessage;
            enter.window = target;
            enter.format = 32;
            enter.message_type = ATOM(XdndEnter);
            enter.data.l[0] = object->d->source->winId();
            enter.data.l[1] = flags;
            enter.data.l[2] = type.size()>0 ? type.at(0) : 0;
            enter.data.l[3] = type.size()>1 ? type.at(1) : 0;
            enter.data.l[4] = type.size()>2 ? type.at(2) : 0;
            // provisionally set the rectangle to 5x5 pixels...
            qt_xdnd_source_sameanswer = QRect(globalPos.x() - 2,
                                               globalPos.y() -2 , 5, 5);

            if (w) {
                qt_handle_xdnd_enter(w, (const XEvent *)&enter, false);
            } else if (target) {
                XSendEvent(QX11Info::display(), proxy_target, False, NoEventMask,
                            (XEvent*)&enter);
            }
        }
    }

    if (target) {
        XClientMessageEvent move;
        move.type = ClientMessage;
        move.window = target;
        move.format = 32;
        move.message_type = ATOM(XdndPosition);
        move.window = target;
        move.data.l[0] = object->d->source->winId();
        move.data.l[1] = 0; // flags
        move.data.l[2] = (globalPos.x() << 16) + globalPos.y();
        move.data.l[3] = X11->time;
        move.data.l[4] = qtaction_to_xdndaction(global_requested_action);

        if (w)
            qt_handle_xdnd_position(w, (const XEvent *)&move, false);
        else
            XSendEvent(QX11Info::display(), proxy_target, False, NoEventMask,
                        (XEvent*)&move);
    } else {
        if (willDrop) {
            willDrop = false;
            updateCursor();
        }
    }
}


void QDragManager::drop()
{
    killTimer(heartbeat);
    heartbeat = -1;
    if (!qt_xdnd_current_target)
        return;

    delete qt_xdnd_deco;
    qt_xdnd_deco = 0;

    XClientMessageEvent drop;
    drop.type = ClientMessage;
    drop.window = qt_xdnd_current_target;
    drop.format = 32;
    drop.message_type = ATOM(XdndDrop);
    drop.data.l[0] = object->d->source->winId();
    drop.data.l[1] = 1 << 24; // flags
    drop.data.l[2] = 0; // ###
    drop.data.l[3] = X11->time;
    drop.data.l[4] = 0;

    QWidget * w = QWidget::find(qt_xdnd_current_proxy_target);

    if (w && w->isDesktop() && !w->acceptDrops())
        w = 0;

    if (w)
        qt_handle_xdnd_drop(w, (const XEvent *)&drop, false);
    else
        XSendEvent(QX11Info::display(), qt_xdnd_current_proxy_target, False,
                    NoEventMask, (XEvent*)&drop);

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
}



bool qt_xdnd_handle_badwindow()
{
    if (qt_xdnd_source_object && qt_xdnd_current_target) {
        qt_xdnd_current_target = 0;
        qt_xdnd_current_proxy_target = 0;
        delete qt_xdnd_source_object;
        qt_xdnd_source_object = 0;
        delete qt_xdnd_deco;
        qt_xdnd_deco = 0;
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


/*!
    \class QDragMoveEvent qevent.h
    \ingroup events
    \ingroup draganddrop
    \brief The QDragMoveEvent class provides an event which is sent while a drag and drop action is in progress.

    When a widget \link QWidget::setAcceptDrops() accepts drop
    events \endlink, it will receive this event repeatedly while the
    drag is within the widget's boundaries. The widget should examine
    the event to see what data it \link QDragMoveEvent::provides()
    provides \endlink, and accept() the drop if appropriate.

    Note that this class inherits most of its functionality from
    QDropEvent.
*/

void qt_xdnd_handle_selection_request(const XSelectionRequestEvent * req)
{
    if (!req)
        return;
    XEvent evt;
    evt.xselection.type = SelectionNotify;
    evt.xselection.display = req->display;
    evt.xselection.requestor = req->requestor;
    evt.xselection.selection = req->selection;
    evt.xselection.target = req->target;
    evt.xselection.property = XNone;
    evt.xselection.time = req->time;
    const char* format = qt_xdnd_atom_to_str(req->target);
    QDragPrivate* dp = QDragManager::self()->dragPrivate(qt_xdnd_source_object);
    if (format && dp->data->hasFormat(QLatin1String(format))) {
        QByteArray a = dp->data->data(QLatin1String(format));
        XChangeProperty (QX11Info::display(), req->requestor, req->property,
                         req->target, 8, PropModeReplace,
                         (unsigned char *)a.data(), a.size());
        evt.xselection.property = req->property;
    }
    // ### this can die if req->requestor crashes at the wrong
    // ### moment
    XSendEvent(QX11Info::display(), req->requestor, False, 0, &evt);
}

/*
        XChangeProperty (QX11Info::display(), req->requestor, req->property,
                          XA_STRING, 8,
                          PropModeReplace,
                          (uchar *)d->text(), strlen(d->text()));
        evt.xselection.property = req->property;
*/

static QByteArray qt_xdnd_obtain_data(const char *format)
{
    QByteArray result;

    QWidget* w;
    if (qt_xdnd_dragsource_xid && qt_xdnd_source_object &&
         (w=QWidget::find(qt_xdnd_dragsource_xid))
         && (!w->isDesktop() || w->acceptDrops()))
    {
        QDragPrivate * o = QDragManager::self()->dragPrivate(qt_xdnd_source_object);
        if (o->data->hasFormat(QLatin1String(format)))
            result = o->data->data(QLatin1String(format));
        return result;
    }

    Atom a = qt_xdnd_str_to_atom(format);
    if (!a)
        return result;

    if (XGetSelectionOwner(X11->display, ATOM(XdndSelection)) == XNone)
        return result; // should never happen?

    QWidget* tw = qt_xdnd_current_widget;
    if (!qt_xdnd_current_widget ||
         qt_xdnd_current_widget->isDesktop()) {
        tw = new QWidget;
    }
    XConvertSelection(QX11Info::display(),
                       ATOM(XdndSelection), a,
                       ATOM(XdndSelection),
                       tw->winId(), CurrentTime);
    XFlush(QX11Info::display());

    XEvent xevent;
    bool got=qt_xclb_wait_for_event(QX11Info::display(),
                                     tw->winId(),
                                     SelectionNotify, &xevent, 5000);
    if (got) {
        Atom type;

        if (qt_xclb_read_property(QX11Info::display(),
                                    tw->winId(),
                                    ATOM(XdndSelection), true,
                                    &result, 0, &type, 0, false)) {
            if (type == ATOM(INCR)) {
                int nbytes = result.size() >= 4 ? *((int*)result.data()) : 0;
                result = qt_xclb_read_incremental_property(QX11Info::display(),
                                                            tw->winId(),
                                                            ATOM(XdndSelection),
                                                            nbytes, false);
            } else if (type != a && type != XNone) {
                DEBUG("Qt clipboard: unknown atom %ld", type);
            }
        }
    }
    if (!qt_xdnd_current_widget ||
         qt_xdnd_current_widget->isDesktop()) {
        delete tw;
    }

    return result;
}


/*
  Enable drag and drop for widget w by installing the proper
  properties on w's toplevel widget.
*/
bool qt_dnd_enable(QWidget* w, bool on)
{
    w = w->topLevelWidget();

    if (on) {
        if (((QExtraWidget*)w)->topData()->dnd)
            return true; // been there, done that
        ((QExtraWidget*)w)->topData()->dnd  = 1;
    }

    qt_motifdnd_enable(w, on);
    return qt_xdnd_enable(w, on);
}

QDrag::DropAction QDragManager::drag(QDrag * o)
{
    if (object == o || !o || !o->d->source)
        return QDrag::IgnoreAction;

    if (object) {
        cancel();
        qApp->removeEventFilter(this);
        beingCancelled = false;
    }

    if (qt_xdnd_source_object) {
        // the last drag and drop operation hasn't finished, so we are going to wait
        // for one second to see if it does... if the finish message comes after this,
        // then we could still have problems, but this is highly unlikely
        QApplication::flush();

        QTime started = QTime::currentTime();
        QTime now = started;
        do {
            XEvent event;
            if (XCheckTypedEvent(QX11Info::display(),
                                   ClientMessage, &event))
                qApp->x11ProcessEvent(&event);

            now = QTime::currentTime();
            if (started > now) // crossed midnight
                started = now;

            // sleep 50ms, so we don't use up CPU cycles all the time.
            struct timeval usleep_tv;
            usleep_tv.tv_sec = 0;
            usleep_tv.tv_usec = 50000;
            select(0, 0, 0, 0, &usleep_tv);
        } while (qt_xdnd_source_object && started.msecsTo(now) < 1000);
    }

    qt_xdnd_source_object = o;
    qt_xdnd_source_object->d->target = 0;
    qt_xdnd_deco = new QShapedPixmapWidget();

    willDrop = false;

    object = o;
    updatePixmap();

    qApp->installEventFilter(this);
    qt_xdnd_source_current_time = X11->time;
    XSetSelectionOwner(QX11Info::display(), ATOM(XdndSelection),
                        object->d->source->topLevelWidget()->winId(),
                        qt_xdnd_source_current_time);
    global_accepted_action = QDrag::CopyAction;
    updateMode(0);
    qt_xdnd_source_sameanswer = QRect();
    move(QCursor::pos());
    heartbeat = startTimer(200);

#ifndef QT_NO_CURSOR
    qApp->setOverrideCursor(Qt::ArrowCursor);
    restoreCursor = true;
    updateCursor();
#endif

    dndCancelled = false;
    qt_xdnd_dragging = true;

    if (!QWidget::mouseGrabber())
        qt_xdnd_deco->grabMouse();

    eventLoop = new QEventLoop;
    (void) eventLoop->exec();
    delete eventLoop;
    eventLoop = 0;

#ifndef QT_NO_CURSOR
    qApp->restoreOverrideCursor();
#endif

    delete qt_xdnd_deco;
    qt_xdnd_deco = 0;
    killTimer(heartbeat);
    heartbeat = -1;
    qt_xdnd_current_screen = -1;
    qt_xdnd_dragging = false;

    if (dndCancelled)
        return QDrag::IgnoreAction;
    switch(global_accepted_action) {
    case QDropEvent::Copy:
        return QDrag::CopyAction;
    case QDropEvent::Move:
        return QDrag::MoveAction;
    case QDropEvent::Link:
        return QDrag::LinkAction;
    case QDropEvent::Private:
    default:
        return QDrag::CopyAction;
    }
    // qt_xdnd_source_object persists until we get an xdnd_finish message
}

void QDragManager::updatePixmap()
{
    if (qt_xdnd_deco) {
        QPixmap pm;
        QPoint pm_hot(default_pm_hotx,default_pm_hoty);
        if (object) {
            pm = object->d->pixmap;
            if (!pm.isNull())
                pm_hot = object->d->hotspot;
        }
        if (pm.isNull()) {
            if (!defaultPm)
                defaultPm = new QPixmap(default_pm);
            pm = *defaultPm;
        }
        qt_xdnd_deco->setPixmap(pm);
        qt_xdnd_deco->move(QCursor::pos()-pm_hot);
        qt_xdnd_deco->repaint();
            //if (willDrop) {
            qt_xdnd_deco->show();
            //} else {
            //    qt_xdnd_deco->hide();
            //}
    }
}

QVariant QDropData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    QByteArray data = qt_motifdnd_active
                      ? qt_motifdnd_obtain_data(mimetype.latin1())
                      : qt_xdnd_obtain_data(mimetype.latin1());
    if (type == QVariant::String)
        return QString::fromUtf8(data);
    return data;
}

bool QDropData::hasFormat(const QString &format) const
{
    return formats().contains(format);
}

QStringList QDropData::formats() const
{
    QStringList formats;
    if (qt_motifdnd_active) {
        int i = 0;
        const char *fmt;
        while ((fmt = qt_motifdnd_format(i))) {
            formats.append(QLatin1String(fmt));
            ++i;
        }
    } else {
        int i = 0;
        while ((qt_xdnd_types[i])) {
            formats.append(QLatin1String(qt_xdnd_atom_to_str(qt_xdnd_types[i])));
            ++i;
        }
    }
    return formats;
}

#endif // QT_NO_DRAGANDDROP

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
//#define QT_RASTER_PAINTENGINE

#include <private/qt_mac_p.h>

#include "qapplication.h"
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#ifdef QT_RASTER_PAINTENGINE
# include <private/qpaintengine_raster_p.h>
#endif
#include <private/qpaintengine_mac_p.h>
#include "qpainter.h"
#include "qstack.h"
#include "qstyle.h"
#include "qtextcodec.h"
#include "qtimer.h"
#include "qdebug.h"

#include <private/qabstractscrollarea_p.h>
#include <qabstractscrollarea.h>
#include <ApplicationServices/ApplicationServices.h>
#include <limits.h>

#include "qwidget_p.h"

#define XCOORD_MAX 32767
#define WRECT_MAX 8191


/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS
//#define DEBUG_WINDOW_CREATE
//#define DEBUG_WINDOW_STATE
//#define DEBUG_WIDGET_PAINT

/*****************************************************************************
  QWidget globals
 *****************************************************************************/
typedef QHash<Qt::WindowType, WindowGroupRef> StaysOnTopHash;
Q_GLOBAL_STATIC(StaysOnTopHash, qt_mac_stays_on_top)
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
const UInt32 kWidgetCreatorQt = 'cute';
enum {
    kWidgetPropertyQWidget = 'QWId' //QWidget *
};
static CFStringRef kObjectQWidget = CFSTR("com.trolltech.qt.widget");
Q_GUI_EXPORT QPoint qt_mac_posInWindow(const QWidget *w);

/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
extern bool qt_event_remove_activate(); //qapplication_mac.cpp
extern void qt_mac_event_release(QWidget *w); //qapplication_mac.cpp
extern void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_window_change(); //qapplication_mac.cpp
extern IconRef qt_mac_create_iconref(const QPixmap &); //qpixmap_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const QPoint &); //qcursor_mac.cpp
extern bool qt_nograb();
extern CGImageRef qt_mac_create_cgimage(const QPixmap &, bool); //qpixmap_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/

static QSize qt_initial_size(QWidget *w) {
    QSize s = w->sizeHint();
    Qt::Orientations exp;
    if(QLayout *layout = w->layout()) {
        if (layout->hasHeightForWidth())
            s.setHeight(layout->totalHeightForWidth(s.width()));
        exp = layout->expandingDirections();
    } else {
        if (w->sizePolicy().hasHeightForWidth())
            s.setHeight(w->heightForWidth(s.width()));
        exp = w->sizePolicy().expandingDirections();
    }
    if (exp & Qt::Horizontal)
        s.setWidth(qMax(s.width(), 200));
    if (exp & Qt::Vertical)
        s.setHeight(qMax(s.height(), 150));
    QRect screen = QApplication::desktop()->screenGeometry(w->pos());
    s.setWidth(qMin(s.width(), screen.width()*2/3));
    s.setHeight(qMin(s.height(), screen.height()*2/3));
    int left, top, right, bottom;
    w->getContentsMargins(&left, &top, &right, &bottom);
    s += QSize(left + right, top + bottom);
    return s;
}

QPoint qt_mac_posInWindow(const QWidget *w)
{
    QPoint ret = w->data->wrect.topLeft();
    while(w && !w->isWindow()) {
        ret += w->pos();
        w =  w->parentWidget();
    }
    return ret;
}

static void qt_mac_release_stays_on_top_group(Qt::WindowType type) //cleanup function
{
    Q_ASSERT(qt_mac_stays_on_top()->contains(type));
    WindowGroupRef group = qt_mac_stays_on_top()->value(type);
    ReleaseWindowGroup(group);
    if(GetWindowGroupRetainCount(group) == 1) { //only the global pointer exists
        qt_mac_stays_on_top()->remove(type);
        ReleaseWindowGroup(group);
    }
}

//find a QWidget from a WindowPtr
QWidget *qt_mac_find_window(WindowPtr window)
{
    if(!window)
        return 0;

    QWidget *ret;
    if(GetWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(ret), 0, &ret) == noErr)
        return ret;
    return 0;
}

inline static void qt_mac_set_fullscreen_mode(bool b)
{
#if 0
    if(b)
        SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
    else
        SetSystemUIMode(kUIModeNormal, 0);
#else
    extern bool qt_mac_app_fullscreen;
    qt_mac_app_fullscreen = b;
    if(b)
        HideMenuBar();
    else
        ShowMenuBar();
#endif
}

Q_GUI_EXPORT HIViewRef qt_mac_hiview_for(const QWidget *w)
{
    return (HIViewRef)w->data->winid;
}
Q_GUI_EXPORT HIViewRef qt_mac_hiview_for(WindowPtr w)
{
    HIViewRef ret = 0;
    OSStatus err = HIViewFindByID(HIViewGetRoot(w), kHIViewWindowContentID, &ret);
    if(err == errUnknownControl)
        ret = HIViewGetRoot(w);
    else if(err != noErr)
        qWarning("That cannot happen! %d [%ld]", __LINE__, long(err));
    return ret;
}

Q_GUI_EXPORT WindowPtr qt_mac_window_for(HIViewRef hiview)
{
    return HIViewGetWindow(hiview);
}
Q_GUI_EXPORT WindowPtr qt_mac_window_for(const QWidget *w)
{
    HIViewRef hiview = qt_mac_hiview_for(w);
    Q_ASSERT(hiview);
    WindowPtr window = qt_mac_window_for(hiview);
    if(!window && HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
        w->window()->d_func()->createWindow_sys();
        hiview = qt_mac_hiview_for(w);
        window = qt_mac_window_for(hiview);
    }
    return window;
}

/* Use this function instead of ReleaseWindowGroup, this will be sure to release the
   stays on top window group (created with qt_mac_get_stays_on_top_group below) */
static void qt_mac_release_window_group(WindowGroupRef group)
{
    bool just_release = true;
    for(StaysOnTopHash::iterator it = qt_mac_stays_on_top()->begin(); it != qt_mac_stays_on_top()->end(); ++it) {
        if(it.value() == group) {
            qt_mac_release_stays_on_top_group(it.key());
            just_release = false;
            break;
        }
    }
    if(just_release)
        ReleaseWindowGroup(group);
}
#define ReleaseWindowGroup(x) Are you sure you wanted to do that? (you wanted qt_mac_release_window_group)

/* We create one static stays on top window group so that all stays on top (aka popups) will
   fall into the same group and be able to be raise()'d with releation to one another (from
   within the same window group). */
static WindowGroupRef qt_mac_get_stays_on_top_group(Qt::WindowType type)
{
    WindowGroupRef group = 0;
    if(!qt_mac_stays_on_top()->contains(type)) {
        CreateWindowGroup(kWindowActivationScopeNone, &group);
        int group_level = kCGNormalWindowLevel;
        if(type == Qt::Dialog)
            group_level += 1;
        else if(type == Qt::Popup)
            group_level += 2;
        SetWindowGroupLevel(group, group_level);
        SetWindowGroupParent(group, GetWindowGroupOfClass(kAllWindowClasses));
        qt_mac_stays_on_top()->insert(type, group);
    } else {
        group = qt_mac_stays_on_top()->value(type);
    }
    RetainWindowGroup(group);
    return group;
}

void qt_mac_set_widget_is_opaque(QWidget *w, bool o)
{
    if (!w->testAttribute(Qt::WA_WState_Created))
        return;
    HIViewFeatures bits;
    HIViewRef hiview = qt_mac_hiview_for(w);
    HIViewGetFeatures(hiview, &bits);
    if ((bits & kHIViewIsOpaque) == o)
        return;
    if(o) {
        HIViewChangeFeatures(hiview, kHIViewIsOpaque, 0);
    } else {
        HIViewChangeFeatures(hiview, 0, kHIViewIsOpaque);
    }
    if (w->isVisible())
        HIViewReshapeStructure(qt_mac_hiview_for(w));
}

void qt_mac_update_ignore_mouseevents(QWidget *w)
{
    if (!w->testAttribute(Qt::WA_WState_Created))
        return;

    if(w->isWindow()) {
        if(w->testAttribute(Qt::WA_TransparentForMouseEvents))
            ChangeWindowAttributes(qt_mac_window_for(w), kWindowIgnoreClicksAttribute, 0);
        else
            ChangeWindowAttributes(qt_mac_window_for(w), 0, kWindowIgnoreClicksAttribute);
        ReshapeCustomWindow(qt_mac_window_for(w));
    } else {
#ifndef kHIViewFeatureIgnoresClicks
#define kHIViewFeatureIgnoresClicks kHIViewIgnoresClicks
#endif
        if(w->testAttribute(Qt::WA_TransparentForMouseEvents))
            HIViewChangeFeatures(qt_mac_hiview_for(w), kHIViewFeatureIgnoresClicks, 0);
        else
            HIViewChangeFeatures(qt_mac_hiview_for(w), 0, kHIViewFeatureIgnoresClicks);
        HIViewReshapeStructure(qt_mac_hiview_for(w));
    }
}

void qt_mac_update_metal_style(QWidget *w)
{
    if (!w->testAttribute(Qt::WA_WState_Created))
        return;

    if(w->isWindow()) {
        if(w->testAttribute(Qt::WA_MacMetalStyle))
            ChangeWindowAttributes(qt_mac_window_for(w), kWindowMetalAttribute, 0);
        else
            ChangeWindowAttributes(qt_mac_window_for(w), 0, kWindowMetalAttribute);
    }
}

void qt_mac_update_opaque_sizegrip(QWidget *window)
{
    if (!window->testAttribute(Qt::WA_WState_Created) || !window->isWindow())
        return;

    HIViewRef growBox;
    HIViewFindByID(HIViewGetRoot(qt_mac_window_for(window)), kHIViewWindowGrowBoxID, &growBox);
    if (!growBox)
        return;
    HIGrowBoxViewSetTransparent(growBox, !window->testAttribute(Qt::WA_MacOpaqueSizeGrip));
}

static OSStatus qt_mac_create_window(WindowClass wclass, WindowAttributes wattr,
                                     Rect *geo, WindowPtr *w)
{
    OSStatus ret;
    if(geo->right <= geo->left)
        geo->right = geo->left + 1;
    if(geo->bottom <= geo->top)
        geo->bottom = geo->top + 1;
    Rect null_rect; SetRect(&null_rect, 0, 0, 1, 1);
    ret = CreateNewWindow(wclass, wattr, &null_rect, w);
    if(ret == noErr) {
        ret = SetWindowBounds(*w, kWindowContentRgn, geo);
        if(ret != noErr)
            qWarning("QWidget: Internal error (%s:%d)", __FILE__, __LINE__);
    }
    return ret;
}

// window events
static EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowClose },
    { kEventClassWindow, kEventWindowExpanded },
    { kEventClassWindow, kEventWindowCollapsed },
    { kEventClassWindow, kEventWindowToolbarSwitchMode },
    { kEventClassWindow, kEventWindowProxyBeginDrag },
    { kEventClassWindow, kEventWindowProxyEndDrag },
    { kEventClassWindow, kEventWindowResizeStarted },
    { kEventClassWindow, kEventWindowResizeCompleted },
    { kEventClassWindow, kEventWindowDragStarted },
    { kEventClassWindow, kEventWindowDragCompleted },
    { kEventClassWindow, kEventWindowBoundsChanging },
    { kEventClassWindow, kEventWindowGetRegion },

    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp }
};
static EventHandlerUPP mac_win_eventUPP = 0;
static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = 0;
}
static const EventHandlerUPP make_win_eventUPP()
{
    if(mac_win_eventUPP)
        return mac_win_eventUPP;
    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_window_event);
}
OSStatus QWidgetPrivate::qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassWindow: {
        WindowRef wid = 0;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                          sizeof(WindowRef), 0, &wid);
        QWidget *widget = qt_mac_find_window(wid);
        if(!widget) {
            handled_event = false;
        } else if(ekind == kEventWindowClose) {
            widget->d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
        } else if(ekind == kEventWindowExpanded) {
            widget->setWindowState((widget->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            QShowEvent qse;
            QApplication::sendSpontaneousEvent(widget, &qse);
        } else if(ekind == kEventWindowZoomed) {
            UInt32 windowPart;
            GetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, 0,
                                  sizeof(windowPart), 0, &windowPart);
            if(windowPart == inZoomIn)
                widget->setWindowState(widget->windowState() & ~Qt::WindowMaximized);
            else if(windowPart == inZoomOut)
                widget->setWindowState(widget->windowState() | Qt::WindowMaximized);
        } else if(ekind == kEventWindowCollapsed) {
            widget->setWindowState(widget->windowState() | Qt::WindowMinimized);
            //we send a hide to be like X11/Windows
            QEvent e(QEvent::Hide);
            QApplication::sendSpontaneousEvent(widget, &e);
        } else if(ekind == kEventWindowToolbarSwitchMode) {
            QToolBarChangeEvent ev(!(GetCurrentKeyModifiers() & cmdKey));
            QApplication::sendSpontaneousEvent(widget, &ev);
        } else if(ekind == kEventWindowGetRegion) {
            WindowRef window;
            GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                              sizeof(window), 0, &window);
            CallNextEventHandler(er, event);
            WindowRegionCode wcode;
            GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, 0,
                              sizeof(wcode), 0, &wcode);
            RgnHandle rgn;
            GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, 0,
                              sizeof(rgn), 0, &rgn);
            if(QWidgetPrivate::qt_widget_rgn(qt_mac_find_window(window), wcode, rgn, false))
                SetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, sizeof(rgn), &rgn);
        } else if(ekind == kEventWindowProxyBeginDrag) {
            QIconDragEvent e;
            QApplication::sendSpontaneousEvent(widget, &e);
        } else if(ekind == kEventWindowResizeStarted || ekind == kEventWindowDragStarted) {
            QMacBlockingFunction::addRef();
        } else if(ekind == kEventWindowResizeCompleted || ekind == kEventWindowDragCompleted) {
            QMacBlockingFunction::subRef();
        } else if(ekind == kEventWindowBoundsChanging) {
            UInt32 flags = 0;
            GetEventParameter(event, kEventParamAttributes, typeUInt32, 0,
                                  sizeof(flags), 0, &flags);
            Rect nr;
            GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, 0,
                                  sizeof(nr), 0, &nr);

            QRect newRect(nr.left, nr.top, nr.right - nr.left, nr.bottom - nr.top);

            QTLWExtra * const tlwExtra = widget->d_func()->maybeTopData();
            if (tlwExtra && tlwExtra->isSetGeometry == 1) {
                widget->d_func()->setGeometry_sys_helper(newRect.left(), newRect.top(), newRect.width(), newRect.height(), tlwExtra->isMove);
            } else {
                //implicitly removes the maximized bit
                if((widget->windowState() & Qt::WindowMaximized) &&
                   IsWindowInStandardState((WindowPtr)widget->handle(), 0, 0))
                    widget->setWindowState(widget->windowState() & ~Qt::WindowMaximized);

                handled_event = false;
                const QRect oldRect = widget->data->crect;
                if((flags & kWindowBoundsChangeOriginChanged)) {
                    if(nr.left != oldRect.x() || nr.top != oldRect.y()) {
                        widget->data->crect.moveTo(nr.left, nr.top);
                        QMoveEvent qme(widget->data->crect.topLeft(), oldRect.topLeft());
                        QApplication::sendSpontaneousEvent(widget, &qme);
                    }
                }
                if((flags & kWindowBoundsChangeSizeChanged)) {
                    if (widget->isWindow()
                            && widget->layout() && widget->layout()->hasHeightForWidth()) {
                        QRect rect = widget->geometry();
                        QSize newSize = QLayout::closestAcceptableSize(widget, newRect.size());
                        int dh = newSize.height() - newRect.height();
                        int dw = newSize.width() - newRect.width();
                        if (dw != 0 || dh != 0) {
                            handled_event = true;  // We want to change the bounds, so we handle the event

                            // set the rect, so we can also do the resize down below (yes, we need to resize).
                            newRect.setBottom(newRect.bottom() + dh);
                            newRect.setRight(newRect.right() + dw);

                            nr.left = newRect.x();
                            nr.top = newRect.y();
                            nr.right = nr.left + newRect.width();
                            nr.bottom = nr.top + newRect.height();
                            SetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, sizeof(Rect), &nr);
                        }
                    }

                    if (oldRect.width() != newRect.width() || oldRect.height() != newRect.height()) {
                        widget->data->crect.setSize(newRect.size());
                        HIRect bounds = CGRectMake(0, 0, newRect.width(), newRect.height());
                        HIViewSetFrame(qt_mac_hiview_for(widget), &bounds);
                        QResizeEvent qre(newRect.size(), oldRect.size());
                        QApplication::sendSpontaneousEvent(widget, &qre);
                        qt_event_request_window_change();
                    }
                }
            }
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassMouse: {
#if 0
        return SendEventToApplication(event);
#endif

        bool send_to_app = false;
        {
            WindowPartCode wpc;
            if (GetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, 0,
                                  sizeof(wpc), 0, &wpc) == noErr && wpc != inContent)
                send_to_app = true;
        }
        if(!send_to_app) {
            WindowRef window;
            if(GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                 sizeof(window), 0, &window) == noErr) {
                HIViewRef hiview;
                if(HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &hiview) == noErr) {
                    if(QWidget *w = QWidget::find((WId)hiview)) {
#if 0
                        send_to_app = !w->isActiveWindow();
#else
                        Q_UNUSED(w);
                        send_to_app = true;
#endif
                    }
                }
            }
        }
        if(send_to_app)
            return SendEventToApplication(event);
        handled_event = false;
        break; }
    default:
        handled_event = false;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

// widget events
static HIObjectClassRef widget_class = 0;
static EventTypeSpec widget_events[] = {
    { kEventClassHIObject, kEventHIObjectConstruct },
    { kEventClassHIObject, kEventHIObjectDestruct },

    { kEventClassControl, kEventControlDraw },
    { kEventClassControl, kEventControlInitialize },
    { kEventClassControl, kEventControlGetPartRegion },
    { kEventClassControl, kEventControlGetClickActivation },
    { kEventClassControl, kEventControlSetFocusPart },
    { kEventClassControl, kEventControlDragEnter },
    { kEventClassControl, kEventControlDragWithin },
    { kEventClassControl, kEventControlDragLeave },
    { kEventClassControl, kEventControlDragReceive },
    { kEventClassControl, kEventControlOwningWindowChanged },

    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseMoved },
    { kEventClassMouse, kEventMouseDragged }
};
static EventHandlerUPP mac_widget_eventUPP = 0;
static void cleanup_widget_eventUPP()
{
    DisposeEventHandlerUPP(mac_widget_eventUPP);
    mac_widget_eventUPP = 0;
}
static const EventHandlerUPP make_widget_eventUPP()
{
    if(mac_widget_eventUPP)
        return mac_widget_eventUPP;
    qAddPostRoutine(cleanup_widget_eventUPP);
    return mac_widget_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_widget_event);
}
OSStatus QWidgetPrivate::qt_widget_event(EventHandlerCallRef er, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassHIObject: {
        HIViewRef view = 0;
        GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef,
                          0, sizeof(view), 0, &view);
        if(ekind == kEventHIObjectConstruct) {
            if(view) {
                HIViewChangeFeatures(view, kHIViewAllowsSubviews, 0);
                SetEventParameter(event, kEventParamHIObjectInstance,
                                  typeVoidPtr, sizeof(view), &view);
            }
        } else if(ekind == kEventHIObjectDestruct) {
            //nothing to really do.. or is there?
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassControl: {
        QWidget *widget = 0;
        HIViewRef hiview = 0;
        if(GetEventParameter(event, kEventParamDirectObject, typeControlRef,
                             0, sizeof(hiview), 0, &hiview) == noErr)
            widget = QWidget::find((WId)hiview);
        if(ekind == kEventControlDraw) {
            if(widget) {
                QMacWindowChangeEvent::exec(true);

                //requested rgn
                widget->d_func()->clp_serial++;
                RgnHandle rgn;
                GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, 0, sizeof(rgn), 0, &rgn);
                QRegion qrgn(qt_mac_convert_mac_region(rgn));

                //get widget region
                RgnHandle widgetRgn = qt_mac_get_rgn();
                GetControlRegion(hiview, kControlStructureMetaPart, widgetRgn);
                widget->d_func()->clp = qt_mac_convert_mac_region(widgetRgn);
                qt_mac_dispose_rgn(widgetRgn);
                if(!widget->isWindow()) {
                    QPoint pt(qt_mac_posInWindow(widget));
                    widget->d_func()->clp.translate(pt.x(), pt.y());
                }

                //update handles
                GrafPtr qd = 0;
                {
                    if(GetEventParameter(event, kEventParamGrafPort, typeGrafPtr, 0, sizeof(qd), 0, &qd) != noErr) {
#ifndef QT_MAC_NO_QUICKDRAW
                        GDHandle dev = 0;
                        GetGWorld(&qd, &dev); //just use the global port..
#endif
                    }
                }
                bool end_cg_context = false;
                CGContextRef cg = 0;
                if(GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, 0, sizeof(cg), 0, &cg) != noErr && qd) {
                    end_cg_context = true;
                    QDBeginCGContext(qd, &cg);
                }
                widget->d_func()->hd = cg;
                widget->d_func()->qd_hd = qd;
                CGContextSaveGState(cg);

#ifdef DEBUG_WIDGET_PAINT
                const bool doDebug = true;
                if(doDebug)  {
                    qDebug("asked to draw %p[%p] [%s::%s] %p[%p] [%d] [%dx%d]", widget, hiview, widget->metaObject()->className(),
                           widget->objectName().local8Bit().data(), widget->parentWidget(),
                           (HIViewRef)(widget->parentWidget() ? qt_mac_hiview_for(widget->parentWidget()) : (HIViewRef)0),
                           HIViewIsCompositingEnabled(hiview), qt_mac_posInWindow(widget).x(), qt_mac_posInWindow(widget).y());
#if 0
                    QVector<QRect> region_rects = qrgn.rects();
                    qDebug("Region! %d", region_rects.count());
                    for(int i = 0; i < region_rects.count(); i++)
                        qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                               region_rects[i].width(), region_rects[i].height());
                    region_rects = widget->d_func()->clp.rects();
                    qDebug("Widget Region! %d", region_rects.count());
                    for(int i = 0; i < region_rects.count(); i++)
                        qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                               region_rects[i].width(), region_rects[i].height());
#endif
                }
#endif
                if (widget->isVisible() && widget->updatesEnabled()) { //process the actual paint event.
                    if(widget->testAttribute(Qt::WA_WState_InPaintEvent))
                        qWarning("QWidget::repaint: Recursive repaint detected");

                    QPoint redirectionOffset(0, 0);
                    // handle the first paintable point since Mac doesn't.
                    if(QWidget *tl = widget->window()) {
                        if(tl->d_func()->extra && !tl->d_func()->extra->mask.isEmpty())
                            redirectionOffset += tl->d_func()->extra->mask.boundingRect().topLeft();
                    }

                    //setup the context
                    widget->setAttribute(Qt::WA_WState_InPaintEvent);
                    QPaintEngine *engine = widget->paintEngine();
                    if (engine)
                        engine->setSystemClip(qrgn);

                    //handle the erase
                    if (engine && !widget->testAttribute(Qt::WA_NoSystemBackground)
                        && (widget->isWindow() || widget->autoFillBackground()) || widget->testAttribute(Qt::WA_TintedBackground)) {
                        QRect rr = qrgn.boundingRect();
#ifdef DEBUG_WIDGET_PAINT
                        if(doDebug)
                            qDebug(" Handling erase for [%s::%s]", widget->metaObject()->className(),
                                   widget->objectName().local8Bit().data());
#endif
                        if (!redirectionOffset.isNull()) {
                            QPainter::setRedirected(widget, widget, redirectionOffset);
                            rr.setWidth(rr.width()+redirectionOffset.x());
                            rr.setHeight(rr.height()+redirectionOffset.y());
                        }
                        bool was_unclipped = widget->testAttribute(Qt::WA_PaintUnclipped);
                        widget->setAttribute(Qt::WA_PaintUnclipped, false);
                        QPainter p(widget);
                        if(was_unclipped)
                            widget->setAttribute(Qt::WA_PaintUnclipped);
                        p.setClipRegion(qrgn.translated(redirectionOffset));

                        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(widget->parent());
                        if (scrollArea && scrollArea->viewport() == widget) {
                            QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(static_cast<QWidget *>(scrollArea)->d_ptr);
                            const QPoint offset = priv->contentsOffset();
                            p.translate(-offset);
                            rr.translate(offset);
                        }

                        widget->d_func()->paintBackground(&p, rr, widget->isWindow());
                        if (widget->testAttribute(Qt::WA_TintedBackground)) {
                            QColor tint = widget->palette().window().color();
                            tint.setAlphaF(.6);
                            p.fillRect(rr, tint);
                        }
                        p.end();
                        if (!redirectionOffset.isNull())
                            QPainter::restoreRedirected(widget);
                    }

                    if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                        CallNextEventHandler(er, event);

                    //send the paint
                    redirectionOffset += widget->data->wrect.topLeft(); // Map from system to qt coordinates
                    if (!redirectionOffset.isNull())
                        QPainter::setRedirected(widget, widget, redirectionOffset);
                    qrgn.translate(redirectionOffset);
                    QPaintEvent e(qrgn);
#ifdef QT3_SUPPORT
                    e.setErased(true);
#endif
                    QApplication::sendSpontaneousEvent(widget, &e);
                    if (!redirectionOffset.isNull())
                        QPainter::restoreRedirected(widget);
#ifdef QT_RASTER_PAINTENGINE
                    if(engine->type() == QPaintEngine::Raster)
                        static_cast<QRasterPaintEngine*>(engine)->flush(widget,
                                                                        qrgn.boundingRect().topLeft());
#endif

                    //cleanup
                    if (engine)
                        engine->setSystemClip(QRegion());

                    widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
                    if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
                        qWarning("QWidget: It is dangerous to leave painters active on a widget outside of the PaintEvent");
                }

                widget->d_func()->clp_serial++;
                widget->d_func()->clp = QRegion();
                widget->d_func()->hd = 0;
                widget->d_func()->qd_hd = 0;
                CGContextRestoreGState(cg);
                if(end_cg_context)
                    QDEndCGContext(qd, &cg);
            } else if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
                CallNextEventHandler(er, event);
            }
        } else if(ekind == kEventControlInitialize) {
            if(HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
                UInt32 features = kControlSupportsDragAndDrop | kControlSupportsClickActivation | kControlSupportsFocus;
                SetEventParameter(event, kEventParamControlFeatures, typeUInt32, sizeof(features), &features);
            } else {
                handled_event = false;
            }
        } else if(ekind == kEventControlSetFocusPart) {
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                CallNextEventHandler(er, event);
        } else if(ekind == kEventControlGetClickActivation) {
            ClickActivationResult clickT = kActivateAndIgnoreClick;
            SetEventParameter(event, kEventParamClickActivation, typeClickActivationResult,
                              sizeof(clickT), &clickT);
        } else if(ekind == kEventControlGetPartRegion) {
            handled_event = false;
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget) && CallNextEventHandler(er, event) == noErr) {
                handled_event = true;
                break;
            }
            if(widget && !widget->isWindow()) {
                ControlPartCode part;
                GetEventParameter(event, kEventParamControlPart, typeControlPartCode, 0,
                                  sizeof(part), 0, &part);
                if(part == kControlClickableMetaPart && widget->testAttribute(Qt::WA_TransparentForMouseEvents)) {
                    RgnHandle rgn;
                    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                      sizeof(rgn), 0, &rgn);
                    SetEmptyRgn(rgn);
                    handled_event = true;
                } else if(part == kControlStructureMetaPart || part == kControlClickableMetaPart) {
                    RgnHandle rgn;
                    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                      sizeof(rgn), 0, &rgn);
                    SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
                    if(QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false))
                        handled_event = true;
                } else if(part == kControlOpaqueMetaPart) {
                    if(widget->d_func()->isOpaque()) {
                        RgnHandle rgn;
                        GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                          sizeof(RgnHandle), 0, &rgn);
                        SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
                        QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false);
                        SetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle,
                                sizeof(RgnHandle), &rgn);
                        handled_event = true;
                    }
                }
            }
        } else if(ekind == kEventControlOwningWindowChanged) {
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                CallNextEventHandler(er, event);
            if(widget && qt_mac_window_for(hiview)) {
                WindowRef foo = 0;
                GetEventParameter(event, kEventParamControlCurrentOwningWindow, typeWindowRef, 0,
                                  sizeof(foo), 0, &foo);
                widget->d_func()->initWindowPtr();
            }
        } else if(ekind == kEventControlDragEnter || ekind == kEventControlDragWithin ||
                  ekind == kEventControlDragLeave || ekind == kEventControlDragReceive) {
            handled_event = false;
            bool drag_allowed = false;
            if(widget) {
                //these are really handled in qdnd_mac.cpp just to modularize the code a little..
                DragRef drag;
                GetEventParameter(event, kEventParamDragRef, typeDragRef, 0, sizeof(drag), 0, &drag);
                if(widget->d_func()->qt_mac_dnd_event(ekind, drag)) {
                    drag_allowed = true;
                    handled_event = true;
                }
            }
            if(ekind == kEventControlDragEnter) {
                const Boolean wouldAccept = drag_allowed ? true : false;
                SetEventParameter(event, kEventParamControlWouldAcceptDrop, typeBoolean,
                        sizeof(wouldAccept), &wouldAccept);
            }
        }
        break; }
    case kEventClassMouse: {
        bool send_to_app = false;
        extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
        if(qt_button_down)
            send_to_app = true;
        if(send_to_app) {
            OSStatus err = SendEventToApplication(event);
            if(err != noErr)
                handled_event = false;
        } else {
            CallNextEventHandler(er, event);
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}
static HIViewRef qt_mac_create_widget(HIViewRef parent)
{
    if(!widget_class) {
        OSStatus err = HIObjectRegisterSubclass(kObjectQWidget, kHIViewClassID, 0, make_widget_eventUPP(),
                                                GetEventTypeCount(widget_events), widget_events,
                                                0, &widget_class);
        if (err && err != hiObjectClassExistsErr)
            qWarning("QWidget: Internal error (%d)", __LINE__);
    }
    HIViewRef ret = 0;
    if(HIObjectCreate(kObjectQWidget, 0, (HIObjectRef*)&ret) != noErr)
        qWarning("QWidget: Internal error (%d)", __LINE__);
    if(ret && parent)
        HIViewAddSubview(parent, ret);
    //HIViewSetVisible(ret, false);
    return ret;
}

static QSize qt_mac_desktopSize()
{
    int w = 0, h = 0;
    CGDisplayCount cg_count;
    CGGetActiveDisplayList(0, 0, &cg_count);
    QVector<CGDirectDisplayID> displays(cg_count);
    CGGetActiveDisplayList(cg_count, displays.data(), &cg_count);
    Q_ASSERT(cg_count == (CGDisplayCount)displays.size());
    for(int i = 0; i < (int)cg_count; ++i) {
        CGRect r = CGDisplayBounds(displays.at(i));
        w = qMax<int>(w, qRound(r.origin.x + r.size.width));
        h = qMax<int>(h, qRound(r.origin.y + r.size.height));
    }
    return QSize(w, h);
}

bool qt_mac_can_clickThrough(const QWidget *w)
{
    static int qt_mac_carbon_clickthrough = -1;
    if (qt_mac_carbon_clickthrough < 0)
        qt_mac_carbon_clickthrough = !qgetenv("QT_MAC_NO_COCOA_CLICKTHROUGH").isEmpty();
    bool ret = !qt_mac_carbon_clickthrough;
    for ( ; w; w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_MacNoClickThrough)) {
            ret = false;
            break;
        }
    }
    return ret;
}

bool qt_mac_is_macdrawer(const QWidget *w)
{
    return (w && w->parentWidget() && w->windowType() == Qt::Drawer);
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::DockWidgetArea where) //users of Qt/Mac can use this..
{
    if(!qt_mac_is_macdrawer(w))
        return false;
    OptionBits edge;
    if(where & Qt::LeftDockWidgetArea)
        edge = kWindowEdgeLeft;
    else if(where & Qt::RightDockWidgetArea)
        edge = kWindowEdgeRight;
    else if(where & Qt::TopDockWidgetArea)
        edge = kWindowEdgeTop;
    else if(where & Qt::BottomDockWidgetArea)
        edge = kWindowEdgeBottom;
    else
        return false;
    WindowPtr window = qt_mac_window_for(w);
    if(edge == GetDrawerPreferredEdge(window)) //no-op
        return false;
    //do it
    SetDrawerPreferredEdge(window, edge);
    if(w->isVisible()) {
        CloseDrawer(window, false);
        OpenDrawer(window, edge, true);
    }
    return true;
}

void QWidgetPrivate::toggleDrawers(bool visible)
{
    for (int i = 0; i < children.size(); ++i) {
        register QObject *object = children.at(i);
        if (!object->isWidgetType())
            continue;
        QWidget *widget = static_cast<QWidget*>(object);
        if(qt_mac_is_macdrawer(widget)) {
            if(visible) {
                if (!widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
                    widget->show();
            } else {
                widget->hide();
                widget->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
            }
        }
    }
}

bool qt_mac_is_macsheet(const QWidget *w)
{
    return (w && w->windowType() == Qt::Sheet
       && w->parentWidget() && w->parentWidget()->window()->windowType() != Qt::Desktop
       && w->parentWidget()->window()->isVisible());
}



/*****************************************************************************
  QWidgetPrivate member functions
 *****************************************************************************/
bool QWidgetPrivate::qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isWindow())
        return false;

    QTLWExtra *topData = w->d_func()->topData();
    QWExtra *extraData = w->d_func()->extraData();
    topData->resizer += up;
    {
        WindowClass wclass;
        GetWindowClass(qt_mac_window_for(w), &wclass);
        if(!(GetAvailableWindowAttributes(wclass) & kWindowResizableAttribute))
            return true;
    }
    bool remove_grip = (topData->resizer ||
                        (extraData->maxw && extraData->maxh &&
                         extraData->maxw == extraData->minw && extraData->maxh == extraData->minh));

    WindowAttributes attr;
    GetWindowAttributes(qt_mac_window_for(w), &attr);
    if(remove_grip) {
        if(attr & kWindowResizableAttribute) {
            ChangeWindowAttributes(qt_mac_window_for(w), kWindowNoAttributes,
                                   kWindowResizableAttribute);
            ReshapeCustomWindow(qt_mac_window_for(w));
        }
    } else if(!(attr & kWindowResizableAttribute)) {
        ChangeWindowAttributes(qt_mac_window_for(w), kWindowResizableAttribute,
                               kWindowNoAttributes);
        ReshapeCustomWindow(qt_mac_window_for(w));
    }
    return true;
}

static WindowPtr qt_root_win = 0;
void QWidgetPrivate::qt_clean_root_win()
{
    if(!qt_root_win)
        return;
    ReleaseWindow(qt_root_win);
    qt_root_win = 0;
}

bool QWidgetPrivate::qt_create_root_win() {
    if(qt_root_win)
        return false;
    Rect r;
    const QSize desktopSize = qt_mac_desktopSize();
    SetRect(&r, 0, 0, desktopSize.width(), desktopSize.height());
    qt_mac_create_window(kOverlayWindowClass, kWindowNoAttributes, &r, &qt_root_win);
    if(!qt_root_win)
        return false;
    qAddPostRoutine(qt_clean_root_win);
    return true;
}

bool QWidgetPrivate::qt_recreate_root_win() {
    if(!qt_root_win) //sanity check
        return false;
    //store old
    WindowPtr old_root_win = qt_root_win;
    HIViewRef old_root_hiview = HIViewGetRoot(qt_root_win);
    //recreate
    qt_root_win = 0;
    qt_create_root_win();
    //cleanup old window
    old_root_hiview = 0;
    ReleaseWindow(old_root_win);
    return true;
}

bool QWidgetPrivate::qt_widget_rgn(QWidget *widget, short wcode, RgnHandle rgn, bool force = false)
{
    bool ret = false;
    switch(wcode) {
    case kWindowStructureRgn: {
        if(widget) {
            if(widget->d_func()->extra && !widget->d_func()->extra->mask.isEmpty()) {
                QRegion rin = qt_mac_convert_mac_region(rgn);
                if(!rin.isEmpty()) {
                    QPoint rin_tl = rin.boundingRect().topLeft(); //in offset
                    rin.translate(-rin_tl.x(), -rin_tl.y()); //bring into same space as below
                    QRegion mask = widget->d_func()->extra->mask;
                    if(widget->isWindow() && !(widget->windowFlags() & Qt::FramelessWindowHint)) {
                        QRegion title;
                        {
                            RgnHandle rgn = qt_mac_get_rgn();
                            GetWindowRegion(qt_mac_window_for(widget), kWindowTitleBarRgn, rgn);
                            title = qt_mac_convert_mac_region(rgn);
                            qt_mac_dispose_rgn(rgn);
                        }
                        QRect br = title.boundingRect();
                        mask.translate(0, br.height()); //put the mask 'under' the titlebar..
                        title.translate(-br.x(), -br.y());
                        mask += title;
                    }

                    QRegion cr = rin & mask;
                    cr.translate(rin_tl.x(), rin_tl.y()); //translate back to incoming space
                    CopyRgn(cr.handle(true), rgn);
                }
                ret = true;
            } else if(force) {
                QRegion cr(widget->geometry());
                CopyRgn(cr.handle(true), rgn);
                ret = true;
            }
        }
        break; }
    default: break;
    }
    //qDebug() << widget << ret << wcode << qt_mac_convert_mac_region(rgn);
    return ret;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidgetPrivate::determineWindowClass()
{
    Q_Q(QWidget);

    const Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    const bool popup = (type == Qt::Popup);
    const bool tool = (type == Qt::Tool || type == Qt::SplashScreen);
    if (type == Qt::ToolTip)
        flags |= Qt::FramelessWindowHint;

    WindowClass wclass = kSheetWindowClass;
    if(qt_mac_is_macdrawer(q))
        wclass = kDrawerWindowClass;
    else if(popup || type == Qt::SplashScreen)
        wclass = kModalWindowClass;
    else if(q->testAttribute(Qt::WA_ShowModal))
        wclass = kMovableModalWindowClass;
    else if(type == Qt::ToolTip)
        wclass = kHelpWindowClass;
    else if(tool)
        wclass = kFloatingWindowClass;
    else
        wclass = kDocumentWindowClass;

    WindowGroupRef grp = 0;
    WindowAttributes wattr = (kWindowCompositingAttribute | kWindowStandardHandlerAttribute);
    if(qt_mac_is_macsheet(q)) {
        //grp = GetWindowGroupOfClass(kMovableModalWindowClass);
        wclass = kSheetWindowClass;
    } else {
        grp = GetWindowGroupOfClass(wclass);
        // Shift things around a bit to get the correct window class based on the presence
        // (or lack) of the border.
        if(flags & Qt::FramelessWindowHint) {
            if(wclass == kDocumentWindowClass) {
                wclass = kSimpleWindowClass;
            } else if(wclass == kFloatingWindowClass) {
                wclass = kToolbarWindowClass;
            } else if (wclass  == kMovableModalWindowClass) {
                wclass  = kModalWindowClass;
            }
        } else {
            if(wclass != kModalWindowClass)
                wattr |= kWindowResizableAttribute;
        }
        // Only add extra decorations (well, buttons) for widgets that can have them
        // and have an actual border we can put them on.
        if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass
                && wclass != kSheetWindowClass && wclass != kPlainWindowClass
                && !(flags & Qt::FramelessWindowHint) && wclass != kDrawerWindowClass
                && wclass != kHelpWindowClass) {
            if(flags & Qt::WindowMaximizeButtonHint)
                wattr |= kWindowFullZoomAttribute;
            if(flags & Qt::WindowMinimizeButtonHint)
                wattr |= kWindowCollapseBoxAttribute;
            if(flags & Qt::WindowSystemMenuHint)
                wattr |= kWindowCloseBoxAttribute;
        }
    }
    if((popup || (tool && type != Qt::SplashScreen)) && !q->isModal())
        wattr |= kWindowHideOnSuspendAttribute;
    wattr |= kWindowLiveResizeAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
    struct {
        UInt32 tag;
        const char *name;
    } known_attribs[] = {
        ADD_DEBUG_WINDOW_NAME(kWindowCompositingAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowMetalAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowNoUpdatesAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowOpaqueForEventsAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowLiveResizeAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowCloseBoxAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
        { 0, 0 }
    }, known_classes[] = {
        ADD_DEBUG_WINDOW_NAME(kHelpWindowClass),
        ADD_DEBUG_WINDOW_NAME(kPlainWindowClass),
        ADD_DEBUG_WINDOW_NAME(kDrawerWindowClass),
        ADD_DEBUG_WINDOW_NAME(kUtilityWindowClass),
        ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
        ADD_DEBUG_WINDOW_NAME(kSheetWindowClass),
        ADD_DEBUG_WINDOW_NAME(kFloatingWindowClass),
        ADD_DEBUG_WINDOW_NAME(kDocumentWindowClass),
        ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
        ADD_DEBUG_WINDOW_NAME(kMovableModalWindowClass),
        ADD_DEBUG_WINDOW_NAME(kModalWindowClass),
        { 0, 0 }
    };
    qDebug("Qt: internal: ************* Creating new window %p (%s::%s)", q, q->metaObject()->className(),
            q->objectName().toLocal8Bit().constData());
    bool found_class = false;
    for(int i = 0; known_classes[i].name; i++) {
        if(wclass == known_classes[i].tag) {
            found_class = true;
            qDebug("Qt: internal: ** Class: %s", known_classes[i].name);
            break;
        }
    }
    if(!found_class)
        qDebug("Qt: internal: !! Class: Unknown! (%d)", (int)wclass);
    if(wattr) {
        WindowAttributes tmp_wattr = wattr;
        qDebug("Qt: internal: ** Attributes:");
        for(int i = 0; tmp_wattr && known_attribs[i].name; i++) {
            if((tmp_wattr & known_attribs[i].tag) == known_attribs[i].tag) {
                tmp_wattr ^= known_attribs[i].tag;
                qDebug("Qt: internal: * %s %s", known_attribs[i].name,
                        (GetAvailableWindowAttributes(wclass) & known_attribs[i].tag) ? "" : "(*)");
            }
        }
        if(tmp_wattr)
            qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)tmp_wattr);
    }
#endif

    /* Just to be extra careful we will change to the kUtilityWindowClass if the
       requested attributes cannot be used */
    if((GetAvailableWindowAttributes(wclass) & wattr) != wattr) {
        WindowClass tmp_class = wclass;
        if(wclass == kToolbarWindowClass || wclass == kUtilityWindowClass)
            wclass = kFloatingWindowClass;
        if(tmp_class != wclass) {
            if(!grp)
                grp = GetWindowGroupOfClass(wclass);
            wclass = tmp_class;
        }
    }
    topData()->wclass = wclass;
    topData()->wattr = wattr;
}

void QWidgetPrivate::initWindowPtr()
{
    Q_Q(QWidget);
    WindowPtr windowRef = qt_mac_window_for(qt_mac_hiview_for(q)); //do not create!
    if(!windowRef)
        return;
    QWidget *window = q->window(), *oldWindow = 0;
    if(GetWindowProperty(windowRef, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(oldWindow), 0, &oldWindow) == noErr) {
        Q_ASSERT(window == oldWindow);
        return;
    }

    if(SetWindowProperty(windowRef, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(window), &window) != noErr)
        qWarning("Qt:Internal error (%s:%d)", __FILE__, __LINE__); //no real way to recover
    if(!q->windowType() != Qt::Desktop) { //setup an event callback handler on the window
        InstallWindowEventHandler(windowRef, make_win_eventUPP(), GetEventTypeCount(window_events),
                window_events, static_cast<void *>(qApp), &window_event);
    }
}

void QWidgetPrivate::createWindow_sys()
{
    Q_Q(QWidget);

    const Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    const bool desktop = (type == Qt::Desktop);
    const bool dialog = (type == Qt::Dialog
                         || type == Qt::Sheet
                         || type == Qt::Drawer
                         || (flags & Qt::MSWindowsFixedSizeDialogHint));
    QTLWExtra *topExtra = topData();
    quint32 wattr = topExtra->wattr;

    if(parentWidget && (parentWidget->window()->windowFlags() & Qt::WindowStaysOnTopHint)) // If our parent has Qt::WStyle_StaysOnTop, so must we
        flags |= Qt::WindowStaysOnTopHint;
    if (0 && q->testAttribute(Qt::WA_ShowModal)  // ### Look at this, again!
            && !(flags & Qt::CustomizeWindowHint)
        && !(desktop || type == Qt::Popup)) {
        flags &= ~(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    }

    Rect r;
    SetRect(&r, data.crect.left(), data.crect.top(), data.crect.right() + 1, data.crect.bottom() + 1);
    data.fstrut_dirty = true;
    WindowRef windowRef = 0;
    if (OSStatus ret = qt_mac_create_window(topExtra->wclass, wattr, &r, &windowRef))
        qWarning("QWidget: Internal error: %s:%d: If you reach this error please contact Trolltech and include the\n"
                "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, long(ret));
    if (!desktop)
        SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
    HIWindowChangeFeatures(windowRef, kWindowCanCollapse, 0);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        if (wattr & kWindowHideOnSuspendAttribute)
            HIWindowChangeAvailability(windowRef, kHIWindowExposeHidden, 0);
        else
            HIWindowChangeAvailability(windowRef, 0, kHIWindowExposeHidden);
    }
#endif
    WindowGroupRef grp = 0;
    if ((flags & Qt::WindowStaysOnTopHint))
        ChangeWindowAttributes(windowRef, kWindowNoAttributes, kWindowHideOnSuspendAttribute);
    if (qt_mac_is_macdrawer(q) && parentWidget)
        SetDrawerParent(windowRef, qt_mac_window_for (parentWidget));
    if (dialog && !parentWidget && !q->testAttribute(Qt::WA_ShowModal))
        grp = GetWindowGroupOfClass(kDocumentWindowClass);
    if (topExtra->group) {
        qt_mac_release_window_group(topExtra->group);
        topExtra->group = 0;
    }
    if (flags & Qt::WindowStaysOnTopHint) {
        topExtra->group = qt_mac_get_stays_on_top_group(type);
        SetWindowGroup(windowRef, topExtra->group);
    } else if (grp) {
        SetWindowGroup(windowRef, grp);
    }
#ifdef DEBUG_WINDOW_CREATE
    if (WindowGroupRef grpf = GetWindowGroup(windowRef)) {
        QCFString cfname;
        CopyWindowGroupName(grpf, &cfname);
        SInt32 lvl;
        GetWindowGroupLevel(grpf, &lvl);
        const char *from = "Default";
        if (topExtra && grpf == topData()->group)
            from = "Created";
        else if (grpf == grp)
            from = "Copied";
        qDebug("Qt: internal: With window group '%s' [%p] @ %d: %s",
                static_cast<QString>(cfname).toLatin1().constData(), grpf, (int)lvl, from);
    } else {
        qDebug("Qt: internal: No window group!!!");
    }
    HIWindowAvailability hi_avail = 0;
    if (HIWindowGetAvailability(windowRef, &hi_avail) == noErr) {
        struct {
            UInt32 tag;
            const char *name;
        } known_avail[] = {
            ADD_DEBUG_WINDOW_NAME(kHIWindowExposeHidden),
            { 0, 0 }
        };
        qDebug("Qt: internal: ** HIWindowAvailibility:");
        for (int i = 0; hi_avail && known_avail[i].name; i++) {
            if ((hi_avail & known_avail[i].tag) == known_avail[i].tag) {
                hi_avail ^= known_avail[i].tag;
                qDebug("Qt: internal: * %s", known_avail[i].name);
            }
        }
        if (hi_avail)
            qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)hi_avail);
    }
#undef ADD_DEBUG_WINDOW_NAME
#endif
    if (extra && !extra->mask.isEmpty())
        ReshapeCustomWindow(windowRef);
    SetWindowModality(windowRef, kWindowModalityNone, 0);
    if (qt_mac_is_macdrawer(q))
        SetDrawerOffsets(windowRef, 0.0, 25.0);
    data.fstrut_dirty = true; // when we create a toplevel widget, the frame strut should be dirty
    HIViewRef hiview = (HIViewRef)data.winid;
    HIViewRef window_hiview = qt_mac_hiview_for(windowRef);
    if(!hiview) {
        hiview = qt_mac_create_widget(window_hiview);
        setWinId((WId)hiview);
    } else {
        HIViewAddSubview(window_hiview, hiview);
    }
    if (hiview) {
        Rect win_rect;
        GetWindowBounds(qt_mac_window_for (window_hiview), kWindowContentRgn, &win_rect);
        HIRect bounds = CGRectMake(0, 0, win_rect.right-win_rect.left, win_rect.bottom-win_rect.top);
        HIViewSetFrame(hiview, &bounds);
        HIViewSetVisible(hiview, true);
        if (q->testAttribute(Qt::WA_DropSiteRegistered))
            registerDropSite(true);
        transferChildren();
    }
    initWindowPtr();

    if (topExtra->posFromMove) {
        updateFrameStrut();
        const QRect &fStrut = frameStrut();
        SetRect(&r, r.left + fStrut.left(), r.top + fStrut.top(),
                    (r.left + fStrut.left() + data.crect.width()) - fStrut.right(),
                    (r.top + fStrut.top() + data.crect.height()) - fStrut.bottom());
        SetWindowBounds(windowRef, kWindowContentRgn, &r);
        topExtra->posFromMove = false;
    }

    if (qt_mac_is_macsheet(q))
        q->setWindowOpacity(0.95);
    else if (topExtra->opacity != 255)
        q->setWindowOpacity(topExtra->opacity / 255.0f);

    // Since we only now have a window, sync our state.
    qt_mac_update_opaque_sizegrip(q);
    qt_mac_update_metal_style(q);
    qt_mac_update_ignore_mouseevents(q);
    setWindowTitle_helper(extra->topextra->caption);
    setWindowIconText_helper(extra->topextra->iconText);
    setWindowModified_sys(q->isWindowModified());
    updateFrameStrut();
}

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);
    window_event = 0;
    HIViewRef destroyid = 0;

    Qt::WindowType type = q->windowType();
    Qt::WindowFlags flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || type == Qt::Drawer
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);

    if (desktop) {
        QSize desktopSize = qt_mac_desktopSize();
        q->setAttribute(Qt::WA_WState_Visible);
        data.crect.setRect(0, 0, desktopSize.width(), desktopSize.height());
        dialog = popup = false;                  // force these flags off
    } else {
        q->setAttribute(Qt::WA_WState_Visible, false);

        if (topLevel && (type != Qt::Drawer)) {
            if(QDesktopWidget *dsk = QApplication::desktop()) { // calc pos/size from screen
                const bool wasResized = q->testAttribute(Qt::WA_Resized);
                const bool wasMoved = q->testAttribute(Qt::WA_Moved);

                int deskn = dsk->primaryScreen();
                if(parentWidget && parentWidget->windowType() != Qt::Desktop)
                    deskn = dsk->screenNumber(parentWidget);
                QRect dskr = dsk->screenGeometry(deskn);
                if (!wasResized)
                    data.crect.setSize(QSize(dskr.width()/2, 4*dskr.height()/10));
                if (!wasMoved)
                    data.crect.moveTopLeft(QPoint(dskr.width()/4, 3*dskr.height()/10));
            }
        }
    }


    if(!window)                              // always initialize
        initializeWindow=true;

    hd = 0;
    if(window) {                                // override the old window (with a new HIViewRef)
        HIViewRef hiview = (HIViewRef)window, parent = 0;
        CFRetain(hiview);
        if(destroyOldWindow)
            destroyid = qt_mac_hiview_for(q);
        bool transfer = false;
        setWinId((WId)hiview);
#ifndef HIViewInstallEventHandler
        // Macro taken from the CarbonEvents Header on Tiger
#define HIViewInstallEventHandler( target, handler, numTypes, list, userData, outHandlerRef ) \
               InstallEventHandler( HIObjectGetEventTarget( (HIObjectRef) (target) ), (handler), (numTypes), (list), (userData), (outHandlerRef) )
#endif
        HIViewInstallEventHandler(hiview, make_widget_eventUPP(), GetEventTypeCount(widget_events), widget_events, 0, 0);
        if(topLevel) {
            determineWindowClass();
            for(int i = 0; i < 2; ++i) {
                if(i == 1) {
                    if(!initializeWindow)
                        break;
                    createWindow_sys();
                }
                if(WindowRef windowref = qt_mac_window_for(hiview)) {
                    RetainWindow(windowref);
                    parent = qt_mac_hiview_for(windowref);
                    break;
                }
            }
            if(!parent)
                transfer = true;
        } else if (parentWidget) {
            // I need to be added to my parent, therefore my parent needs an HIViewRef
            parentWidget->createWinId();
            parent = qt_mac_hiview_for(parentWidget);
        }
        if(parent)
            HIViewAddSubview(parent, hiview);
        if(transfer)
            transferChildren();
        data.fstrut_dirty = true; // we'll re calculate this later
        q->setAttribute(Qt::WA_WState_Visible, HIViewIsVisible(hiview));
        if(initializeWindow) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(), data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            q->setAttribute(Qt::WA_WState_Visible, HIViewIsVisible(hiview));
        }
        initWindowPtr();
    } else if(desktop) {                        // desktop widget
        if(!qt_root_win)
            QWidgetPrivate::qt_create_root_win();
        CFRetain(qt_root_win);
        if(HIViewRef hiview = HIViewGetRoot(qt_root_win)) {
            CFRetain(hiview);
            setWinId((WId)hiview);
        }
    } else if(topLevel) {
        determineWindowClass();
        if(HIViewRef hiview = qt_mac_create_widget(0)) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(),
                                       data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            setWinId((WId)hiview);
        }
    } else {
        data.fstrut_dirty = false; // non-toplevel widgets don't have a frame, so no need to update the strut
        if(HIViewRef hiview = qt_mac_create_widget(qt_mac_hiview_for(parentWidget))) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(), data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            setWinId((WId)hiview);
            if (q->testAttribute(Qt::WA_DropSiteRegistered))
                registerDropSite(true);
        }
    }

    updateIsOpaque();
    if (!topLevel && initializeWindow)
        setWSGeometry();

    if(destroyid) {
        HIViewRemoveFromSuperview(destroyid);
        CFRelease(destroyid);
    }
}

/*!
    Returns the QuickDraw handle of the widget. Use of this function is not
    portable. This function will return 0 if QuickDraw is not supported, or
    if the handle could not be created.

    \warning This function is only available on Mac OS X.
*/

Qt::HANDLE
QWidget::macQDHandle() const
{
    Q_D(const QWidget);
    return d->qd_hd;
}

/*!
    Returns the CoreGraphics handle of the widget. Use of this function is
    not portable. This function will return 0 if no painter context can be
    established, or if the handle could not be created.

    \warning This function is only available on Mac OS X.

    \sa handle()
*/

Qt::HANDLE
QWidget::macCGHandle() const
{
    return handle();
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
    d->deactivateWidgetCleanup();
    qt_mac_event_release(this);
    if(testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
        QObjectList chldrn = children();
        for(int i = 0; i < chldrn.size(); i++) {  // destroy all widget children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows, destroySubWindows);
        }
        if(mac_mouse_grabber == this)
            releaseMouse();
        if(mac_keyboard_grabber == this)
            releaseKeyboard();
        if(acceptDrops())
            setAcceptDrops(false);

        if(testAttribute(Qt::WA_ShowModal))          // just be sure we leave modal
            QApplicationPrivate::leaveModal(this);
        else if((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);
        if(destroyWindow) {
            if(d->window_event)
                RemoveEventHandler(d->window_event);
            if(HIViewRef hiview = qt_mac_hiview_for(this)) {
                WindowPtr window = isWindow() ? qt_mac_window_for(hiview) : 0;
                if(window) {
                    RemoveWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget);
                    ReleaseWindow(window);
                } else {
                    HIViewRemoveFromSuperview(hiview);
                    CFRelease(hiview);
                }
            }
        }
        d->setWinId(0);
    }
}

void QWidgetPrivate::transferChildren()
{
    Q_Q(QWidget);
    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) {
        QObject *obj = chlist.at(i);
        if(obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if(!w->isWindow()) {
                if (!topData()->caption.isEmpty())
                    setWindowTitle_helper(extra->topextra->caption);
                HIViewAddSubview(qt_mac_hiview_for(q), qt_mac_hiview_for(w));
            }
        }
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WindowFlags f)
{
    Q_Q(QWidget);
    QTLWExtra *topData = maybeTopData();
    bool wasCreated = q->testAttribute(Qt::WA_WState_Created);

    EventHandlerRef old_window_event = 0;
    HIViewRef old_id = 0;
    if (wasCreated && !(q->windowType() == Qt::Desktop)) {
        old_id = qt_mac_hiview_for(q);
        old_window_event = window_event;
    }
    QWidget* oldtlw = q->window();

    if (q->testAttribute(Qt::WA_DropSiteRegistered))
        q->setAttribute(Qt::WA_DropSiteRegistered, false);

    //recreate and setup flags
    QObjectPrivate::setParent_helper(parent);
    QPoint pt = q->pos();
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);
    setWinId(0); //do after the above because they may want the id

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    adjustFlags(data.window_flags, q);
    //### simplify logic after TP
    if (wasCreated && !q->isWindow() && !parent->testAttribute(Qt::WA_WState_Created))
        parent->d_func()->createWinId();
    if (parent && !q->isWindow() && parent->testAttribute(Qt::WA_WState_Created))
        q->create(0, true, false);
    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (wasCreated) {
        if (q->data->winid != 0) {
            transferChildren();
            if (topData && !topData->caption.isEmpty())
                setWindowTitle_helper(topData->caption);
        } else {
            uncreateRecursively(false);
        }
    }

    if (q->testAttribute(Qt::WA_AcceptDrops)
        || (!q->isWindow() && q->parentWidget()
            && q->parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
        q->setAttribute(Qt::WA_DropSiteRegistered, true);

    //cleanup
    if(old_window_event)
        RemoveEventHandler(old_window_event);
    if(old_id) { //don't need old window anymore
        WindowPtr window = (oldtlw == q) ? qt_mac_window_for(old_id) : 0;
        if(window) {
            RemoveWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget);
            ReleaseWindow(window);
        } else {
            HIViewRemoveFromSuperview(old_id);
            CFRelease(old_id);
        }
    }
    qt_event_request_window_change();
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created)) {
        QPoint p = pos + data->crect.topLeft();
        return isWindow() ?  p : parentWidget()->mapToGlobal(p);
    }
    QPoint tmp = d->mapToWS(pos);
    HIPoint hi_pos = CGPointMake(tmp.x(), tmp.y());
    HIViewConvertPoint(&hi_pos, qt_mac_hiview_for(this), 0);
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    return QPoint((int)hi_pos.x+win_rect.left, (int)hi_pos.y+win_rect.top);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created)) {
        QPoint p = isWindow() ?  pos : parentWidget()->mapFromGlobal(pos);
        return p - data->crect.topLeft();
    }
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    HIPoint hi_pos = CGPointMake(pos.x()-win_rect.left, pos.y()-win_rect.top);
    HIViewConvertPoint(&hi_pos, 0, qt_mac_hiview_for(this));
    return d->mapFromWS(QPoint((int)hi_pos.x, (int)hi_pos.y));
}

void QWidgetPrivate::updateSystemBackground()
{
}

void QWidgetPrivate::setCursor_sys(const QCursor &cursor)
{
    Q_Q(QWidget);
    if(qApp && q->isEnabled() && qApp->activeWindow() &&
       QApplication::widgetAt(QCursor::pos()) == q) {
        const QCursor *n = &cursor;
        if(QApplication::overrideCursor())
            n = QApplication::overrideCursor();
        qt_mac_set_cursor(n, QCursor::pos());
    }
}

void QWidgetPrivate::unsetCursor_sys()
{
    Q_Q(QWidget);
    if(qApp && q->isEnabled() && qApp->activeWindow() &&
       QApplication::widgetAt(QCursor::pos()) == q) {
        const QCursor *n = 0;
        if(QApplication::overrideCursor()) {
            n = QApplication::overrideCursor();
        } else {
            for(QWidget *p = q; p; p = p->parentWidget()) {
                QWExtra *extra = p->d_func()->extraData();
                if(extra && extra->curs) {
                    n = extra->curs;
                    break;
                }
            }
        }
        const QCursor def(Qt::ArrowCursor);
        if(!n) n = &def; //I give up..
        qt_mac_set_cursor(n, QCursor::pos());
    }
}

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    if(q->isWindow())
        SetWindowTitleWithCFString(qt_mac_window_for(q), QCFString(caption));
}

void QWidgetPrivate::setWindowModified_sys(bool mod)
{
    Q_Q(QWidget);
    if (q->isWindow() && q->testAttribute(Qt::WA_WState_Created))
        SetWindowModified(qt_mac_window_for(q), mod);
}

void QWidgetPrivate::setWindowIcon_sys(bool forceReset)
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created))
        return;

    QTLWExtra *topData = this->topData();
    if (topData->iconPixmap && !forceReset) // already set
        return;

    QIcon icon = q->windowIcon();
    QPixmap *pm = 0;
    if (!icon.isNull()) {
        // now create the extra
        if (!topData->iconPixmap) {
            pm = new QPixmap(icon.pixmap(QSize(22, 22)));
            topData->iconPixmap = pm;
        } else {
            pm = topData->iconPixmap;
        }
    }
    if (q->isWindow()) {
        if (icon.isNull()) {
            RemoveWindowProxy(qt_mac_window_for(q));
        } else {
            WindowClass wclass;
            GetWindowClass(qt_mac_window_for(q), &wclass);
            if (wclass == kDocumentWindowClass)
                SetWindowProxyIcon(qt_mac_window_for(q), qt_mac_create_iconref(*pm));
        }
    }
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
    Q_Q(QWidget);
    if(q->isWindow() && !iconText.isEmpty())
        SetWindowAlternateTitle(qt_mac_window_for(q), QCFString(iconText));
}

void QWidget::grabMouse()
{
    if(isVisible() && !qt_nograb()) {
        if(mac_mouse_grabber)
            mac_mouse_grabber->releaseMouse();
        mac_mouse_grabber=this;
    }
}

void QWidget::grabMouse(const QCursor &)
{
    if(isVisible() && !qt_nograb()) {
        if(mac_mouse_grabber)
            mac_mouse_grabber->releaseMouse();
        mac_mouse_grabber=this;
    }
}

void QWidget::releaseMouse()
{
    if(!qt_nograb() && mac_mouse_grabber == this)
        mac_mouse_grabber = 0;
}

void QWidget::grabKeyboard()
{
    if(!qt_nograb()) {
        if(mac_keyboard_grabber)
            mac_keyboard_grabber->releaseKeyboard();
        mac_keyboard_grabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if(!qt_nograb() && mac_keyboard_grabber == this)
        mac_keyboard_grabber = 0;
}

QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}

void QWidget::activateWindow()
{
    QWidget *tlw = window();
    if(!tlw->isVisible() || !tlw->isWindow() || (tlw->windowType() == Qt::Desktop))
        return;
    qt_event_remove_activate();
    qt_mac_set_fullscreen_mode((tlw->windowState() & Qt::WindowFullScreen) &&
                               !qApp->desktop()->screenNumber(this));
    WindowPtr window = qt_mac_window_for(tlw);
    if((tlw->windowType() == Qt::Popup) || (tlw->windowType() == Qt::Tool) ||
       qt_mac_is_macdrawer(tlw) || IsWindowActive(window)) {
        ActivateWindow(window, true);
        qApp->setActiveWindow(tlw);
    } else if(!isMinimized()){
        SelectWindow(window);
    }
    SetUserFocusWindow(window);
}

void QWidget::update()
{
    update(0, 0, width(), height());
}

void QWidget::update(const QRect &r)
{
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if(w < 0)
        w = data->crect.width()  - x;
    if(h < 0)
        h = data->crect.height() - y;
    if(w && h)
        update(QRegion(x, y, w, h));
}

void QWidget::update(const QRegion &rgn)
{
    if(updatesEnabled() && isVisible()) {
        if (testAttribute(Qt::WA_WState_InPaintEvent))
            QApplication::postEvent(this, new QUpdateLaterEvent(rgn));
        else
            HIViewSetNeedsDisplayInRegion(qt_mac_hiview_for(this), rgn.handle(true), true);
    }
}

void QWidget::repaint(const QRegion &rgn)
{
    if(rgn.isEmpty())
        return;

    HIViewSetNeedsDisplayInRegion(qt_mac_hiview_for(this), rgn.handle(true), true);
#if 0 && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    OSStatus (*HIViewRender_ptr)(HIViewRef) = HIViewRender; // workaround for gcc warning
    if(HIViewRender_ptr)
        (*HIViewRender_ptr)(qt_mac_hiview_for(window())); //yes the top level!!
#endif
}

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop)) //desktop is always visible
        return;

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);

    if(q->isWindow() && !topData()->is_moved) {
        q->createWinId();
        if (QWidget *p = q->parentWidget()) {
            p->createWinId();
            RepositionWindow(qt_mac_window_for(q), qt_mac_window_for(p), kWindowCenterOnParentWindow);
        } else {
            RepositionWindow(qt_mac_window_for(q), 0, kWindowCenterMainScreen);
        }
    }
    data.fstrut_dirty = true;
    if(q->isWindow()) {
        WindowPtr window = qt_mac_window_for(q);
        SizeWindow(window, q->width(), q->height(), true);
        if(qt_mac_is_macsheet(q)) {
            qt_event_request_showsheet(q);
        } else if(qt_mac_is_macdrawer(q)) {
            OpenDrawer(window, kWindowEdgeDefault, false);
        } else {
            if (data.window_modality == Qt::WindowModal) {
                if (q->parentWidget())
                    SetWindowModality(window, kWindowModalityWindowModal,
                                      qt_mac_window_for(q->parentWidget()->window()));
            }
            ShowHide(window, true);
            toggleDrawers(true);
        }
        if(q->windowState() & Qt::WindowMinimized) //show in collapsed state
            CollapseWindow(window, true);
        qt_event_request_activate(q);
    } else if(!q->parentWidget() || q->parentWidget()->isVisible()) {
        HIViewSetVisible(qt_mac_hiview_for(q), true);
    }
    qt_event_request_window_change();
}

void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop)) //you can't hide the desktop!
        return;

    if(q->isWindow()) {
        WindowPtr window = qt_mac_window_for(q);
        if(qt_mac_is_macsheet(q)) {
            WindowPtr parent = 0;
            if(GetSheetWindowParent(window, &parent) != noErr || !parent)
                ShowHide(window, false);
            else
                HideSheetWindow(window);
        } else if(qt_mac_is_macdrawer(q)) {
            CloseDrawer(window, false);
        } else {
            ShowHide(window, false);
            toggleDrawers(false);
            if (data.window_modality == Qt::WindowModal) {
                if (q->parentWidget())
                    SetWindowModality(window, kWindowModalityNone,
                                      qt_mac_window_for(q->parentWidget()->window()));
            }
        }
        if(q->isActiveWindow() && !(q->windowType() == Qt::Popup)) {
            QWidget *w = 0;
            if(q->parentWidget())
                w = q->parentWidget()->window();
            if(!w || (!w->isVisible() && !w->isMinimized())) {
                for(WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true);
                    wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
                    if((w = qt_mac_find_window(wp)))
                        break;
                }
            }
            if(w && w->isVisible() && !w->isMinimized())
                qt_event_request_activate(w);
        }
    } else {
        HIViewSetVisible(qt_mac_hiview_for(q), false);
    }
    qt_event_request_window_change();
    deactivateWidgetCleanup();
    qt_mac_event_release(q);
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    bool needShow = false;
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;

    if(isWindow()) {
        if((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if(newstate & Qt::WindowFullScreen) {
                if(QTLWExtra *tlextra = d->topData()) {
                    if(tlextra->normalGeometry.width() < 0) {
                        if(testAttribute(Qt::WA_Resized))
                            tlextra->normalGeometry = geometry();
                        else
                            tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
                    }
                    tlextra->savedFlags = windowFlags();
                }
                needShow = isVisible();
                const QRect fullscreen(qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this)));
                setParent(0, Qt::Window | Qt::FramelessWindowHint | (windowFlags() & 0xffff0000)); //save
                setGeometry(fullscreen);
                if(!qApp->desktop()->screenNumber(this))
                    qt_mac_set_fullscreen_mode(true);
            } else {
                needShow = isVisible();
                setParent(0, d->topData()->savedFlags);
                setGeometry(d->topData()->normalGeometry);
                if(!qApp->desktop()->screenNumber(this))
                    qt_mac_set_fullscreen_mode(false);
                d->topData()->normalGeometry.setRect(0, 0, -1, -1);
            }
        }

        d->createWinId();

        WindowRef window = qt_mac_window_for(this);
        if((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized))
            CollapseWindow(window, (newstate & Qt::WindowMinimized) ? true : false);

        if((newstate & Qt::WindowMaximized) && !((newstate & Qt::WindowFullScreen))) {
            if(QTLWExtra *tlextra = d->topData()) {
                if(tlextra->normalGeometry.width() < 0) {
                    if(testAttribute(Qt::WA_Resized))
                        tlextra->normalGeometry = geometry();
                    else
                        tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
                }
            }
        } else if(!(newstate & Qt::WindowFullScreen)) {
//            d->topData()->normalGeometry = QRect(0, 0, -1, -1);
        }

#ifdef DEBUG_WINDOW_STATE
#define WSTATE(x) qDebug("%s -- %s -- %s", #x, (newstate & x) ? "true" : "false", (oldstate & x) ? "true" : "false")
        WSTATE(Qt::WindowMinimized);
        WSTATE(Qt::WindowMaximized);
        WSTATE(Qt::WindowFullScreen);
#undef WSTATE
#endif
        if(!(newstate & (Qt::WindowMinimized|Qt::WindowFullScreen)) &&
           ((oldstate & Qt::WindowFullScreen) || (oldstate & Qt::WindowMinimized) ||
            (oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized))) {
            if(newstate & Qt::WindowMaximized) {
                Rect bounds;
                data->fstrut_dirty = true;
                QDesktopWidget *dsk = QApplication::desktop();
                QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
                SetRect(&bounds, avail.x(), avail.y(), avail.x() + avail.width(), avail.y() + avail.height());
                if(QWExtra *extra = d->extraData()) {
                    if(bounds.right - bounds.left > extra->maxw)
                        bounds.right = bounds.left + extra->maxw;
                    if(bounds.bottom - bounds.top > extra->maxh)
                        bounds.bottom = bounds.top + extra->maxh;
                }
                if(d->topData()) {
                    QRect fs = d->frameStrut();
                    bounds.left += fs.left();
                    if(bounds.right < avail.x()+avail.width())
                        bounds.right = qMin<short>((uint)avail.x()+avail.width(), bounds.right+fs.left());
                    if(bounds.bottom < avail.y()+avail.height())
                        bounds.bottom = qMin<short>((uint)avail.y()+avail.height(), bounds.bottom+fs.top());
                    bounds.top += fs.top();
                    bounds.right -= fs.right();
                    bounds.bottom -= fs.bottom();
                }
                QRect orect(geometry().x(), geometry().y(), width(), height()),
                      nrect(bounds.left, bounds.top, bounds.right - bounds.left,
                            bounds.bottom - bounds.top);
                if(orect != nrect) { // no real point..
                    Rect oldr;
                    QTLWExtra *tlextra = d->topData();
                    SetRect(&oldr, tlextra->normalGeometry.left(), tlextra->normalGeometry.top(),
                        tlextra->normalGeometry.right() + 1, tlextra->normalGeometry.bottom() + 1);
                    SetWindowUserState(window, &oldr);

                    SetWindowStandardState(window, &bounds);
                    ZoomWindow(window, inZoomOut, false);
                    setGeometry(nrect);
                }
            } else if(oldstate & Qt::WindowMaximized) {
                ZoomWindow(window, inZoomIn, false);
                if(QTLWExtra *tlextra = d->topData()) {
                    setGeometry(tlextra->normalGeometry);
                    tlextra->normalGeometry.setRect(0, 0, -1, -1);
                }
            }
        }
    }

    data->window_state = newstate;

    if(needShow)
        show();

    if(newstate & Qt::WindowActive)
        activateWindow();

    qt_event_request_window_change();
    QWindowStateChangeEvent e(oldstate);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop))
        return;
    if(q->isWindow()) {
        //raise this window
        BringToFront(qt_mac_window_for(q));
        //we get to be the active process now
        ProcessSerialNumber psn;
        GetCurrentProcess(&psn);
        SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
    } else if(q->parentWidget()) {
        HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderAbove, 0);
        qt_event_request_window_change();
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop))
        return;
    if(q->isWindow()) {
        SendBehind(qt_mac_window_for(q), 0);
    } else if(q->parentWidget()) {
        HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderBelow, 0);
        qt_event_request_window_change();
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget *w)
{
    Q_Q(QWidget);
    if(!w || q->isWindow() || (q->windowType() == Qt::Desktop))
        return;

    QWidget *p = q->parentWidget();
    if(!p || p != w->parentWidget())
        return;
    HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderBelow, qt_mac_hiview_for(w));
    qt_event_request_window_change();
}

/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to OS X's 16bit coordinate system.

  Sets the geometry of the widget to data.crect, but clipped to sizes
  that OS X can handle. Unmaps widgets that are completely outside the
  valid range.

  Maintains data.wrect, which is the geometry of the OS X widget,
  measured in this widget's coordinate system.

  if the parent is not clipped, parentWRect is empty, otherwise
  parentWRect is the geometry of the parent's OS X rect, measured in
  parent's coord sys
*/
void QWidgetPrivate::setWSGeometry(bool dontShow)
{
    Q_Q(QWidget);

    /*
      There are up to four different coordinate systems here:
      Qt coordinate system for this widget.
      X coordinate system for this widget (relative to wrect).
      Qt coordinate system for parent
      X coordinate system for parent (relative to parent's wrect).
    */
    QRect validRange(-XCOORD_MAX,-XCOORD_MAX, 2*XCOORD_MAX, 2*XCOORD_MAX);
    QRect wrectRange(-WRECT_MAX,-WRECT_MAX, 2*WRECT_MAX, 2*WRECT_MAX);
    QRect wrect;
    //xrect is the X geometry of my X widget. (starts out in  parent's Qt coord sys, and ends up in parent's X coord sys)
    QRect xrect = data.crect;

    QRect parentWRect = q->parentWidget()->data->wrect;

    if (parentWRect.isValid()) {
        // parent is clipped, and we have to clip to the same limit as parent
        if (!parentWRect.contains(xrect)) {
            xrect &= parentWRect;
            wrect = xrect;
            //translate from parent's to my Qt coord sys
            wrect.translate(-data.crect.topLeft());
        }
        //translate from parent's Qt coords to parent's X coords
        xrect.translate(-parentWRect.topLeft());

    } else {
        // parent is not clipped, we may or may not have to clip

        if (data.wrect.isValid() && QRect(QPoint(),data.crect.size()).contains(data.wrect)) {
            // This is where the main optimization is: we are already
            // clipped, and if our clip is still valid, we can just
            // move our window, and do not need to move or clip
            // children

            QRect vrect = xrect & q->parentWidget()->rect();
            vrect.translate(-data.crect.topLeft()); //the part of me that's visible through parent, in my Qt coords
            if (data.wrect.contains(vrect)) {
                xrect = data.wrect;
                xrect.translate(data.crect.topLeft());
                HIRect bounds = CGRectMake(xrect.x(), xrect.y(),
                                           xrect.width(), xrect.height());
                HIViewSetFrame(qt_mac_hiview_for(q), &bounds);
                if (q->testAttribute(Qt::WA_OutsideWSRange)) {
                    q->setAttribute(Qt::WA_OutsideWSRange, false);
                    if (!dontShow) {
                        q->setAttribute(Qt::WA_Mapped);
                        HIViewSetVisible(qt_mac_hiview_for(q), true);
                    }
                }
                return;
            }
        }

        if (!validRange.contains(xrect)) {
            // we are too big, and must clip
            xrect &=wrectRange;
            wrect = xrect;
            wrect.translate(-data.crect.topLeft());
            //parent's X coord system is equal to parent's Qt coord
            //sys, so we don't need to map xrect.
        }

    }

    // unmap if we are outside the valid window system coord system
    bool outsideRange = !xrect.isValid();
    bool mapWindow = false;
    if (q->testAttribute(Qt::WA_OutsideWSRange) != outsideRange) {
        q->setAttribute(Qt::WA_OutsideWSRange, outsideRange);
        if (outsideRange) {
            HIViewSetVisible(qt_mac_hiview_for(q), false);
            q->setAttribute(Qt::WA_Mapped, false);
        } else if (!q->isHidden()) {
            mapWindow = true;
        }
    }

    if (outsideRange)
        return;

    bool jump = (data.wrect != wrect);
    data.wrect = wrect;


    // and now recursively for all children...
    // ### can be optimized
    for (int i = 0; i < children.size(); ++i) {
        QObject *object = children.at(i);
        if (object->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(object);
            if (!w->isWindow() && w->testAttribute(Qt::WA_WState_Created))
                w->d_func()->setWSGeometry();
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    HIRect bounds = CGRectMake(xrect.x(), xrect.y(),
                               xrect.width(), xrect.height());
    HIViewSetFrame(qt_mac_hiview_for(q), &bounds);

    if  (jump) {
        updateSystemBackground();
        q->update();
    }
    if (mapWindow && !dontShow) {
        q->setAttribute(Qt::WA_Mapped);
        HIViewSetVisible(qt_mac_hiview_for(q), true);
    }
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    if(q->windowType() == Qt::Desktop)
        return;

    // Special case for resizing a window: call SetWindowBounds which will
    // send us a kWindowBoundsChangeSizeChanged event, whose handler
    // calls setGeometry_sys_helper().
    // The reason for doing it this way is that SetWindowBounds
    // repaints the window immediately and the only way we can send our resize
    // events at the proper time (after the window has been resized but before
    // the paint) is to handle the BoundsChange event.
    if (q->isWindow()) {
        topData()->isSetGeometry = 1;
        topData()->isMove = isMove;
        Rect r; SetRect(&r, x, y, x + w, y + h);
        SetWindowBounds(qt_mac_window_for(q), kWindowContentRgn, &r);
        topData()->isSetGeometry = 0;
    } else {
        setGeometry_sys_helper(x, y, w, h, isMove);
    }
}

void QWidgetPrivate::setGeometry_sys_helper(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    if(q->isWindow() && isMove)
        topData()->is_moved = 1;
    if(QWExtra *extra = extraData()) {        // any size restrictions?
        if(q->isWindow()) {
            WindowPtr window = qt_mac_window_for(q);
            qt_mac_update_sizer(q);
            if(q->windowFlags() & Qt::WindowMaximizeButtonHint) {
                if(extra->maxw && extra->maxh && extra->maxw == extra->minw
                        && extra->maxh == extra->minh)
                    ChangeWindowAttributes(window, kWindowNoAttributes, kWindowFullZoomAttribute);
                else
                    ChangeWindowAttributes(window, kWindowFullZoomAttribute, kWindowNoAttributes);
            }
        }

        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);

        // Deal with size increment
        if(QTLWExtra *top = topData()) {
            if(top->incw) {
                w = w/top->incw;
                w *= top->incw;
            }
            if(top->inch) {
                h = h/top->inch;
                h *= top->inch;
            }
        }
    }

    if (q->isWindow()) {
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QPoint oldp = q->pos();
    QSize  olds = q->size();
    const bool isResize = (olds != QSize(w, h));
    if(!q->isWindow() && !isResize && QPoint(x, y) == oldp)
        return;
    if(isResize && q->isMaximized())
        data.window_state = data.window_state & ~Qt::WindowMaximized;
    const bool visible = q->isVisible();
    data.crect = QRect(x, y, w, h);

    if(q->isWindow()) {
        if(QWExtra *extra = extraData()) { //set constraints
            const float max_f(20000);
#define SF(x) ((x > max_f) ? max_f : x)
            HISize max = CGSizeMake(SF(extra->maxw), SF(extra->maxh));
            HISize min = CGSizeMake(SF(extra->minw), SF(extra->minh));
#undef SF
            SetWindowResizeLimits(qt_mac_window_for(q), &min, &max);
        }

        //update the widget.
        HIRect bounds = CGRectMake(0, 0, w, h);
        HIViewSetFrame(qt_mac_hiview_for(q), &bounds);
    } else {
        setWSGeometry();
    }

    if(isMove || isResize) {
        if(!visible) {
            if(isMove && q->pos() != oldp)
                q->setAttribute(Qt::WA_PendingMoveEvent, true);
            if(isResize)
                q->setAttribute(Qt::WA_PendingResizeEvent, true);
        } else {
            if(isResize) { //send the resize event..
                QResizeEvent e(q->size(), olds);
                QApplication::sendEvent(q, &e);
            }
            if(isMove && q->pos() != oldp) { //send the move event..
                QMoveEvent e(q->pos(), oldp);
                QApplication::sendEvent(q, &e);
            }
        }
    }
    qt_event_request_window_change();
}

void QWidgetPrivate::setConstraints_sys()
{
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    const bool valid_rect = r.isValid();
    if(!updatesEnabled() &&  (valid_rect || children().isEmpty()))
        return;

    if (HIViewGetNeedsDisplay(qt_mac_hiview_for(this))) {
        update(valid_rect ? r : rect());
        return;
    }
    if(!valid_rect) {        // scroll children
        QPoint pd(dx, dy);
        QWidgetList moved;
        QObjectList chldrn = children();
        for(int i = 0; i < chldrn.size(); i++) {  //first move all children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType()) {
                QWidget *w = (QWidget*)obj;
                if(!w->isWindow()) {
                    w->data->crect = QRect(w->pos() + pd, w->size());
                    HIRect bounds = CGRectMake(w->data->crect.x(), w->data->crect.y(),
                                               w->data->crect.width(), w->data->crect.height());
                    HIViewSetFrame(qt_mac_hiview_for(w), &bounds);
                    moved.append(w);
                }
            }
        }
        //now send move events (do not do this in the above loop, breaks QAquaFocusWidget)
        for(int i = 0; i < moved.size(); i++) {
            QWidget *w = moved.at(i);
            QMoveEvent e(w->pos(), w->pos() - pd);
            QApplication::sendEvent(w, &e);
        }
    }
    if(isVisible()) {
        HIRect scrollrect = CGRectMake(r.x(), r.y(), r.width(), r.height());
        HIViewScrollRect(qt_mac_hiview_for(this), valid_rect ? &scrollrect : 0, dx, dy);
    }
}

int QWidget::metric(PaintDeviceMetric m) const
{
    switch(m) {
    case PdmHeightMM: // 75 dpi is 3dpmm
        return (metric(PdmHeight)*100)/288;
    case PdmWidthMM: // 75 dpi is 3dpmm
        return (metric(PdmWidth)*100)/288;
    case PdmHeight:
    case PdmWidth: {
        HIRect rect;
        HIViewGetFrame(qt_mac_hiview_for(this), &rect);
        if(m == PdmWidth)
            return (int)rect.size.width;
        return (int)rect.size.height; }
    case PdmDepth:
        return 32;
    case PdmNumColors:
        return INT_MAX;
    case PdmDpiX:
    case PdmPhysicalDpiX: {
        extern float qt_mac_defaultDpi_x(); //qpaintdevice_mac.cpp
        return int(qt_mac_defaultDpi_x()); }
    case PdmDpiY:
    case PdmPhysicalDpiY: {
        extern float qt_mac_defaultDpi_y(); //qpaintdevice_mac.cpp
        return int(qt_mac_defaultDpi_y()); }
    default: //leave this so the compiler complains when new ones are added
        qWarning("QWidget::metric: Unhandled parameter %d", m);
        return QPaintDevice::metric(m);
    }
    return 0;
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->wclass = 0;
    extra->topextra->group = 0;
    extra->topextra->is_moved = 0;
    extra->topextra->resizer = 0;
    extra->topextra->isSetGeometry = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group) {
        qt_mac_release_window_group(extra->topextra->group);
        extra->topextra->group = 0;
    }
}

void QWidgetPrivate::updateFrameStrut()
{
    Q_Q(QWidget);

    QWidgetPrivate *that = const_cast<QWidgetPrivate*>(this);

    that->data.fstrut_dirty = false;
    QTLWExtra *top = that->topData();

    WindowPtr window = qt_mac_window_for(q);
    Rect window_r, content_r;
    //get bounding rects
    RgnHandle rgn = qt_mac_get_rgn();
    GetWindowRegion(window, kWindowStructureRgn, rgn);
    GetRegionBounds(rgn, &window_r);
    GetWindowRegion(window, kWindowContentRgn, rgn);
    GetRegionBounds(rgn, &content_r);
    qt_mac_dispose_rgn(rgn);
    //put into qt structure
    top->frameStrut.setCoords(content_r.left - window_r.left,
                              content_r.top - window_r.top,
                              window_r.right - content_r.right,
                              window_r.bottom - content_r.bottom);
}

void QWidgetPrivate::registerDropSite(bool on)
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created))
        return;
    SetControlDragTrackingEnabled(qt_mac_hiview_for(q), on);
}

void QWidget::setMask(const QRegion &region)
{
    Q_D(QWidget);
    // ### Paul: Consider making this cross-platform?
    if (region.isEmpty() && (!d->extra || d->extraData()->mask.isEmpty()))
        return;

    d->createExtra();
    d->extra->mask = region;
    if (!testAttribute(Qt::WA_WState_Created))
        return;

    if (isWindow())
        ReshapeCustomWindow(qt_mac_window_for(this));
    else
        HIViewReshapeStructure(qt_mac_hiview_for(this));
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

extern "C" {
    typedef struct CGSConnection *CGSConnectionRef;
    typedef struct CGSWindow *CGSWindowRef;
    extern OSStatus CGSSetWindowAlpha(CGSConnectionRef, CGSWindowRef, float);
    extern CGSWindowRef GetNativeWindowFromWindowRef(WindowRef);
    extern CGSConnectionRef _CGSDefaultConnection();
}

void QWidget::setWindowOpacity(qreal level)
{
    Q_D(QWidget);

    if (!isWindow())
        return;

    level = qBound(0.0, level, 1.0);
    d->topData()->opacity = (uchar)(level * 255);
    if (!testAttribute(Qt::WA_WState_Created))
        return;
    CGSSetWindowAlpha(_CGSDefaultConnection(),
                      GetNativeWindowFromWindowRef(qt_mac_window_for(this)), level);
}

qreal QWidget::windowOpacity() const
{
    return isWindow() ? ((QWidget*)this)->d_func()->topData()->opacity / 255.0 : 1.0;
}

struct QPaintEngineCleanupHandler
{
    inline QPaintEngineCleanupHandler() : engine(0) {}
    inline ~QPaintEngineCleanupHandler() { delete engine; }
    QPaintEngine *engine;
};

Q_GLOBAL_STATIC(QPaintEngineCleanupHandler, engineHandler)

QPaintEngine *QWidget::paintEngine() const
{
    QPaintEngine *&pe = engineHandler()->engine;
#ifdef QT_RASTER_PAINTENGINE
    if (!pe) {
        if(qgetenv("QT_MAC_USE_COREGRAPHICS").isNull())
            pe = new QRasterPaintEngine();
        else
            pe = new QCoreGraphicsPaintEngine();
    }
    if (pe->isActive()) {
        QPaintEngine *engine =
            qgetenv("QT_MAC_USE_COREGRAPHICS").isNull()
            ? (QPaintEngine*)new QRasterPaintEngine() : (QPaintEngine*)new QCoreGraphicsPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
#else
    if (!pe)
        pe = new QCoreGraphicsPaintEngine();
    if (pe->isActive()) {
        QPaintEngine *engine = new QCoreGraphicsPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
#endif
    return pe;
}

void QWidgetPrivate::setModal_sys()
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created))
        return;

    const QWidget * const windowParent = q->window()->parentWidget();
    const QWidget * const primaryWindow = windowParent ? windowParent->window() : 0;
    const bool primaryWindowModal = primaryWindow ? primaryWindow->testAttribute(Qt::WA_ShowModal) : false;
    const bool modal = q->testAttribute(Qt::WA_ShowModal);

    //setup the proper window class
    const WindowRef window = qt_mac_window_for(q);
    WindowClass old_wclass;
    GetWindowClass(window, &old_wclass);

    if (modal || primaryWindowModal) {
        if(old_wclass == kDocumentWindowClass || old_wclass == kFloatingWindowClass || old_wclass == kUtilityWindowClass) {
            HIWindowChangeClass(window ? window : qt_mac_window_for(q), kMovableModalWindowClass);
        }
    } else if(window) {
        WindowClass newClass = topData()->wclass;
        if (old_wclass != newClass && newClass != 0)
            HIWindowChangeClass(qt_mac_window_for(q), newClass);
    }
}

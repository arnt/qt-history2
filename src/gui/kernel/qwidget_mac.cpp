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

#include <private/qt_mac_p.h>

#include "qapplication.h"
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#include <private/qpaintengine_mac_p.h>
#include "qpainter.h"
#include "qstack.h"
#include "qstyle.h"
#include "qtextcodec.h"
#include "qtimer.h"
#include "qcleanuphandler.h"

#include <ApplicationServices/ApplicationServices.h>
#include <limits.h>

#include "qwidget_p.h"

#define d d_func()
#define q q_func()

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
static WindowGroupRef qt_mac_stays_on_top_group = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
const UInt32 kWidgetCreatorQt = 'cute';
enum {
    kWidgetPropertyQWidget = 'QWId' //QWidget *
};


/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_set_paintevent_clipping(QPaintDevice*, const QRegion&); //qpaintengine_mac.cpp
extern void qt_clear_paintevent_clipping(QPaintDevice *); //qpaintengine_mac.cpp
extern QSize qt_naturalWidgetSize(QWidget *); //qwidget.cpp
extern void qt_mac_unicode_reset_input(QWidget *); //qapplication_mac.cpp
extern void qt_mac_unicode_init(QWidget *); //qapplication_mac.cpp
extern void qt_mac_unicode_cleanup(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
extern bool qt_event_remove_activate(); //qapplication_mac.cpp
extern void qt_mac_event_release(QWidget *w); //qapplication_mac.cpp
extern void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_window_change(); //qapplication_mac.cpp
extern IconRef qt_mac_create_iconref(const QPixmap &); //qpixmap_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const Point *); //qcursor_mac.cpp
extern bool qt_nograb();
CGImageRef qt_mac_create_cgimage(const QPixmap &, Qt::PixmapDrawingMode, bool); //qpixmap_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/

static QSize qt_initial_size(QWidget *w) {
    QSize s = w->sizeHint();
    QSizePolicy::ExpandData exp;
#ifndef QT_NO_LAYOUT
    QLayout *layout = w->layout();
    if (layout) {
        if (layout->hasHeightForWidth())
            s.setHeight(layout->totalHeightForWidth(s.width()));
        exp = layout->expanding();
    } else
#endif
    {
        if (w->sizePolicy().hasHeightForWidth())
            s.setHeight(w->heightForWidth(s.width()));
        exp = w->sizePolicy().expanding();
    }
    if (exp & QSizePolicy::Horizontally)
        s.setWidth(qMax(s.width(), 200));
    if (exp & QSizePolicy::Vertically)
        s.setHeight(qMax(s.height(), 150));
#if defined(Q_WS_X11)
    QRect screen = QApplication::desktop()->screenGeometry(w->x11Info()->screen());
#else // all others
    QRect screen = QApplication::desktop()->screenGeometry(w->pos());
#endif
    s.setWidth(qMin(s.width(), screen.width()*2/3));
    s.setHeight(qMin(s.height(), screen.height()*2/3));
    return s + w->contentsMarginSize();    //account for the margins
}

QPoint posInWindow(const QWidget *w)
{
    QPoint ret = w->data->wrect.topLeft();
    while (w && !w->isTopLevel()) {
        ret += w->pos();
        w =  w->parentWidget();
    }
    return ret;
}

static void qt_mac_release_stays_on_top_group() //cleanup function
{
    ReleaseWindowGroup(qt_mac_stays_on_top_group);
    if(GetWindowGroupRetainCount(qt_mac_stays_on_top_group) == 1) { //only the global pointer exists
        ReleaseWindowGroup(qt_mac_stays_on_top_group);
        qt_mac_stays_on_top_group = 0;
    }
}

//find a QWidget from a WindowPtr
QWidget *qt_mac_find_window(WindowPtr window)
{
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

//find a WindowPtr from a QWidget/HIView
WindowPtr qt_mac_window_for(HIViewRef hiview)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3)
        return HIViewGetWindow(hiview);
#endif
    return GetControlOwner(hiview);

}
WindowPtr qt_mac_window_for(const QWidget *w)
{
    return qt_mac_window_for((HIViewRef)w->winId());
}

/* Use this function instead of ReleaseWindowGroup, this will be sure to release the
   stays on top window group (created with qt_mac_get_stays_on_top_group below) */
static void qt_mac_release_window_group(WindowGroupRef group)
{
    if(group == qt_mac_stays_on_top_group)
        qt_mac_release_stays_on_top_group();
    else
        ReleaseWindowGroup(group);
}
#define ReleaseWindowGroup(x) Are you sure you wanted to do that? (you wanted qt_mac_release_window_group)

/* We create one static stays on top window group so that all stays on top (aka popups) will
   fall into the same group and be able to be raise()'d with releation to one another (from
   within the same window group). */
static WindowGroupRef qt_mac_get_stays_on_top_group()
{
    if(!qt_mac_stays_on_top_group) {
        CreateWindowGroup(kWindowActivationScopeNone, &qt_mac_stays_on_top_group);
        SetWindowGroupLevel(qt_mac_stays_on_top_group, kCGOverlayWindowLevelKey);
        SetWindowGroupParent(qt_mac_stays_on_top_group, GetWindowGroupOfClass(kAllWindowClasses));
    }
    RetainWindowGroup(qt_mac_stays_on_top_group);
    return qt_mac_stays_on_top_group;
}


void qt_mac_update_metal_style(QWidget *w)
{
    if(w->isTopLevel()) {
        if(w->testAttribute(Qt::WA_MacMetalStyle))
            ChangeWindowAttributes(qt_mac_window_for(w), kWindowMetalAttribute, 0);
        else
            ChangeWindowAttributes(qt_mac_window_for(w), 0, kWindowMetalAttribute);
    }
}

static OSStatus qt_mac_create_window(WindowClass wclass, WindowAttributes wattr,
                                     Rect *geo, WindowPtr *w)
{
    OSStatus ret;
    if(geo->right == geo->left)
        geo->right++;
    if(geo->bottom == geo->top)
        geo->bottom++;
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
        Rect null_rect; SetRect(&null_rect, 0, 0, 1, 1);
        ret = CreateNewWindow(wclass, wattr, &null_rect, w);
        if(ret == noErr) {
            ret = SetWindowBounds(*w, kWindowContentRgn, geo);
            if(ret != noErr)
                qWarning("%s:%d This error shouldn't really ever happen!!!", __FILE__, __LINE__);
        }
    } else {
        ret = CreateNewWindow(wclass, wattr, geo, w);
    }
    return ret;
}

// window events
static EventTypeSpec window_events[] = {
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
    case kEventClassWindow:
        if(ekind == kEventWindowGetRegion) {
            WindowRef window;
            GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                              sizeof(window), 0, &window);
            CallNextEventHandler(er, event);
            WindowRegionCode wcode;
            GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, 0,
                              sizeof(wcode), 0, &wcode);
            RgnHandle rgn;
            GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL,
                              sizeof(rgn), NULL, &rgn);
            if(QWidgetPrivate::qt_widget_rgn(qt_mac_find_window(window), wcode, rgn, false))
                SetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, sizeof(rgn), &rgn);
        } else {
            handled_event = false;
        }
        break;
    case kEventClassMouse: {
        bool send_to_app = false;
        {
            WindowPartCode wpc;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                if (GetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, 0,
                                      sizeof(wpc), 0, &wpc) == noErr && wpc != inContent)
                    send_to_app = true;
            } else
#endif
            {
                HIPoint hipt;
                WindowRef wref;
                if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, 0,
                                      sizeof(hipt), 0, &hipt) == noErr
                    && GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                         sizeof(wref), 0, &wref) == noErr) {
                    Point lopt = { int(hipt.y), int(hipt.x) };
                    wpc = FindWindow(lopt, &wref);
                    if (wpc != inContent)
                        send_to_app = true;
                }
            }
        }
        if(!send_to_app) {
            WindowRef window;
            if(GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                 sizeof(window), 0, &window) == noErr) {
                HIViewRef hiview;
                if(HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &hiview) == noErr) {
                    if(QWidget *w = QWidget::find((WId)hiview))
                        send_to_app = !w->isActiveWindow();
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
static HIObjectClassRef widget_class = NULL;
static CFStringRef kObjectQWidget = CFSTR("com.trolltech.qt.widget");
static EventTypeSpec widget_events[] = {
    { kEventClassHIObject, kEventHIObjectConstruct },
    { kEventClassHIObject, kEventHIObjectDestruct },

    { kEventClassControl, kEventControlDraw },
    { kEventClassControl, kEventControlInitialize },
    { kEventClassControl, kEventControlGetPartRegion },
    { kEventClassControl, kEventControlGetClickActivation },
    { kEventClassControl, kEventControlDragEnter },
    { kEventClassControl, kEventControlDragWithin },
    { kEventClassControl, kEventControlDragLeave },
    { kEventClassControl, kEventControlDragReceive }
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
OSStatus QWidgetPrivate::qt_widget_event(EventHandlerCallRef, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassHIObject:
        if(ekind == kEventHIObjectConstruct) {
            HIViewRef view;
            if(GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef,
                                 NULL, sizeof(view), NULL, &view) == noErr) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3)
                    HIViewChangeFeatures(view, kHIViewAllowsSubviews, 0);
#endif
            }
        } else if(ekind == kEventHIObjectDestruct) {
            //nothing to really do.. or is there?
        } else {
            handled_event = false;
        }
        break;
    case kEventClassControl: {
        QWidget *widget = 0;
        HIViewRef hiview = 0;
        if(GetEventParameter(event, kEventParamDirectObject, typeControlRef,
                             NULL, sizeof(hiview), NULL, &hiview) == noErr)
            widget = QWidget::find((WId)hiview);
        if(ekind == kEventControlDraw) {
            if(widget) {
                //requested rgn
                widget->d->clp_serial++;
                RgnHandle rgn;
                GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn);
                QRegion qrgn(qt_mac_convert_mac_region(rgn));

                //get widget region
                RgnHandle widgetRgn = qt_mac_get_rgn();
                GetControlRegion(hiview, kControlStructureMetaPart, widgetRgn);
                widget->d->clp = qt_mac_convert_mac_region(widgetRgn);
                qt_mac_dispose_rgn(widgetRgn);
                if(!widget->isTopLevel()) {
                    QPoint pt(posInWindow(widget));
                    widget->d->clp.translate(pt.x(), pt.y());
                }

                //update qd port
                GrafPtr old_qdref = 0;
                if(GetEventParameter(event, kEventParamGrafPort, typeGrafPtr, NULL, sizeof(old_qdref), NULL, &old_qdref) != noErr)
                    GetGWorld(&old_qdref, 0); //just use the global port..
                if(old_qdref)
                    widget->d->hd = old_qdref;

#ifdef DEBUG_WIDGET_PAINT
                qDebug("asked to draw %p [%s::%s] %p", hiview, widget->metaObject()->className(),
                       widget->objectName().local8Bit(),
                       (HIViewRef)(widget->parentWidget() ? widget->parentWidget()->winId() : (WId)-1));
                QVector<QRect> region_rects = qrgn.rects();
                qDebug("Region! %d", region_rects.count());
                for(int i = 0; i < region_rects.count(); i++)
                    qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                           region_rects[i].width(), region_rects[i].height());
                region_rects = widget->d->clp.rects();
                qDebug("Widget Region! %d", region_rects.count());
                for(int i = 0; i < region_rects.count(); i++)
                    qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                           region_rects[i].width(), region_rects[i].height());
#endif
                if (isVisible() && isUpdatesEnabled()) { //process the actual paint event.
                    if(widget->testWState(Qt::WState_InPaintEvent))
                        qWarning("QWidget::repaint: recursive repaint detected.");

                    widget->setWState(Qt::WState_InPaintEvent);
                    qt_set_paintevent_clipping(widget, qrgn);

                    // Mapping region from system to qt (32 bit) coordinate system.
                    QPoint redirectionOffset = widget->data->wrect.topLeft();

                    if(!widget->testAttribute(Qt::WA_NoBackground) &&
                       !widget->d->isBackgroundInherited()) {
                        QBrush bg = widget->palette().brush(widget->d->bg_role);
                        QRect rr = qrgn.boundingRect();
                        bool was_unclipped = widget->testAttribute(Qt::WA_PaintUnclipped);
                        widget->setAttribute(Qt::WA_PaintUnclipped, false);
                        QPainter p(widget);
                        if(was_unclipped)
                            widget->setAttribute(Qt::WA_PaintUnclipped);
                        p.setClipRegion(qrgn);
                        QPixmap pm = bg.texture();
                        if(!pm.isNull())
                            p.drawTiledPixmap(rr, pm,
                                              QPoint((rr.x()+redirectionOffset.x())%pm.width(),
                                                     (rr.y()+redirectionOffset.y())%pm.height()));
                        else
                            p.fillRect(rr, bg.color());
                    }
                    qrgn.translate(redirectionOffset);
                    if (!redirectionOffset.isNull())
                        QPainter::setRedirected(widget, widget, redirectionOffset);
                    QPaintEvent e(qrgn);
                    QApplication::sendSpontaneousEvent(widget, &e);
                    qt_clear_paintevent_clipping(widget);

                    if (!redirectionOffset.isNull())
                        QPainter::restoreRedirected(widget);

                    widget->clearWState(Qt::WState_InPaintEvent);
                    if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
                        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
                }
                SetPort(old_qdref); //restore the state..

                //remove the old pointers, not necessary long-term, but short term it simplifies things --Sam
                widget->d->clp_serial++;
                widget->d->clp = QRegion();
                widget->d->hd = 0;
            }
        } else if(ekind == kEventControlInitialize) {
            UInt32 features = kControlSupportsDragAndDrop | kControlSupportsClickActivation;
            if(QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
                features |= (kControlSupportsEmbedding|kControlSupportsGetRegion);
            SetEventParameter(event, kEventParamControlFeatures, typeUInt32, sizeof(features), &features);
        } else if(ekind == kEventControlGetClickActivation) {
            ClickActivationResult clickT = kActivateAndIgnoreClick;
            SetEventParameter(event, kEventParamClickActivation, typeClickActivationResult,
                              sizeof(clickT), &clickT);
        } else if(ekind == kEventControlGetPartRegion) {
            handled_event = false;
            if(widget && !widget->isTopLevel()) {
                ControlPartCode part;
                GetEventParameter(event, kEventParamControlPart, typeControlPartCode, 0,
                                  sizeof(part), 0, &part);
                if(part == kControlStructureMetaPart
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                   || part == kControlClickableMetaPart
#endif
                    ) {
                    RgnHandle rgn;
                    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, NULL,
                                      sizeof(rgn), NULL, &rgn);
                    SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
                    if(QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false))
                        handled_event = true;
                }
            }
        } else if(ekind == kEventControlDragEnter || ekind == kEventControlDragWithin ||
                  ekind == kEventControlDragLeave || ekind == kEventControlDragReceive) {
            handled_event = false;
            if(widget) {
                //these are really handled in qdnd_mac.cpp just to modularize the code a little..
                DragRef drag;
                GetEventParameter(event, kEventParamDragRef, typeDragRef, NULL, sizeof(drag), NULL, &drag);
                if(widget->d->qt_mac_dnd_event(ekind, drag))
                    handled_event = true;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    if(ekind == kEventControlDragEnter)
                        SetEventParameter(event, kEventParamControlWouldAcceptDrop, typeBoolean,
                                          sizeof(handled_event), &handled_event);
                }
#endif
            }
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
        if(HIObjectRegisterSubclass(kObjectQWidget, kHIViewClassID, 0, make_widget_eventUPP(),
                                    GetEventTypeCount(widget_events), widget_events,
                                    NULL, &widget_class) != noErr)
            qWarning("That cannot happen!!! %d", __LINE__);
    }
    HIViewRef ret = 0;
    if(widget_class) {
        if(HIObjectCreate(kObjectQWidget, 0, (HIObjectRef*)&ret) != noErr)
            qWarning("That cannot happen!!! %d", __LINE__);
        if(ret)
            HIViewAddSubview(parent, ret);
    }
    return ret;
}

bool qt_mac_can_clickThrough(const QWidget *w)
{
    // Idea here is that if a parent doesn't have a clickthrough property,
    // neither can it's child
    while (w) {
	if (w->testAttribute(Qt::WA_MacNoClickThrough))
	    return false;
	w = w->parentWidget();
    }
    return true;
}

bool qt_mac_is_macdrawer(const QWidget *w)
{
#if 1
    if(w && w->parentWidget() && w->testWFlags(Qt::WMacDrawer) == Qt::WMacDrawer)
        return true;
#else
    Q_UNUSED(w);
#endif
    return false;
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::DockWindowArea where) //users of Qt/Mac can use this..
{
    if(!qt_mac_is_macdrawer(w))
        return false;
    OptionBits edge;
    if(where & Qt::LeftDockWindowArea)
        edge = kWindowEdgeLeft;
    else if(where & Qt::RightDockWindowArea)
        edge = kWindowEdgeRight;
    else if(where & Qt::TopDockWindowArea)
        edge = kWindowEdgeTop;
    else if(where & Qt::BottomDockWindowArea)
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
                if (!widget->testWState(Qt::WState_ExplicitShowHide))
                    widget->show();
            } else {
                widget->hide();
                widget->clearWState(Qt::WState_ExplicitShowHide);
            }
        }
    }
}

bool qt_mac_is_macsheet(const QWidget *w)
{
#if 1
    if(w && w->testWFlags(Qt::WMacSheet) == Qt::WMacSheet
       && w->parentWidget() && !w->parentWidget()->topLevelWidget()->isDesktop()
       && w->parentWidget()->topLevelWidget()->isVisible())
        return true;
#else
    Q_UNUSED(w);
#endif
    return false;
}



/*****************************************************************************
  QWidgetPrivate member functions
 *****************************************************************************/
bool QWidgetPrivate::qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isTopLevel())
        return false;

    w->d->extraData()->topextra->resizer += up;
    {
        WindowClass wclass;
        GetWindowClass(qt_mac_window_for(w), &wclass);
        if(!(GetAvailableWindowAttributes(wclass) & kWindowResizableAttribute))
            return true;
    }
    bool remove_grip = (w->d->extraData()->topextra->resizer ||
                        (w->d->extraData()->maxw && w->d->extraData()->maxh &&
                         w->d->extraData()->maxw == w->d->extraData()->minw && w->d->extraData()->maxh == w->d->extraData()->minh));

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

static QList<QWidget *> qt_root_win_widgets;
static WindowPtr qt_root_win = 0;
void QWidgetPrivate::qt_clean_root_win()
{
    if(!qt_root_win)
        return;
    if(HIViewRef root_hiview = HIViewGetRoot(qt_root_win)) {
        for(int i = 0; i < qt_root_win_widgets.count(); i++) {
            QWidget *w = qt_root_win_widgets.at(i);
            if((HIViewRef)w->winId() == root_hiview)
                w->d->setWinId(0); //at least now we'll just crash
        }
    }
    qt_root_win_widgets.clear();
    ReleaseWindow(qt_root_win);
    qt_root_win = NULL;
}

bool QWidgetPrivate::qt_create_root_win() {
    if(qt_root_win)
        return false;
    Rect r;
    int w = 0, h = 0;
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
        w = qMax(w, (*g)->gdRect.right);
        h = qMax(h, (*g)->gdRect.bottom);
    }
    SetRect(&r, 0, 0, w, h);
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
    qt_root_win = NULL;
    qt_create_root_win();
    if(HIViewRef root_hiview = HIViewGetRoot(qt_root_win)) {
        for(int i = 0; i < qt_root_win_widgets.count(); i++) { //reset points
            QWidget *w = qt_root_win_widgets.at(i);
            if((HIViewRef)w->winId() == old_root_hiview)
                w->d->setWinId((WId)root_hiview);
        }
    }
    //cleanup old window
    old_root_hiview = 0;
    ReleaseWindow(old_root_win);
    return true;
}

bool QWidgetPrivate::qt_widget_rgn(QWidget *widget, short wcode, RgnHandle rgn, bool force = false)
{
    switch(wcode) {
    case kWindowStructureRgn: {
        bool ret = false;
        if(widget) {
            if(widget->d->extra && !widget->d->extra->mask.isEmpty()) {
                QRegion rin = qt_mac_convert_mac_region(rgn);
                if(!rin.isEmpty()) {
                    QPoint rin_tl = rin.boundingRect().topLeft(); //in offset
                    rin.translate(-rin_tl.x(), -rin_tl.y()); //bring into same space as below
                    QRegion mask = widget->d->extra->mask;
                    if(widget->isTopLevel() &&
                       (!widget->testWFlags(Qt::WStyle_Customize) || !widget->testWFlags(Qt::WStyle_NoBorder))) {
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
        return ret; }
    default: break;
    }
    return false;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    d->window_event = 0;
    WId destroyid = 0;
    setWState(Qt::WState_Created);                        // set created flag

    if(!parentWidget() || parentWidget()->isDesktop())
        setWFlags(Qt::WType_TopLevel);            // top-level widget

    bool topLevel = testWFlags(Qt::WType_TopLevel);
    bool popup = testWFlags(Qt::WType_Popup);
    bool dialog = testWFlags(Qt::WType_Dialog);
    bool desktop = testWFlags(Qt::WType_Desktop);

    //position
    QRect dskr;
    if(desktop) {
        int w = 0, h = 0;
        for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
            w = qMax(w, (*g)->gdRect.right);
            h = qMax(h, (*g)->gdRect.bottom);
        }
        dskr = QRect(0, 0, w, h);
        setWState(Qt::WState_Visible);
    } else {
        if(QDesktopWidget *dsk = QApplication::desktop()) {
            int deskn = dsk->primaryScreen();
            if(parentWidget() && !parentWidget()->isDesktop())
                deskn = dsk->screenNumber(parentWidget());
            dskr = dsk->screenGeometry(deskn);
        }
        clearWState(Qt::WState_Visible);
    }
    if(desktop)                             // desktop widget
        data->crect.setRect(0, 0, dskr.width(), dskr.height());
    else if(topLevel)                    // calc pos/size from screen
        data->crect.setRect(dskr.width()/4, 3*dskr.height()/10, dskr.width()/2, 4*dskr.height()/10);
    else                                    // child widget
        data->crect.setRect(0, 0, 100, 30);

    if(desktop)
        dialog = popup = false;                  // force these flags off
    if(!window)                              // always initialize
        initializeWindow=true;
    if(dialog || popup || desktop) {          // these are top-level, too
        topLevel = true;
        setWFlags(Qt::WType_TopLevel);
        if(popup)
            setWFlags(Qt::WStyle_Tool|Qt::WStyle_StaysOnTop); // a popup is a tool window
    }
    if(topLevel && parentWidget()) { // if our parent has Qt::WStyle_StaysOnTop, so must we
        QWidget *ptl = parentWidget()->topLevelWidget();
        if(ptl && ptl->testWFlags(Qt::WStyle_StaysOnTop))
            setWFlags(Qt::WStyle_StaysOnTop);
    }
    if(dialog && !testWFlags(Qt::WShowModal) && parentWidget()
            && parentWidget()->testWFlags(Qt::WShowModal))
        setWFlags(Qt::WShowModal);
    if(!testWFlags(Qt::WStyle_Customize) && !(desktop || popup) && !testWFlags(Qt::WShowModal))
        setWFlags(Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu);

    d->hd = 0;
    d->cg_hd = 0;
    if(window) {                                // override the old window
        data->fstrut_dirty = true; // we'll re calculate this later
        if(destroyOldWindow)
            destroyid = winId();
        HIViewRef hiview = (HIViewRef)window;
        HIRect bounds;
        HIViewGetFrame(hiview, &bounds);
        if(HIViewIsVisible(hiview))
            setWState(Qt::WState_Visible);
        else
            clearWState(Qt::WState_Visible);
        data->crect.setRect((int)bounds.origin.x, (int)bounds.origin.y, (int)bounds.size.width, (int)bounds.size.height);
        d->setWinId((WId)hiview);
    } else if(desktop) {                        // desktop widget
        if(!qt_root_win)
            QWidgetPrivate::qt_create_root_win();
        qt_root_win_widgets.append(this);
        if(HIViewRef hiview = HIViewGetRoot(qt_root_win)) {
            CFRetain((HIViewRef)hiview);
            d->setWinId((WId)hiview);
        }
    } else if(isTopLevel()) {
        Rect r;
        SetRect(&r, data->crect.left(), data->crect.top(), data->crect.right(), data->crect.bottom());
        WindowClass wclass = kSheetWindowClass;
        if(qt_mac_is_macdrawer(this))
            wclass = kDrawerWindowClass;
        else if(popup || (testWFlags(Qt::WStyle_Splash) == Qt::WStyle_Splash))
            wclass = kModalWindowClass;
        else if(testWFlags(Qt::WShowModal))
            wclass = kMovableModalWindowClass;
        else if(testWFlags(Qt::WStyle_ToolTip))
            wclass = kHelpWindowClass;
        else if(testWFlags(Qt::WStyle_Tool)
                || (dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop()))
            wclass = kFloatingWindowClass;
        else if(dialog)
            wclass = kToolbarWindowClass;
        else
            wclass = kDocumentWindowClass;

        WindowGroupRef grp = 0;
        WindowAttributes wattr = kWindowCompositingAttribute;
        if(testWFlags(Qt::WStyle_Customize)) {
            if(qt_mac_is_macsheet(this)) {
                grp = GetWindowGroupOfClass(kMovableModalWindowClass);
                wclass = kSheetWindowClass;
            } else {
                grp = GetWindowGroupOfClass(wclass);
                // Shift things around a bit to get the correct window class based on the presence
                // (or lack) of the border.
                if(testWFlags(Qt::WStyle_NoBorder) || testWFlags(Qt::WStyle_ToolTip)) {
                    if(wclass == kDocumentWindowClass)
                        wclass = kPlainWindowClass;
                    else if(wclass == kFloatingWindowClass)
                        wclass = kToolbarWindowClass;
                } else {
                    if(wclass != kModalWindowClass)
                        wattr |= kWindowResizableAttribute;
                    if(wclass == kToolbarWindowClass) {
                        if(!parentWidget() || parentWidget()->isDesktop())
                            wclass = kDocumentWindowClass;
                        else
                            wclass = kFloatingWindowClass;
                    }
                }
                // Only add extra decorations (well, buttons) for widgets that can have them
                // and have an actual border we can put them on.
                if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass
                    && wclass != kSheetWindowClass && wclass != kPlainWindowClass
                    && !testWFlags(Qt::WStyle_NoBorder) && wclass != kDrawerWindowClass
                    && wclass != kHelpWindowClass) {
                    if(testWFlags(Qt::WStyle_Maximize))
                        wattr |= kWindowFullZoomAttribute;
                    if(testWFlags(Qt::WStyle_Minimize))
                        wattr |= kWindowCollapseBoxAttribute;
                    if(testWFlags(Qt::WStyle_Title) || testWFlags(Qt::WStyle_SysMenu))
                       wattr |= kWindowCloseBoxAttribute;
                }
            }
        }
        if(testWFlags(Qt::WStyle_Tool) && testWFlags(Qt::WStyle_Splash) != Qt::WStyle_Splash && !isModal())
            wattr |= kWindowHideOnSuspendAttribute;
        wattr |= kWindowLiveResizeAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
        struct {
            UInt32 tag;
            const char *name;
        } known_attribs[] = {
            ADD_DEBUG_WINDOW_NAME(kWindowCompositingAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowMetalAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
            ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
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
#undef ADD_DEBUG_WINDOW_NAME
        qDebug("Qt: internal: ************* Creating new window %p (%s::%s)", this, metaObject()->className(), objectName().local8Bit());
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

        WindowRef window = 0;
        if(OSStatus ret = qt_mac_create_window(wclass, wattr, &r, &window))
            qWarning("Qt: internal: %s:%d If you reach this error please contact Trolltech and include the\n"
                   "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, ret);
        QWidget *me = this;
        if(SetWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(me), &me) != noErr)
            qWarning("Qt: internal: %s:%d This should not happen!", __FILE__, __LINE__); //no real way to recover
        if(!desktop) { //setup an event callback handler on the window
            SetAutomaticControlDragTrackingEnabledForWindow(window, true);
            InstallWindowEventHandler(window, make_win_eventUPP(), GetEventTypeCount(window_events),
                                      window_events, static_cast<void *>(qApp), &d->window_event);
        }
	if(testWFlags(Qt::WStyle_StaysOnTop) || !parentWidget())
	    ChangeWindowAttributes(window, kWindowNoAttributes, kWindowHideOnSuspendAttribute);
        if(qt_mac_is_macdrawer(this) && parentWidget())
            SetDrawerParent(window, qt_mac_window_for(parentWidget()));
        if(dialog && !parentWidget() && !testWFlags(Qt::WShowModal))
            grp = GetWindowGroupOfClass(kDocumentWindowClass);
        if(testWFlags(Qt::WStyle_StaysOnTop)) {
            if(d->topData()->group)
                qt_mac_release_window_group(d->topData()->group);
            d->topData()->group = qt_mac_get_stays_on_top_group();
            SetWindowGroup(window, d->topData()->group);
        } else if(grp) {
            SetWindowGroup(window, grp);
        }
#ifdef DEBUG_WINDOW_CREATE
        if(WindowGroupRef grpf = GetWindowGroup(window)) {
            QCFString cfname;
            CopyWindowGroupName(grpf, &cfname);
            SInt32 lvl;
            GetWindowGroupLevel(grpf, &lvl);
            const char *from = "Default";
            if(d->topData() && grpf == d->topData()->group)
                from = "Created";
            else if(grpf == grp)
                from = "Copied";
            qDebug("Qt: internal: With window group '%s' [%p] @ %d: %s",
                   static_cast<QString>(cfname).latin1(), grpf, (int)lvl, from);
        } else {
            qDebug("Qt: internal: No window group!!!");
        }
#endif
        if(d->extra && !d->extra->mask.isEmpty())
           ReshapeCustomWindow(window);
        if(testWFlags(Qt::WType_Popup) || testWFlags(Qt::WStyle_Tool))
            SetWindowModality(window, kWindowModalityNone, NULL);
        if(qt_mac_is_macsheet(this))
            setWindowOpacity(0.70);
        else if(qt_mac_is_macdrawer(this))
            SetDrawerOffsets(window, 0.0, 25.0);
        data->fstrut_dirty = true; // when we create a toplevel widget, the frame strut should be dirty
        HIViewRef window_hiview = 0;
        OSStatus err = HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &window_hiview);
        if(err == errUnknownControl)
            window_hiview = HIViewGetRoot(window);
        else if(err != noErr)
            qWarning("That cannot happen! %d [%ld]", __LINE__, err);
        if(HIViewRef hiview = qt_mac_create_widget(window_hiview)) {
            Rect win_rect;
            GetWindowBounds(qt_mac_window_for(window_hiview), kWindowContentRgn, &win_rect);
            HIRect bounds = CGRectMake(0, 0, win_rect.right-win_rect.left, win_rect.bottom-win_rect.top);
            HIViewSetFrame(hiview, &bounds);
            HIViewSetVisible(hiview, true);
            d->setWinId((WId)hiview);
        }
    } else {
        data->fstrut_dirty = false; // non-toplevel widgets don't have a frame, so no need to update the strut
        if(HIViewRef hiview = qt_mac_create_widget((HIViewRef)parentWidget()->winId())) {
            HIRect bounds = CGRectMake(data->crect.x(), data->crect.y(), data->crect.width(), data->crect.height());
            HIViewSetFrame(hiview, &bounds);
            d->setWinId((WId)hiview);
        }
    }

    d->macDropEnabled = false;
    if(HIViewRef destroy_hiview = (HIViewRef)destroyid) {
        WindowPtr window = isTopLevel() ? qt_mac_window_for(destroy_hiview) : 0;
        CFRelease(destroy_hiview);
        if(window)
            ReleaseWindow(window);
    }
    qt_mac_unicode_init(this);
}

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    d->deactivateWidgetCleanup();
    qt_mac_event_release(this);
    qt_mac_unicode_cleanup(this);
    if(isDesktop() && destroyWindow)
        qt_root_win_widgets.removeAll(this);
    if(testWState(Qt::WState_Created)) {
        clearWState(Qt::WState_Created);
        QObjectList chldrn = children();
        for(int i = 0; i < chldrn.size(); i++) {  // destroy all widget children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType())
                ((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
        }
        if(mac_mouse_grabber == this)
            releaseMouse();
        if(mac_keyboard_grabber == this)
            releaseKeyboard();
        if(acceptDrops())
            setAcceptDrops(false);

        if(testWFlags(Qt::WShowModal))          // just be sure we leave modal
            qt_leave_modal(this);
        else if(testWFlags(Qt::WType_Popup))
            qApp->closePopup(this);
        if(destroyWindow) {
            if(d->window_event)
                RemoveEventHandler(d->window_event);
            if(HIViewRef hiview = (HIViewRef)winId()) {
                WindowPtr window = isTopLevel() ? qt_mac_window_for(hiview) : 0;
                CFRelease(hiview);
                if(window) {
                    RemoveWindowProperty(qt_mac_window_for(this), kWidgetCreatorQt, kWidgetPropertyQWidget);
                    ReleaseWindow(window);
                }
            }
        }
    }
    d->setWinId(0);
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WFlags f)
{
    QCursor oldcurs;
    bool setcurs=q->testAttribute(Qt::WA_SetCursor);
    if(setcurs) {
        oldcurs = q->cursor();
        q->unsetCursor();
    }

    EventHandlerRef old_window_event = 0;
    HIViewRef old_id = 0;
    if(!q->isDesktop()) {
        old_id = (HIViewRef)q->winId();
        old_window_event = d->window_event;
    }
    QWidget* oldtlw = q->topLevelWidget();

    //recreate and seutp flags
    setWinId(0);
    QObjectPrivate::setParent_helper(parent);
    bool     dropable = q->acceptDrops();
    bool     enable = q->isEnabled();
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s = q->size();
    QString capt = q->windowTitle();
    data.widget_flags = f;
    q->clearWState(Qt::WState_Created | Qt::WState_Visible | Qt::WState_Hidden | Qt::WState_ExplicitShowHide);
    q->create();
    if(q->isTopLevel() || (!parent || parent->isVisible()))
        q->setWState(Qt::WState_Hidden);
    if(dropable)
        q->setAcceptDrops(false);

    //reparent children
    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) {
        QObject *obj = chlist.at(i);
        if(obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if(!w->isTopLevel())
                HIViewAddSubview((HIViewRef)q->winId(), (HIViewRef)w->winId());
        }
    }

    //get new hd, now move
    q->setGeometry(0, 0, s.width(), s.height());

    //reset flags and show (if neccesary)
    q->setEnabled(enable);
    q->setFocusPolicy(fp);
    q->setAcceptDrops(dropable);
    if(!capt.isNull()) {
        topData()->caption = QString::null;
        q->setWindowTitle(capt);
    }
    if(setcurs)
        q->setCursor(oldcurs);

    //cleanup
    if(old_window_event)
        RemoveEventHandler(old_window_event);
    if(old_id) { //don't need old window anymore
        WindowPtr window = (oldtlw == q) ? qt_mac_window_for(old_id) : 0;
        HIViewRemoveFromSuperview(old_id);
        CFRelease(old_id);
        if(window)
            ReleaseWindow(window);
    }
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    QPoint tmp = d->mapToWS(pos);
    HIPoint hi_pos = CGPointMake(tmp.x(), tmp.y());
    HIViewConvertPoint(&hi_pos, (HIViewRef)winId(), 0);
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    return QPoint((int)hi_pos.x+win_rect.left, (int)hi_pos.y+win_rect.top);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    HIPoint hi_pos = CGPointMake(pos.x()-win_rect.left, pos.y()-win_rect.top);
    HIViewConvertPoint(&hi_pos, 0, (HIViewRef)winId());
    return d->mapFromWS(QPoint((int)hi_pos.x, (int)hi_pos.y));
}

void QWidgetPrivate::setFont_sys(QFont *)
{
}

void QWidgetPrivate::updateSystemBackground()
{
}


void QWidget::setCursor(const QCursor &cursor)
{
    d->createExtra();
    delete d->extraData()->curs;
    d->extraData()->curs = new QCursor(cursor);
    setAttribute(Qt::WA_SetCursor);

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
        Point mouse_pos;
        QPoint qmp(QCursor::pos());
        mouse_pos.h = qmp.x();
        mouse_pos.v = qmp.y();

        const QCursor *n = &cursor;
        if(QApplication::overrideCursor())
            n = QApplication::overrideCursor();
        qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::unsetCursor()
{
    if(!isTopLevel()) {
        if(QWExtra *extra = d->extraData()) {
            delete extra->curs;
            extra->curs = 0;
        }
        setAttribute(Qt::WA_SetCursor, false);
    }

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
        Point mouse_pos;
        QPoint qmp(QCursor::pos());
        mouse_pos.h = qmp.x();
        mouse_pos.v = qmp.y();

        const QCursor *n = 0;
        if(QApplication::overrideCursor()) {
            n = QApplication::overrideCursor();
        } else {
            for(QWidget *p = this; p; p = p->parentWidget()) {
                QWExtra *extra = p->d->extraData();
                if(extra && extra->curs) {
                    n = extra->curs;
                    break;
                }
            }
        }
        const QCursor def(Qt::ArrowCursor);
        if(!n) n = &def; //I give up..
        qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::setWindowModified(bool mod)
{
    setAttribute(Qt::WA_WindowModified, mod);
    if(isTopLevel())
        SetWindowModified(qt_mac_window_for(this), mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(Qt::WA_WindowModified);
}

void QWidget::setWindowTitle(const QString &cap)
{
    if(d->topData() && d->topData()->caption == cap)
        return; // for less flicker
    d->topData()->caption = cap;
    if(isTopLevel())
        SetWindowTitleWithCFString(qt_mac_window_for(this), QCFString(cap));
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setWindowIcon_sys(const QPixmap &pixmap)
{
    QTLWExtra* x = d->topData();
    delete x->icon;
    x->icon = 0;
    if(!pixmap.isNull())
        x->icon = new QPixmap(pixmap);
    if(q->isTopLevel()) {
#ifdef QT_COMPAT
        //I hate to do this but, I've moved the application icon to
        //where it belongs in *gasp* QApplication but I'm afraid to
        //break assumptions so I'll leave this in for now maybe we can
        //consider its removal for 5.0 --Sam
	if(qApp && qApp->mainWidget() == q && qApp->windowIcon().isNull()) {
            void qt_mac_set_app_icon(const QPixmap &); //qapplication_mac.cpp
            qt_mac_set_app_icon(pixmap);
        }
#endif
        if(pixmap.isNull()) {
            RemoveWindowProxy(qt_mac_window_for(q));
        } else {
            WindowClass wclass;
            GetWindowClass((WindowPtr)qt_mac_window_for(q), &wclass);
            if(wclass == kDocumentWindowClass)
                SetWindowProxyIcon(qt_mac_window_for(q), qt_mac_create_iconref(pixmap));
        }
    }
}

void QWidget::setWindowIconText(const QString &iconText)
{
    d->topData()->iconText = iconText;
    if(isTopLevel() && !iconText.isEmpty())
        SetWindowAlternateTitle(qt_mac_window_for(this), QCFString(iconText));
    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
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
    QWidget *tlw = topLevelWidget();
    if(!tlw->isVisible() || !tlw->isTopLevel() || tlw->isDesktop())
        return;
    qt_event_remove_activate();
    qt_mac_set_fullscreen_mode((tlw->windowState() & Qt::WindowFullScreen) &&
                               !qApp->desktop()->screenNumber(this));
    WindowPtr window = qt_mac_window_for(tlw);
    if(tlw->isPopup() || tlw->testWFlags(Qt::WStyle_Tool) || qt_mac_is_macdrawer(tlw)) {
        ActivateWindow(window, true);
    } else {
        if(IsWindowActive(window)) {
            ActivateWindow(window, true);
            qApp->setActiveWindow(tlw);
        } else if(!isMinimized()){
            SelectWindow(window);
        }
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
    if(isUpdatesEnabled() && isVisible()) {
        if(w < 0)
            w = data->crect.width()  - x;
        if(h < 0)
            h = data->crect.height() - y;
        if(w && h) {
            QRegion rgn(x, y, w, h);
            HIViewSetNeedsDisplayInRegion((HIViewRef)winId(), rgn.handle(true), true);
        }
    }
}

void QWidget::update(const QRegion &rgn)
{
    if(isUpdatesEnabled() && isVisible())
        HIViewSetNeedsDisplayInRegion((HIViewRef)winId(), rgn.handle(true), true);
}

void QWidget::repaint(const QRegion &rgn)
{
    HIViewSetNeedsDisplayInRegion((HIViewRef)winId(), rgn.handle(true), true);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    OSStatus (*HIViewRender_ptr)(HIViewRef) = HIViewRender; // workaround for gcc warning
    if(HIViewRender_ptr)
        (*HIViewRender_ptr)((HIViewRef)topLevelWidget()->winId()); //yes the top level!!
#endif
}

void QWidgetPrivate::show_sys()
{
    if(q->isDesktop()) //desktop is always visible
        return;

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;

    if(q->isTopLevel()) {
        QDesktopWidget *dsk = QApplication::desktop();
        if(!d->topData()->is_moved && dsk) {
            int movex = q->x(), movey = q->y();
            QRect r = q->frameGeometry();
            QRect avail = dsk->availableGeometry(dsk->screenNumber(q));
                if(r.bottom() > avail.bottom())
                    movey = avail.bottom() - r.height();
                if(r.right() > avail.right())
                    movex = avail.right() - r.width();
                q->move(qMax(avail.left(), movex), qMax(avail.top(), movey));
        }
    }
    data.fstrut_dirty = true;
    if(q->isTopLevel()) {
        WindowPtr window = qt_mac_window_for(q);
        SizeWindow(window, q->width(), q->height(), true);
        if(qt_mac_is_macsheet(q)) {
            qt_event_request_showsheet(q);
        } else if(qt_mac_is_macdrawer(q)) {
            OpenDrawer(window, kWindowEdgeDefault, false);
        } else {
            ShowHide(window, true);
            d->toggleDrawers(true);
        }
        if(q->windowState() & Qt::WindowMinimized) //show in collapsed state
            CollapseWindow(window, true);
        qt_event_request_activate(q);
    } else if(!q->parentWidget() || q->parentWidget()->isVisible()) {
        HIViewSetVisible((HIViewRef)q->winId(), true);
        qt_event_request_window_change();
    }
}

void QWidgetPrivate::hide_sys()
{
    if(q->isDesktop()) //you can't hide the desktop!
        return;

    if(q->isTopLevel()) {
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
            d->toggleDrawers(false);
        }
        if(q->isActiveWindow() && !q->isPopup()) {
            QWidget *w = 0;
            if(q->parentWidget())
                w = q->parentWidget()->topLevelWidget();
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
        HIViewSetVisible((HIViewRef)q->winId(), false);
        qt_event_request_window_change();
    }
    deactivateWidgetCleanup();
    qt_mac_event_release(q);
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Qt::WindowStates oldstate = windowState();

    bool needShow = false;
    if(isTopLevel()) {
        WindowPtr window = qt_mac_window_for(this);
        if((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if(newstate & Qt::WindowFullScreen) {
                if(QTLWExtra *tlextra = d->topData()) {
                    if(tlextra->normalGeometry.width() < 0) {
                        if(testAttribute(Qt::WA_Resized))
                            tlextra->normalGeometry = geometry();
                        else
                            tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
                    }
                    tlextra->savedFlags = getWFlags();
                }
                needShow = isVisible();
                const QRect fullscreen(qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this)));
                setParent(0, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder |
                          (getWFlags() & 0xffff0000));                           // preserve some widget flags
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
                if(QTLWExtra *tlextra = d->topData()) {
                    if(data->fstrut_dirty)
                        d->updateFrameStrut();
                    bounds.left += tlextra->fleft;
                    if(bounds.right < avail.x()+avail.width())
                        bounds.right = qMin((uint)avail.x()+avail.width(), bounds.right+tlextra->fleft);
                    if(bounds.bottom < avail.y()+avail.height())
                        bounds.bottom = qMin((uint)avail.y()+avail.height(), bounds.bottom+tlextra->ftop);
                    bounds.top += tlextra->ftop;
                    bounds.right -= tlextra->fright;
                    bounds.bottom -= tlextra->fbottom;
                }
                QRect orect(geometry().x(), geometry().y(), width(), height()),
                      nrect(bounds.left, bounds.top, bounds.right - bounds.left,
                            bounds.bottom - bounds.top);
                if(orect != nrect) { // no real point..
                    Rect oldr;
                    if(QTLWExtra *tlextra = d->topData())
                        SetRect(&oldr, tlextra->normalGeometry.left(), tlextra->normalGeometry.top(),
                                tlextra->normalGeometry.right()+1, tlextra->normalGeometry.bottom()+1);
                    else
                        SetRect(&oldr, orect.x(), orect.y(), orect.right(), orect.bottom());
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

    data->widget_state &= ~(Qt::WState_Minimized | Qt::WState_Maximized | Qt::WState_FullScreen);
    if(newstate & Qt::WindowMinimized)
        data->widget_state |= Qt::WState_Minimized;
    if(newstate & Qt::WindowMaximized)
        data->widget_state |= Qt::WState_Maximized;
    if(newstate & Qt::WindowFullScreen)
        data->widget_state |= Qt::WState_FullScreen;

    if(needShow)
        show();

    if(newstate & Qt::WindowActive)
        activateWindow();

    qt_event_request_window_change();
    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::raise_sys()
{
    if(q->isDesktop())
        return;
    if(q->isTopLevel()) {
        //raise this window
        BringToFront(qt_mac_window_for(q));
        //we get to be the active process now
        ProcessSerialNumber psn;
        GetCurrentProcess(&psn);
        SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
    } else if(QWidget *p = q->parentWidget()) {
        HIViewSetZOrder((HIViewRef)q->winId(), kHIViewZOrderAbove, 0);
        qt_event_request_window_change();
    }
}

void QWidgetPrivate::lower_sys()
{
    if(q->isDesktop())
        return;
    if(q->isTopLevel()) {
        SendBehind(qt_mac_window_for(q), 0);
    } else if(QWidget *p = q->parentWidget()) {
        HIViewSetZOrder((HIViewRef)q->winId(), kHIViewZOrderBelow, 0);
        qt_event_request_window_change();
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget *w)
{
    if(!w || q->isTopLevel() || q->isDesktop())
        return;

    QWidget *p = q->parentWidget();
    if(!p || p != w->parentWidget())
        return;
    HIViewSetZOrder((HIViewRef)q->winId(), kHIViewZOrderBelow, (HIViewRef)w->winId());
    qt_event_request_window_change();
}

/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to X11's 16bit coordinate system.

  Sets the geometry of the widget to data.crect, but clipped to sizes
  that X can handle. Unmaps widgets that are completely outside the
  valid range.

  Maintains data.wrect, which is the geometry of the X widget,
  measured in this widget's coordinate system.

  if the parent is not clipped, parentWRect is empty, otherwise
  parentWRect is the geometry of the parent's X rect, measured in
  parent's coord sys
*/
void QWidgetPrivate::setWSGeometry()
{

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

        if (data.wrect.isValid()) {
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
                HIViewSetFrame((HIViewRef)q->winId(), &bounds);
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
            HIViewSetVisible((HIViewRef)q->winId(), false);
            q->setAttribute(Qt::WA_Mapped, false);
        } else if (q->isShown()) {
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
            if (!w->isTopLevel())
                w->d->setWSGeometry();
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    //if (jump) //avoid flicker when jumping
    //    XSetWindowBackgroundPixmap(dpy, data.winid, XNone);
    HIRect bounds = CGRectMake(xrect.x(), xrect.y(),
                               xrect.width(), xrect.height());
    HIViewSetFrame((HIViewRef)q->winId(), &bounds);

    if  (jump) {
        updateSystemBackground();
        q->update();
    }
    if (mapWindow) {
        q->setAttribute(Qt::WA_Mapped);
        HIViewSetVisible((HIViewRef)q->winId(), true);
    }
}


void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    if(q->isTopLevel() && isMove)
        d->topData()->is_moved = 1;
    if(q->isDesktop())
        return;
    if(QWExtra *extra = d->extraData()) {        // any size restrictions?
        if(q->isTopLevel()) {
            WindowPtr window = qt_mac_window_for(q);
            qt_mac_update_sizer(q);
            if(q->testWFlags(Qt::WStyle_Maximize)) {
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
        if(QTLWExtra *top = d->topData()) {
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

    if (q->isTopLevel()) {
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QPoint oldp = q->pos();
    QSize  olds = q->size();
    const bool isResize = (olds != QSize(w, h));
    if(!q->isTopLevel() && !isResize && QPoint(x, y) == oldp)
        return;
    if(isResize && q->isMaximized())
        q->clearWState(Qt::WState_Maximized);
    const bool visible = q->isVisible();
    data.crect = QRect(x, y, w, h);

    if(q->isTopLevel()) {
        //update the widget also..
        HIRect bounds = CGRectMake(0, 0, w, h);
        HIViewSetFrame((HIViewRef)q->winId(), &bounds);
        qt_event_request_window_change();

        Rect r; SetRect(&r, x, y, x+w, y+h);
#if 0
        HIViewSetDrawingEnabled((HIViewRef)q->winId(), false);
        SetWindowBounds(qt_mac_window_for(q), kWindowContentRgn, &r);
        HIViewSetDrawingEnabled((HIViewRef)q->winId(), true);
        q->update();
#else
        SetWindowBounds(qt_mac_window_for(q), kWindowContentRgn, &r);
#endif
    } else {
        d->setWSGeometry();
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
}

void QWidget::setMinimumSize(int minw, int minh)
{
    if(minw < 0 || minh < 0)
        qWarning("Qt: QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if(d->extraData()->minw == minw && d->extraData()->minh == minh)
        return;
    d->extraData()->minw = minw;
    d->extraData()->minh = minh;
    if(minw > width() || minh > height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        resize(qMax(minw,width()), qMax(minh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if(maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
        qWarning("Qt: QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 objectName().toLocal8Bit().constData(), metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if(maxw < 0 || maxh < 0) {
        qWarning("Qt: QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                 objectName().toLocal8Bit().constData(), metaObject()->className(), maxw, maxh);
        maxw = qMax(maxw, 0);
        maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if(d->extraData()->maxw == maxw && d->extraData()->maxh == maxh)
        return;
    d->extraData()->maxw = maxw;
    d->extraData()->maxh = maxh;
    if(maxw < width() || maxh < height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        resize(qMin(maxw,width()), qMin(maxh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
    }
    updateGeometry();
}


void QWidget::setSizeIncrement(int w, int h)
{
    d->topData()->incw = w;
    d->topData()->inch = h;
}

void QWidget::setBaseSize(int w, int h)
{
    d->topData()->basew = w;
    d->topData()->baseh = h;
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    bool valid_rect = r.isValid();
    if(!isUpdatesEnabled() &&  (valid_rect || children().isEmpty()))
        return;

    if(!valid_rect) {        // scroll children
        QPoint pd(dx, dy);
        QWidgetList moved;
        QObjectList chldrn = children();
        for(int i = 0; i < chldrn.size(); i++) {  //first move all children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType()) {
                QWidget *w = (QWidget*)obj;
                if(!w->isTopLevel()) {
                    w->data->crect = QRect(w->pos() + pd, w->size());
                    HIRect bounds = CGRectMake(w->data->crect.x(), w->data->crect.y(),
                                               w->data->crect.width(), w->data->crect.height());
                    HIViewSetFrame((HIViewRef)w->winId(), &bounds);
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
    HIRect scrollrect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    HIViewScrollRect((HIViewRef)winId(), valid_rect ? &scrollrect : 0, dx, dy);
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
        HIViewGetFrame((HIViewRef)winId(), &rect);
        if(m == PdmWidth)
            return (int)rect.size.width;
        return (int)rect.size.height; }
    case PdmDepth:
        return 32;
    case PdmDpiX:
    case PdmPhysicalDpiX: {
        short dpix, dpiy;
        ScreenRes(&dpix, &dpiy);
        return dpix; }
    case PdmDpiY:
    case PdmPhysicalDpiY: {
        short dpix, dpiy;
        ScreenRes(&dpix, &dpiy);
        return dpiy; }
    default: //leave this so the compiler complains when new ones are added
        qWarning("Qt: QWidget::metric unhandled parameter %d", m);
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
    extra->topextra->group = 0;
    extra->topextra->is_moved = 0;
    extra->topextra->resizer = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group)
        qt_mac_release_window_group(extra->topextra->group);
}

bool QWidget::acceptDrops() const
{
    return d->macDropEnabled;
}

void QWidgetPrivate::updateFrameStrut() const
{
    QWidgetPrivate *that = const_cast<QWidgetPrivate*>(this);
    if(!data.fstrut_dirty) {
        that->data.fstrut_dirty = q->isVisible();
        return;
    }
    that->data.fstrut_dirty = false;
    QTLWExtra *top = that->topData();
    top->fleft = top->fright = top->ftop = top->fbottom = 0;
    if(!q->isDesktop() && q->isTopLevel()) {
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
        top->fleft = content_r.left - window_r.left;
        top->ftop = content_r.top - window_r.top;
        top->fright = window_r.right - content_r.right;
        top->fbottom = window_r.bottom - window_r.bottom;
    }
}

void QWidget::setAcceptDrops(bool on)
{
    if(on == d->macDropEnabled)
        return;
    d->macDropEnabled = on;
    SetControlDragTrackingEnabled((HIViewRef)winId(), on);
}

void QWidget::setMask(const QRegion &region)
{
    d->createExtra();
    if(region.isEmpty() && d->extraData()->mask.isEmpty())
        return;

    d->extra->mask = region;
    if(isTopLevel())
        ReshapeCustomWindow(qt_mac_window_for(this));
    else
        HIViewReshapeStructure((HIViewRef)winId());
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

void QWidget::resetInputContext()
{
    if (!hasFocus())
        return;
    qt_mac_unicode_reset_input(this);
}

void QWidget::setWindowOpacity(qreal level)
{
    if(!isTopLevel())
        return;

    level = qMin(qMax(level, 0), 1.0);
    QMacSavedPortInfo::setWindowAlpha(this, level);
    d->topData()->opacity = (uchar)(level * 255);
}

qreal QWidget::windowOpacity() const
{
    return isTopLevel() ? ((QWidget*)this)->d->topData()->opacity / 255.0 : 0.0;
}


static QSingleCleanupHandler<QPaintEngine> qt_paintengine_cleanup_handler;
static QPaintEngine *qt_widget_paintengine = 0;
QPaintEngine *QWidget::paintEngine() const
{
    if (!qt_widget_paintengine) {
#if !defined(QMAC_NO_COREGRAPHICS)
        if(!qgetenv("QT_MAC_USE_QUICKDRAW"))
            qt_widget_paintengine = new QCoreGraphicsPaintEngine();
        else
#endif
            qt_widget_paintengine = new QQuickDrawPaintEngine();
        qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
    }
    if (qt_widget_paintengine->isActive()) {
        QPaintEngine *engine =
#if !defined(QMAC_NO_COREGRAPHICS)
        !qgetenv("QT_MAC_USE_QUICKDRAW")
            ? new QCoreGraphicsPaintEngine() :
#endif
            new QQuickDrawPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
    return qt_widget_paintengine;
}


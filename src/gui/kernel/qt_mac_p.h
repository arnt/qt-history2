/****************************************************************************
**
** Definition of ???.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_MAC_H
#define QT_MAC_H

#include <private/qcore_mac_p.h>
#include "qglobal.h"

#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3)
# define QMAC_NO_COREGRAPHICS
#endif

#include "qpainter.h"
#include "qwidget.h"
#include "private/qwidget_p.h"
#ifdef QT_THREAD_SUPPORT
#include "qmutex.h"
#include "qthread.h"
extern QMutex *qt_mac_port_mutex; //qapplication_mac.cpp
#endif

class QMacBlockingFunction //implemented in qeventloop_mac.cpp
{
private:
    class Object;
    static Object *block;
public:
    QMacBlockingFunction();
    ~QMacBlockingFunction();
    static bool blocking() { return block != 0; }
};

#include "qpaintdevice.h"
extern WindowPtr qt_mac_window_for(HIViewRef); //qwidget_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    bool valid_gworld;
    void init();

public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    inline QMacSavedPortInfo(QWidget *w, bool set_clip = false)
        { init(); setPaintDevice(w, set_clip); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRect &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRegion &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    ~QMacSavedPortInfo();
    static bool setClipRegion(const QRect &r);
    static bool setClipRegion(const QRegion &r);
    inline static bool setClipRegion(QWidget *w)
        { return setClipRegion(w->d_func()->clippedRegion()); }
    static bool setPaintDevice(QPaintDevice *);
    static bool setPaintDevice(QWidget *, bool set_clip=false, bool with_child=true);
    static void setWindowAlpha(QWidget *, float);
};

extern "C" {
    typedef struct CGSConnection *CGSConnectionRef;
    typedef struct CGSWindow *CGSWindowRef;
    extern OSStatus CGSSetWindowAlpha(CGSConnectionRef, CGSWindowRef, float);
    extern CGSWindowRef GetNativeWindowFromWindowRef(WindowRef);
    extern CGSConnectionRef _CGSDefaultConnection();
}
inline void
QMacSavedPortInfo::setWindowAlpha(QWidget *w, float l)
{
    CGSSetWindowAlpha(_CGSDefaultConnection(),
                      GetNativeWindowFromWindowRef(qt_mac_window_for((HIViewRef)w->winId())), l);
}

inline bool
QMacSavedPortInfo::setClipRegion(const QRect &rect)
{
    Rect r;
    SetRect(&r, rect.x(), rect.y(), rect.right()+1, rect.bottom()+1);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->lock();
#endif
    ClipRect(&r);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->unlock();
#endif
    return true;
}

inline bool
QMacSavedPortInfo::setClipRegion(const QRegion &r)
{
    if(r.isEmpty())
        return setClipRegion(QRect());
    RgnHandle rgn = r.handle();
    if(!rgn)
        return setClipRegion(r.boundingRect());
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->lock();
#endif
    SetClip(rgn);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->unlock();
#endif
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QWidget *w, bool set_clip, bool with_child)
{
    if (!w)
        return false;
    if(!setPaintDevice((QPaintDevice *)w))
        return false;
    if(set_clip)
        return setClipRegion(w->d_func()->clippedRegion(with_child));
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
    if(!pd)
        return false;
    bool ret = true;
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->lock();
#endif
    extern Qt::HANDLE qt_mac_handle(const QPaintDevice *); // qpaintdevice_mac.cpp
    if(pd->devType() == QInternal::Widget)
        SetPortWindowPort(qt_mac_window_for((HIViewRef)(static_cast<QWidget*>(pd)->winId())));
    else if(pd->devType() == QInternal::Pixmap || pd->devType() == QInternal::Printer)
        SetGWorld((GrafPtr)qt_mac_handle(pd), 0); //set the gworld
    else
        ret = false;
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->unlock();
#endif
    return ret;
}


inline void
QMacSavedPortInfo::init()
{
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->lock();
#endif
    GetBackColor(&back);
    GetForeColor(&fore);
    GetGWorld(&world, &handle);
    valid_gworld = true;
    clip = NewRgn();
    GetClip(clip);
    GetPenState(&pen);
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    bool set_state = false;
    if(valid_gworld) {
        set_state = IsValidPort(world);
        if(set_state)
            SetGWorld(world,handle); //always do this one first
    } else {
        setPaintDevice(qt_mac_safe_pdev);
    }
    if(set_state) {
        SetClip(clip);
        SetPenState(&pen);
        RGBForeColor(&fore);
        RGBBackColor(&back);
    }
    DisposeRgn(clip);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
        qt_mac_port_mutex->unlock();
#endif
}

#endif // QT_MAC_H

#ifndef QT_MAC_H
#define QT_MAC_H

#undef OLD_DEBUG
#ifdef DEBUG
#define OLD_DEBUG DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#ifndef __IMAGECAPTURE__
#define __IMAGECAPTURE__
#endif
#include <Carbon/Carbon.h>
#include <QuickTime/Movies.h>
#include "qglobal.h"

#ifdef Q_WS_MAC9
#include "qt_mac9.h"
#endif

#ifdef Q_WS_MACX
#define QMAC_DEFAULT_STYLE "QAquaStyle"
#else
#define QMAC_DEFAULT_STYLE "QPlatinumStyle"
#endif

#undef DEBUG
#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#endif
#undef OLD_DEBUG

#ifdef Q_WS_MAC
#include <qwidget.h>
extern int mac_window_count; //qwidget_mac.cpp
#ifdef QT_THREAD_SUPPORT
#include <qthread.h>
extern QMutex *qt_mac_port_mutex; //qapplication_mac.cpp
#endif

class QMacBlockingFunction : public QObject //done in qapplication_mac.cpp
{
private:
    static bool block;
public:
    QMacBlockingFunction();
    ~QMacBlockingFunction() { block = FALSE; }
    static bool blocking() { return block; }

protected:
    void timerEvent(QTimerEvent *);
};

class QMacSavedFontInfo 
{
private:
    void init(CGrafPtr);
protected:
    short tfont, tface;
    int tsize;
public:
    inline QMacSavedFontInfo() { GWorldPtr w; GDHandle h; GetGWorld(&w, &h); init(w); }
    inline QMacSavedFontInfo(CGrafPtr w) { init(w); }
    ~QMacSavedFontInfo();
};

inline QMacSavedFontInfo::~QMacSavedFontInfo() 
{
    if(mac_window_count) {
	TextFont(tfont);
	TextFace(tface);
	TextSize(tsize);
    }
}

inline void QMacSavedFontInfo::init(CGrafPtr w) 
{
    if(mac_window_count) {
	tfont = GetPortTextFont(w);
	tface = GetPortTextFace(w);
	tsize = GetPortTextSize(w);
    }
}

#include <qptrlist.h>
#include <qpaintdevice.h>
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp
extern QPainter *qt_mac_current_painter; //qpainter_mac.cpp
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    QMacSavedFontInfo *fi;
    QPainter *painter;
    void init();
    
    static QPtrList<QMacSavedPortInfo> gports;
    bool valid_gworld;
    inline void register_self() { gports.append(this); }
    inline void deregister_self() { gports.remove(this); }
public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRect &r) 
	{ init(); setPaintDevice(pd); setClipRegion(r); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRegion &r) 
	{ init(); setPaintDevice(pd); setClipRegion(r); }
    ~QMacSavedPortInfo();
    static bool setClipRegion(const QRect &r);
    static bool setClipRegion(const QRegion &r);
    static bool setPaintDevice(QPaintDevice *);
    static bool flush(QPaintDevice *);
    static bool flush(QPaintDevice *, const QRegion &r, bool force=FALSE);
    static void setAlphaTransparancy(QWidget *, float);
};

inline bool QMacSavedPortInfo::flush(QPaintDevice *pdev) 
{
#ifdef Q_WS_MACX
    if ( pdev->devType() == QInternal::Widget ) {
	QWidget *w = (QWidget *)pdev;
	if ( !w->isHidden() && QDIsPortBuffered(GetWindowPort((WindowPtr)w->handle()))) {
	    QDFlushPortBuffer(GetWindowPort((WindowPtr)w->handle()), NULL);
	    return TRUE;
	}
    } 
#endif
    return FALSE;
}

inline bool QMacSavedPortInfo::flush(QPaintDevice *pdev, const QRegion &r, bool force) 
{
#ifdef Q_WS_MACX
    if ( pdev->devType() == QInternal::Widget ) {
	QWidget *w = (QWidget *)pdev;
	if ( !w->isHidden() || QDIsPortBuffered(GetWindowPort((WindowPtr)w->handle()))) {
	    QDFlushPortBuffer(GetWindowPort((WindowPtr)w->handle()), r.handle(force));
	    return TRUE;
	}
    } 
#endif
    return FALSE;
}

#ifdef Q_WS_MACX
extern "C" {
    typedef struct CGSConnection *CGSConnectionRef;
    typedef struct CGSWindow *CGSWindowRef;
    extern OSStatus CGSSetWindowAlpha(CGSConnectionRef, CGSWindowRef, float);
    extern CGSWindowRef GetNativeWindowFromWindowRef(WindowRef);
    extern CGSConnectionRef _CGSDefaultConnection();
}
#endif
inline void QMacSavedPortInfo::setAlphaTransparancy(QWidget *w, float l)
{
#ifdef Q_WS_MACX
    CGSSetWindowAlpha(_CGSDefaultConnection(), 
		      GetNativeWindowFromWindowRef((WindowRef)w->handle()), l);

#endif
}

inline bool QMacSavedPortInfo::setClipRegion(const QRect &rect)
{
    Rect r;
    SetRect(&r, rect.x(), rect.y(), rect.right()+1, rect.bottom()+1);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->lock();
#endif
    qt_mac_current_painter = NULL;
    ClipRect(&r);
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->unlock();
#endif
    return TRUE;
}

inline bool QMacSavedPortInfo::setClipRegion(const QRegion &r)
{
    if(r.isNull())
	return setClipRegion(QRect());
    else if(!r.handle())
	return setClipRegion(r.boundingRect());
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->lock();
#endif
    qt_mac_current_painter = NULL;
    SetClip(r.handle());
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->unlock();
#endif
    return TRUE;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->lock();
#endif
    bool ret = TRUE;
    if(!pd)
	return FALSE;
    qt_mac_current_painter = NULL;
    switch(pd->devType()) {
    case QInternal::Printer:
    case QInternal::Pixmap:
	SetGWorld((GrafPtr)pd->handle(), 0); //set the gworld
	break;
    case QInternal::Widget:
	SetPortWindowPort((WindowPtr)pd->handle());
	break;
    default:
	ret = FALSE;
	break;
    }
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->unlock();
#endif
    return ret;
}
    

inline void QMacSavedPortInfo::init()
{
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->lock();
#endif
    fi = NULL;
    painter = qt_mac_current_painter;
    if(mac_window_count) {
   	GetBackColor(&back);
	GetForeColor(&fore);
	GetGWorld(&world, &handle);
	valid_gworld = TRUE;
	register_self();
	fi = new QMacSavedFontInfo(world);
	clip = NewRgn();
	GetClip(clip);
	GetPenState(&pen);
    }
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    deregister_self();
    if(mac_window_count) {
	if(valid_gworld) 
	    SetGWorld(world,handle); //always do this one first
	else
	    setPaintDevice(qt_mac_safe_pdev);
	SetClip(clip);
	DisposeRgn(clip);
	SetPenState(&pen);
	RGBForeColor(&fore);
	RGBBackColor(&back);
    }
    if(fi)
	delete fi;
    qt_mac_current_painter = painter;
#if defined(QT_THREAD_SUPPORT)
    if(qt_mac_port_mutex)
	qt_mac_port_mutex->unlock();
#endif
}

#endif // QT_MAC_H

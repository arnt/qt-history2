#ifndef QT_MAC_H
#define QT_MAC_H

#undef OLD_DEBUG
#ifdef DEBUG
#define OLD_DEBUG DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#include "Carbon.h"
#include "Movies.h"
#include "PMCore.h"
#include "OSUtils.h"

#ifdef Q_WS_MAC9
#include "qt_mac9.h"
#endif

#undef DEBUG
#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#endif
#undef OLD_DEBUG

extern int mac_window_count;

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

#include <qpaintdevice.h>
extern QPaintDevice *g_cur_paintdev;
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    QMacSavedFontInfo *fi;
    void init();
public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    ~QMacSavedPortInfo();
    static bool setPaintDevice(QPaintDevice *);
};

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
    if(pd == g_cur_paintdev || !(g_cur_paintdev = pd))
	return FALSE;
    switch(pd->devType()) {
    case QInternal::Printer:
    case QInternal::Pixmap:
	SetGWorld((GrafPtr)pd->handle(), 0); //set the gworld
	break;
    case QInternal::Widget:
	SetPortWindowPort((WindowPtr)pd->handle());
	break;
    default:
	qDebug("ugh?!");
	return FALSE;
    }
    return TRUE;
}
    

inline void QMacSavedPortInfo::init()
{
    fi = NULL;
    if(mac_window_count) {
	GetBackColor(&back);
	GetForeColor(&fore);
	GetGWorld(&world, &handle);
	fi = new QMacSavedFontInfo(world);
	clip = NewRgn();
	GetClip(clip);
	GetPenState(&pen);
    }
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    setPaintDevice(NULL);
    if(mac_window_count) {
	SetGWorld(world,handle); //always do this one first
	SetClip(clip);
	DisposeRgn(clip);
	SetPenState(&pen);
	RGBForeColor(&fore);
	RGBBackColor(&back);
    }
    if(fi)
	delete fi;
}

#endif // QT_MAC_H

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

extern int mac_window_count; //qwidget_mac.cpp

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

#include <qlist.h>
#include <qpaintdevice.h>
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    QMacSavedFontInfo *fi;
    void init();
    
    static QList<QMacSavedPortInfo> gports;
    bool valid_gworld;
    inline void register_self() { gports.append(this); }
    inline void deregister_self() { gports.remove(this); }
public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    ~QMacSavedPortInfo();
    static bool setPaintDevice(QPaintDevice *);

    static void removingGWorld(const GWorldPtr w);
};

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
    if(!pd)
	return FALSE;
    switch(pd->devType()) {
    case QInternal::Printer:
    case QInternal::Pixmap:
    int x = (int)pd->handle();
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
	int x = (int)world;
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
    int x = (int)world;
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
}

//sanity checks
inline void QMacSavedPortInfo::removingGWorld(const GWorldPtr w) 
{
    if(!gports.count())
        return;
    for(QListIterator<QMacSavedPortInfo> it(gports); it.current(); ++it) {
        int x = (int)(*it)->world;
        int y = (int)w;
        if((*it)->world == w) 
            (*it)->valid_gworld = FALSE;
    }
}            
#endif // QT_MAC_H

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

class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
public:
    QMacSavedPortInfo();
    ~QMacSavedPortInfo();
};

extern int mac_window_count;

inline QMacSavedPortInfo::QMacSavedPortInfo()
{
    if(mac_window_count) {
	GetBackColor(&back);
	GetForeColor(&fore);
	GetGWorld(&world, &handle);
	clip = NewRgn();
	GetClip(clip);
	GetPenState(&pen);
    }
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    if(mac_window_count) {
	SetGWorld(world,handle); //always do this one first
	SetClip(clip);
	DisposeRgn(clip);
	SetPenState(&pen);
	RGBForeColor(&fore);
	RGBBackColor(&back);
    }
}

#endif // QT_MAC_H

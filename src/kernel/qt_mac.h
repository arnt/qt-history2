#ifndef QT_MAC_H
#define QT_MAC_H

#include "Carbon.h"
#include "Movies.h"

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

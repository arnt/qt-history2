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


inline QMacSavedPortInfo::QMacSavedPortInfo()
{
    GetBackColor(&back);
    GetForeColor(&fore);
    GetGWorld(&world, &handle);
    clip = NewRgn();
    GetClip(clip);
    GetPenState(&pen);
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    SetGWorld(world,handle);
    SetClip(clip);
    DisposeRgn(clip);
    SetPenState(&pen);
    RGBForeColor(&fore);
    RGBBackColor(&back);
}

#endif // QT_MAC_H

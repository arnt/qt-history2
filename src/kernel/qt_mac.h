#ifndef QT_MACH_H

#ifdef QTDEBUG
#warning "Guess I didn't realize this would ever happen.."
#endif

#if defined(DEBUG)
#    define QTDEBUG DEBUG
#    undef DEBUG
#    define DEBUG 0
#endif

#include "/System/Library/Frameworks/Carbon.framework/Headers/Carbon.h"
#include "/System/Library/Frameworks/QuickTime.framework/Headers/Movies.h"

#if defined (QTDEBUG)
#    undef DEBUG
#    define DEBUG QTDEBUG
#    undef QTDEBUG
#endif

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

#endif




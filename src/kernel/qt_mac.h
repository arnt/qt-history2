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
public:
    QMacSavedPortInfo();
    ~QMacSavedPortInfo();
};


inline QMacSavedPortInfo::QMacSavedPortInfo()
{
    GetGWorld(&world, &handle);
    clip = NewRgn();
    GetClip(clip);
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    SetGWorld(world,handle);
    SetClip(clip);
    DisposeRgn(clip);
}

#endif




#ifndef QT_MACH_H

#if defined(DEBUG)
#    define QTDEBUG
#    undef DEBUG
#    define DEBUG 0
#endif

#include "/System/Library/Frameworks/Carbon.framework/Headers/Carbon.h"
#include "/System/Library/Frameworks/QuickTime.framework/Headers/Movies.h"

#if defined (QTDEBUG)
#    undef DEBUG
#    define DEBUG
#endif

#endif




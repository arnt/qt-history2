#ifndef QT_MAC9_H
#define QT_MAC9_H

#include "qglobal.h"
#ifdef Q_WS_MAC9

//these don't belong here, but will be moved to an
//appropriate place later
//features
#ifndef QT_NO_IMAGEIO_JPEG
#define QT_NO_IMAGEIO_JPEG
#endif
#ifndef QT_NO_IMAGEIO_MNG
#define QT_NO_IMAGEIO_MNG
#endif
#ifndef QT_NO_SQL
#define QT_NO_SQL
#endif
#define QT_FATAL_ASSERT
#define QT_NO_XINERAMA
#define QT_NO_OPENGL
#define QT_NO_STYLE_WINDOWSXP 

//mac thingy
#ifndef QMAC_ONE_PIXEL_LOCK
#define QMAC_ONE_PIXEL_LOCK
#endif
//carbon things
#define ALLOW_OLD_CARBON
#define _EVENT_HANDLERS 0
#define ALLOW_OLD_CREATE_FOO_CONTROL_PARAMETERS 0
#define CARBON_ON_MAC_O 1
#define ALLOW_OLD_BLOCKING_APIS 0

//hacks to work around wchar problems
#define __WCHARTDEF__
//#define __NO_WIDE_CHAR 1
#undef wchar_t
typedef unsigned short hide_wchar_t; 
#define wchar_t hide_wchar_t

#include <qcstring.h> //pull in some string stuff
#define strcasecmp(x, y) qstricmp(x, y)
#define strncasecmp(x, y, n) qstrnicmp(x, y, n) 
#define strdup(x) qstrdup(x)

#include <stddef.h>
#include <cstdlib> //pull in some posix stuff
using std::calloc;
using std::free;
using std::realloc;

//hacks to work around malloc(0) returning NULL
static inline void *_mac9_malloc(size_t s) { return std::malloc(s ? s : 1); }
#define malloc(x) _mac9_malloc(x)

#endif //MAC9

#endif //MAC9_H

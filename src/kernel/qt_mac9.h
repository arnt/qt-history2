#ifndef QT_MAC9_H
#define QT_MAC9_H

#include "qglobal.h"
#ifdef Q_WS_MAC9

//these don't belong here, but will be moved to an
//appropriate place later
#ifndef QT_NO_IMAGEIO_JPEG
#define QT_NO_IMAGEIO_JPEG
#endif
#ifndef QT_NO_IMAGEIO_MNG
#define QT_NO_IMAGEIO_MNG
#endif
#ifndef QT_NO_IMAGEIO_PNG
#define QT_NO_IMAGEIO_PNG
#endif
#ifndef QT_NO_SQL
#define QT_NO_SQL
#endif

//hacks to work around wchar problems
#undef wchar_t
typedef unsigned short hide_wchar_t; 
#define wchar_t hide_wchar_t

#include <qcstring.h> //pull in some string stuff
#define strcasecmp(x, y) qstricmp(x, y)
#define strncasecmp(x, y, n) qstrnicmp(x, y, n) 
#define strdup(x) qstrdup(x)

#include <cstdlib> //pull in some posix stuff
using namespace std;

//hacks to work around malloc(0) returning NULL
static inline void *_mac9_malloc(size_t s) { return malloc(s ? s : 1); }
#define malloc(x) _mac9_malloc(x)

#endif //MAC9

#endif //MAC9_H
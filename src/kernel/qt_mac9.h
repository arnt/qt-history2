#include "qglobal.h"
#ifdef Q_WS_MAC9

#ifndef QT_MAC9_H
#define QT_MAC9_H

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
#undef wchar_t
typedef unsigned short hide_wchar_t;
#define wchar_t hide_wchar_t

#include <qcstring.h>
#define strcasecmp(x, y) qstricmp(x, y)
#define strncasecmp(x, y, n) qstrnicmp(x, y, n) 
#include <cstdlib>
using namespace std;

#endif //MAC9_H

#endif //MAC9
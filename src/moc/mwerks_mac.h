#ifndef QT_MOC9_H
#define QT_MOC9_H

#include "qglobal.h"
#ifdef Q_WS_MAC9

//these don't belong here, need a way to access this outside .h files
#define QT_NO_CODECS
#define QT_LITE_UNICODE
#define MOC_MWERKS_PLUGIN 1 /*make moc a plugin*/

//get the qt mac9 stuff
#include "qt_mac9.h"

#endif
#endif
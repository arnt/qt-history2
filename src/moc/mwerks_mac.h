#ifndef QT_MOC9_H
#define QT_MOC9_H

#include "qglobal.h"
#ifdef Q_WS_MAC9

//these don't belong here, need a way to access this outside .h files
#define QT_NO_CODECS
#define QT_LITE_UNICODE


/*make moc a plugin*/
#define MOC_MWERKS_PLUGIN 1 
enum moc_status {
    moc_success = 1,
    moc_parse_error = 2,
    moc_no_qobject = 3,
    moc_not_time = 4,
    moc_no_source = 5,
    moc_general_error = 6
};

//get the qt mac9 stuff
#include "qt_mac9.h"

#endif
#endif
#ifndef UILIB_GLOBAL_H
#define UILIB_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_UILIB_LIBRARY
# define QT_UILIB_EXPORT
#else
# define QT_UILIB_EXPORT
#endif
#else
#define QT_UILIB_EXPORT
#endif

#endif // UILIB_GLOBAL_H



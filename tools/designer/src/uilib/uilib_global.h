#ifndef UILIB_GLOBAL_H
#define UILIB_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_UILIB_LIBRARY
# define QT_UILIB_EXPORT __declspec(dllexport)
#else
# define QT_UILIB_EXPORT __declspec(dllimport)
#endif
#else
#define QT_UILIB_EXPORT
#endif

#endif // UILIB_GLOBAL_H



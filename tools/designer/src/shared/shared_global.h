#ifndef SHARED_GLOBAL_H
#define SHARED_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_SHARED_LIBRARY
# define QT_SHARED_EXPORT __declspec(dllexport)
#else
# define QT_SHARED_EXPORT __declspec(dllimport)
#endif
#else
#define QT_SHARED_EXPORT
#endif

#endif // SHARED_GLOBAL_H


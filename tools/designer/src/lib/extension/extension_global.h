#ifndef EXTENSION_GLOBAL_H
#define EXTENSION_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_EXTENSION_LIBRARY
# define QT_EXTENSION_EXPORT __declspec(dllexport)
#else
# define QT_EXTENSION_EXPORT __declspec(dllimport)
#endif
#else
#define QT_EXTENSION_EXPORT
#endif

#endif // EXTENSION_GLOBAL_H


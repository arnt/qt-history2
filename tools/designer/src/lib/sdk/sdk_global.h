#ifndef SDK_GLOBAL_H
#define SDK_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_SDK_LIBRARY
# define QT_SDK_EXPORT __declspec(dllexport)
#else
# define QT_SDK_EXPORT __declspec(dllimport)
#endif
#else
#define QT_SDK_EXPORT
#endif

#endif // SDK_GLOBAL_H


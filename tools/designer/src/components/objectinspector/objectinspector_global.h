#ifndef OBJECTINSPECTOR_GLOBAL_H
#define OBJECTINSPECTOR_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_OBJECTINSPECTOR_LIBRARY
# define QT_OBJECTINSPECTOR_EXPORT __declspec(dllexport)
#else
# define QT_OBJECTINSPECTOR_EXPORT __declspec(dllimport)
#endif
#else
#define QT_OBJECTINSPECTOR_EXPORT
#endif

#endif // OBJECTINSPECTOR_GLOBAL_H



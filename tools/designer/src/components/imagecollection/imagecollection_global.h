#ifndef IMAGECOLLECTION_GLOBAL_H
#define IMAGECOLLECTION_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_IMAGECOLLECTION_LIBRARY
# define QT_IMAGECOLLECTION_EXPORT __declspec(dllexport)
#else
# define QT_IMAGECOLLECTION_EXPORT __declspec(dllimport)
#endif
#else
#define QT_IMAGECOLLECTION_EXPORT
#endif

#endif // IMAGECOLLECTION_GLOBAL_H


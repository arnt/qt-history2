#ifndef PROPERTYEDITOR_GLOBAL_H
#define PROPERTYEDITOR_PGLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_PROPERTYEDITOR_LIBRARY
# define QT_PROPERTYEDITOR_EXPORT __declspec(dllexport)
#else
# define QT_PROPERTYEDITOR_EXPORT __declspec(dllimport)
#endif
#else
#define QT_PROPERTYEDITOR_EXPORT
#endif

#endif // PROPERTYEDITOR_PGLOBAL_H



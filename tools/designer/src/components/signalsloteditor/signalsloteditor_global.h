#ifndef SIGNALSLOTEDITOR_GLOBAL_H
#define SIGNALSLOTEDITOR_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_SIGNALSLOTEDITOR_LIBRARY
# define QT_SIGNALSLOTEDITOR_EXPORT __declspec(dllexport)
#else
# define QT_SIGNALSLOTEDITOR_EXPORT __declspec(dllimport)
#endif
#else
#define QT_SIGNALSLOTEDITOR_EXPORT
#endif

#endif // SIGNALSLOTEDITOR_GLOBAL_H

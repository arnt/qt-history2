#ifndef DLLDEFS_H
#define DLLDEFS_H

#include <qglobal.h>

#if defined(Q_OS_WIN32)
#  if defined(EDITOR_DLL)
#    define EDITOR_EXPORT __declspec(dllexport)
#  else
#    define EDITOR_EXPORT __declspec(dllimport)
#  endif
#else
#  define EDITOR_EXPORT
#endif

#endif //DLLDEFS_H

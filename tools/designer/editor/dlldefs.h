#ifndef DLLDEFS_H
#define DLLDEFS_H

#include <qglobal.h>

#if defined(Q_OS_WIN32)
#  if defined(CPP_DLL)
#    define CPP_EXPORT __declspec(dllexport)
#  else
#    define CPP_EXPORT __declspec(dllimport)
#  endif
#else
#  define CPP_EXPORT
#endif

#endif //DLLDEFS_H

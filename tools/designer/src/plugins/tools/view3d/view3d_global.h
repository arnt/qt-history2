#ifndef VIEW3D_GLOBAL_H
#define VIEW3D_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef VIEW3D_LIBRARY
# define VIEW3D_EXPORT
#else
# define VIEW3D_EXPORT
#endif
#else
#define VIEW3D_EXPORT
#endif

#endif // VIEW3D_GLOBAL_H

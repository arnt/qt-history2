/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

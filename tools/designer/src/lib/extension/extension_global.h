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

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

#ifndef UILIB_GLOBAL_H
#define UILIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_UILIB_LIBRARY
# define QT_UILIB_EXPORT __declspec(dllexport)
#else
# define QT_UILIB_EXPORT __declspec(dllimport)
#endif
#else
#define QT_UILIB_EXPORT
#endif

#endif // UILIB_GLOBAL_H

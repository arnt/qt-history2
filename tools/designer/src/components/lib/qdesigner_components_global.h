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

#ifndef QDESIGNER_COMPONENTS_GLOBAL_H
#define QDESIGNER_COMPONENTS_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QDESIGNER_COMPONENTS_LIBRARY
# define QDESIGNER_COMPONENTS_EXPORT __declspec(dllexport)
#else
# define QDESIGNER_COMPONENTS_EXPORT __declspec(dllimport)
#endif
#else
#define QDESIGNER_COMPONENTS_EXPORT
#endif

#endif // QDESIGNER_COMPONENTS_GLOBAL_H

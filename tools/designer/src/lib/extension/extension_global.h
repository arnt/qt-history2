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

#include <QtCore/qglobal.h>

#define QT_EXTENSION_EXTERN Q_DECL_EXPORT
#define QT_EXTENSION_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_COMPONENTS_EXPORT
#elif QT_EXTENSION_LIBRARY
#  define QT_EXTENSION_EXPORT QT_EXTENSION_EXTERN
#else
#  define QT_EXTENSION_EXPORT QT_EXTENSION_IMPORT
#endif

#endif // EXTENSION_GLOBAL_H

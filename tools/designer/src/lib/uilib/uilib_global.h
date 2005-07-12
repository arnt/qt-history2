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

#define QT_UILIB_EXTERN Q_DECL_EXPORT
#define QT_UILIB_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_COMPONENTS_EXPORT
#elif QT_UILIB_LIBRARY
#  define QT_UILIB_EXPORT QT_UILIB_EXTERN
#else
#  define QT_UILIB_EXPORT QT_UILIB_IMPORT
#endif

#endif // UILIB_GLOBAL_H

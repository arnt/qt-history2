/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef UILIB_GLOBAL_H
#define UILIB_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#define QDESIGNER_UILIB_EXTERN Q_DECL_EXPORT
#define QDESIGNER_UILIB_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_UILIB_EXPORT
#elif defined(QDESIGNER_UILIB_LIBRARY)
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_EXTERN
#else
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_IMPORT
#endif

QT_END_HEADER

#endif // UILIB_GLOBAL_H

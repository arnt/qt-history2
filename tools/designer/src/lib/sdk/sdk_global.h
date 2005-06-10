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

#ifndef SDK_GLOBAL_H
#define SDK_GLOBAL_H

#include <QtCore/qglobal.h>

#define QT_SDK_EXTERN Q_DECL_EXPORT
#define QT_SDK_IMPORT Q_DECL_IMPORT

#ifdef QT_SDK_LIBRARY
#  define QT_SDK_EXPORT QT_SDK_EXTERN
#else
#  define QT_SDK_EXPORT QT_SDK_IMPORT
#endif

#endif // SDK_GLOBAL_H

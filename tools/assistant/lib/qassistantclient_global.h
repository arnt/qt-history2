/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QASSISTANTCLIENT_GLOBAL_H
#define QASSISTANTCLIENT_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#if defined(QT_ASSISTANT_CLIENT_STATIC) || !defined(QT_DLL)
#  define QT_ASSISTANT_CLIENT_EXPORT
#elif defined(QT_ASSISTANT_CLIENT_LIBRARY)
#  define QT_ASSISTANT_CLIENT_EXPORT Q_DECL_EXPORT
#else
#  define QT_ASSISTANT_CLIENT_EXPORT Q_DECL_IMPORT
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif

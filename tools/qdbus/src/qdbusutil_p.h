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

#ifndef QDBUSUTIL_H
#define QDBUSUTIL_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER


namespace QDBusUtil
{
    QDBUS_EXPORT bool isValidInterfaceName(const QString &ifaceName);

    QDBUS_EXPORT bool isValidUniqueConnectionName(const QString &busName);

    QDBUS_EXPORT bool isValidBusName(const QString &busName);

    QDBUS_EXPORT bool isValidMemberName(const QString &memberName);

    QDBUS_EXPORT bool isValidErrorName(const QString &errorName);

    QDBUS_EXPORT bool isValidObjectPath(const QString &path);

    QDBUS_EXPORT bool isValidSignature(const QString &signature);

    QDBUS_EXPORT bool isValidSingleSignature(const QString &signature);
}

QT_END_HEADER

#endif

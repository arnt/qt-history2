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

#include "qdbusextratypes.h"
#include "qdbusutil_p.h"

void QDBusObjectPath::check()
{
    if (!QDBusUtil::isValidObjectPath(value))
        value.clear();
}

void QDBusSignature::check()
{
    if (!QDBusUtil::isValidSignature(value))
        value.clear();
}

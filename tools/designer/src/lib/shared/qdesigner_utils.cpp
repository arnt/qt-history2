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

#include "qdesigner_utils_p.h"

namespace qdesigner_internal
{
    QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message)
    {
        QString prefixedMessage = QLatin1String("Designer: ");
        prefixedMessage += message;
        qWarning(prefixedMessage.toUtf8().constData());
    }
} // namespace qdesigner_internal

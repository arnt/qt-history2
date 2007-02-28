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

#include "iconloader_p.h"

#include <QtCore/QFile>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

namespace qdesigner_internal {

QDESIGNER_SHARED_EXPORT QIcon createIconSet(const QString &name)
{
    QStringList candidates = QStringList()
        << (QString::fromUtf8(":/trolltech/formeditor/images/") + name)
#ifdef Q_WS_MAC
        << (QString::fromUtf8(":/trolltech/formeditor/images/mac/") + name)
#else
        << (QString::fromUtf8(":/trolltech/formeditor/images/win/") + name)
#endif
        << (QString::fromUtf8(":/trolltech/formeditor/images/designer_") + name);

    foreach (QString f, candidates) {
        if (QFile::exists(f))
            return QIcon(f);
    }

    return QIcon();
}

QDESIGNER_SHARED_EXPORT QIcon emptyIcon()
{
    static const QIcon empty_icon(QLatin1String(":/trolltech/formeditor/images/emptyicon.png"));
    return  empty_icon;
} // namespace qdesigner_internal
}


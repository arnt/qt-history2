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

#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QIcon>
#include <QPixmap>
#include <QString>

inline QIcon createIconSet(const QString &name)
{
    QPixmap pix(QString::fromUtf8(":/trolltech/formeditor/images/") + name);

    if (!pix.isNull())
        return pix;

    const QString ResouceString =
#ifdef Q_WS_MAC
        QLatin1String(":/trolltech/formeditor/images/mac/");
#else
        QLatin1String(":/trolltech/formeditor/images/win/");
#endif
    pix = QPixmap(ResouceString + name);
    if (!pix.isNull())
        return pix;

    return QPixmap(QString::fromUtf8(":/trolltech/formeditor/images/designer_") + name);
}

#endif // ICONLOADER_H

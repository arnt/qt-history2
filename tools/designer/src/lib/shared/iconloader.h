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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QtCore/QString>
#include <QtCore/QFile>

#include <QtGui/QIcon>
#include <QtGui/QPixmap>

inline QIcon createIconSet(const QString &name)
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

#endif // ICONLOADER_H

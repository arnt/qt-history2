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

#ifndef QFACTORYLOADER_P_H
#define QFACTORYLOADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qobject.h"
#include "QtCore/qstringlist.h"
#include "private/qlibrary_p.h"

#ifndef QT_NO_LIBRARY

QT_BEGIN_NAMESPACE

class QFactoryLoaderPrivate;

class Q_CORE_EXPORT QFactoryLoader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFactoryLoader)

public:
    QFactoryLoader(const char *iid,
                   const QStringList &paths = QStringList(),
                   const QString &suffix = QString(),
                   Qt::CaseSensitivity = Qt::CaseSensitive);
    ~QFactoryLoader();

    QStringList keys() const;
    QObject *instance(const QString &key) const;

};

QT_END_NAMESPACE

#endif // QT_NO_LIBRARY

#endif // QFACTORYLOADER_P_H

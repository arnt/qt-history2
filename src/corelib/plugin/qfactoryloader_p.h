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

#include <qobject.h>
#include <qstringlist.h>
#include "qlibrary_p.h"

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

#endif // QFACTORYLOADER_P_H

/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFACTORYLOADER_P_H
#define QFACTORYLOADER_P_H

#ifndef QT_H
#include <qobject.h>
#include <qmap.h>
#include <qstringlist.h>
#include "qlibrary_p.h"
#endif // QT_H
class QFactoryLoaderPrivate;

class Q_CORE_EXPORT QFactoryLoader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFactoryLoader)

public:
    QFactoryLoader(const char *iid,
                   const QStringList &paths = QString(),
                   const QString &suffix = QString(),
                   CaseSensitivity = CaseSensitive,
                   QObject *parent = 0);
    ~QFactoryLoader();

    QStringList keys() const;
    QObject *instance(const QString &key) const;

};

#endif // QFACTORYLOADER_P_H

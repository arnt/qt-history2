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
#include "q4library_p.h"
#endif // QT_H

class QFactoryLoader : public QObject
{
    Q_OBJECT
public:
    QFactoryLoader(const char *iid,
                   const QStringList &paths = QString(),
                   const QString &suffix = QString(),
                   QObject *parent = 0);
    ~QFactoryLoader();

    QStringList keys() const;
    void *create(const QString &key) const;

private:
    QList<Q4LibraryPrivate*> libraryList;
    QMap<QString,Q4LibraryPrivate*> keyMap;
};

#endif // QFACTORYLOADER_P_H

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

#ifndef QRESOURCE_H
#define QRESOURCE_H

#include <QtCore/qlist.h>

class QMetaResource;
class QMetaResourcePrivate;
class QResourcePrivate;

void qInitResourceIO();

class Q_CORE_EXPORT QResource {
public:
    QString name() const;

    uint size() const;
    const uchar *data() const;

    bool isContainer() const;
    QResource *child(const QString &name) const;
    QList<QResource *> children() const;
    const QResource *parent() const;

    static QResource *find(const QString &resource);
    static void addSearchPath(const QString &path);

private:
    friend struct QResourceNode;
    friend class QMetaResource;
    friend class QMetaResourcePrivate;
    QResource();
    ~QResource();

    QResourcePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QResource)

};

/* Don't use this */
class Q_CORE_EXPORT QMetaResource {
private:
    QMetaResourcePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QMetaResource)

public:
    QMetaResource(const uchar *resource);
    ~QMetaResource();
};

#endif // QRESOURCE_H

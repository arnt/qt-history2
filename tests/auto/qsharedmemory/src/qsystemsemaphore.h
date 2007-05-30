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

#ifndef QSYSTEMSEMAPHORE_H
#define QSYSTEMSEMAPHORE_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

#ifndef QT_NO_SYSTEMSEMAPHORE

class QSystemSemaphorePrivate;

class QSystemSemaphore
{

public:
    enum OpenMode
    {
        Open,
        Create
    };

    QSystemSemaphore(const QString &key, int initialValue = 0, OpenMode mode = Open);
    ~QSystemSemaphore();

    void setKey(const QString &key, int initialValue = 0, OpenMode mode = Open);
    QString key() const;

    bool acquire();
    bool release(int n = 1);

private:
    Q_DISABLE_COPY(QSystemSemaphore)
    QSystemSemaphorePrivate *d;
};

#endif // QT_NO_SYSTEMSEMAPHORE

QT_END_HEADER

#endif // QSYSTEMSEMAPHORE_H


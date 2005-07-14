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

#ifndef QOBJECTCLEANUPHANDLER_H
#define QOBJECTCLEANUPHANDLER_H

#include "QtCore/qobject.h"

QT_MODULE(Core)

class Q_CORE_EXPORT QObjectCleanupHandler : public QObject
{
    Q_OBJECT

public:
    QObjectCleanupHandler();
    ~QObjectCleanupHandler();

    QObject* add(QObject* object);
    void remove(QObject *object);
    bool isEmpty() const;
    void clear();

private:
    // ### move into d pointer
    QObjectList cleanupObjects;

private slots:
    void objectDestroyed(QObject *);
};

#endif // QOBJECTCLEANUPHANDLER_H

/****************************************************************************
**
** Definition of ???.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QOBJECTCLEANUPHANDLER_H
#define QOBJECTCLEANUPHANDLER_H

#ifndef QT_H
#include "qlist.h"
#include "qobject.h"
#endif // QT_H

#define Q_DEFINED_QOBJECTLIST
#include "qwinexport.h"

class Q_EXPORT QObjectCleanupHandler : public QObject
{
    Q_OBJECT

public:
    QObjectCleanupHandler();
    ~QObjectCleanupHandler();

    QObject* add( QObject* object );
    void remove( QObject *object );
    bool isEmpty() const;
    void clear();

private:
    // ### move into d pointer
    QObjectList cleanupObjects;

private slots:
    void objectDestroyed( QObject * );
};

#endif // QOBJECTCLEANUPHANDLER_H

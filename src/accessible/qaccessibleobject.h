/****************************************************************************
**
** Definition of the QAccessibleObject class.
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

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#ifndef QT_H
#include "qaccessible.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleObjectPrivate;
class QObject;

class Q_EXPORT QAccessibleObject : public Qt, public QAccessibleInterface
{
public:
    QAccessibleObject( QObject *object );
    virtual ~QAccessibleObject();

    QRESULT	queryInterface( const QUuid &, QUnknownInterface** );
    Q_REFCOUNT

    bool	isValid() const;
    QObject	*object() const;

private:
    QAccessibleObjectPrivate *d;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEOBJECT_H

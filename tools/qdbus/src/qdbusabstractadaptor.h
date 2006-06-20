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

#ifndef QDBUSABSTRACTADAPTOR_H
#define QDBUSABSTRACTADAPTOR_H

#include <QtCore/qobject.h>
#include "qdbusmacros.h"

class QDBusAbstractAdaptorPrivate;
class QDBUS_EXPORT QDBusAbstractAdaptor: public QObject
{
    Q_OBJECT
protected:
    QDBusAbstractAdaptor(QObject *parent);

public:
    ~QDBusAbstractAdaptor();

protected:
    void setAutoRelaySignals(bool enable);

private:
    Q_DECLARE_PRIVATE(QDBusAbstractAdaptor)
};

#endif

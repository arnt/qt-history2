/****************************************************************************
**
** Definition of QIODevice related classes
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIODEVICE_P_H
#define QIODEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QIODevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qobjectdefs.h"
#endif // QT_H

class Q_CORE_EXPORT QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QIODevice);

protected:
    QIODevicePrivate()
        : q_ptr(0), ioMode(0), ioSt(QIODevice::Ok) {}
    virtual ~QIODevicePrivate();

    QIODevice *q_ptr;

private:
    int ioMode;
    int ioSt;
    QString errStr;
};

#endif // QIODEVICE_P_H

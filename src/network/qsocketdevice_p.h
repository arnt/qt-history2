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

#ifndef QSOCKETDEVICE_P_H
#define QSOCKETDEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QSocketDevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qobjectdefs.h"
#include "private/qiodevice_p.h"
#include "private/qioengine_p.h"
#endif // QT_H

class QSocketDevicePrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QSocketDevice)

protected:
    friend class QSocketDeviceEngine;
    QSocketDevicePrivate()
        : fd(-1), t(QSocketDevice::Stream), p(0), pp(0), e(QSocketDevice::NoError),
          protocol(QSocketDevice::IPv4), socketEngine(0) {}
    ~QSocketDevicePrivate();

private:
    enum Option {
#if 0
        // not used anywhere
        Broadcast,
#endif
        ReceiveBuffer,
        ReuseAddress,
        SendBuffer
    };

    int option(Option) const;
    void setOption(Option, int);
    void fetchConnectionParameters() const;
#if defined(Q_OS_WIN32)
    void fetchPeerConnectionParameters() const;
#endif

    int createNewSocket();
    QSocketDevice::Protocol getProtocol() const;

    // the mutable variables are computed lazily
    int fd;
    QSocketDevice::Type t;
    mutable Q_UINT16 p;
    mutable QHostAddress a;
    mutable Q_UINT16 pp;
    mutable QHostAddress pa;
    mutable QSocketDevice::Error e;
    mutable QSocketDevice::Protocol protocol;

    static void init();

    mutable QSocketDeviceEngine *socketEngine;
};

class QSocketDeviceEnginePrivate : public QIOEnginePrivate
{
    Q_DECLARE_PUBLIC(QSocketDeviceEngine)
protected:
    mutable QSocketDevice *device;
};

#if !defined(Q_OS_WIN32)
inline void QSocketDevicePrivate::init() {}
#endif

#endif

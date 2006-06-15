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

#ifndef QNETWORKINTERFACEPRIVATE_H
#define QNETWORKINTERFACEPRIVATE_H

#include <QtCore/qatomic.h>
#include <QtCore/qlist.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qhostaddress.h>

class QNetworkAddressEntryPrivate
{
public:
    QHostAddress address;
    QHostAddress netmask;
    QHostAddress broadcast;
};

class QNetworkInterfacePrivate
{
public:
    QNetworkInterfacePrivate() : ref(1), index(0), flags(0)
    { }
    ~QNetworkInterfacePrivate()
    { }

    QAtomic ref;
    int index;                  // interface index, if know
    QNetworkInterface::InterfaceFlags flags;

    QString name;
    QString hardwareAddress;

    QList<QNetworkAddressEntry> addressEntries;

    static QString makeHwAddress(int len, uchar *data);
};

class QNetworkInterfaceManager
{
public:
    QNetworkInterfaceManager();
    ~QNetworkInterfaceManager();

    QNetworkInterfacePrivate* interfaceFromName(const QString &name);
    QNetworkInterfacePrivate* interfaceFromIndex(int index);
    QList<QNetworkInterfacePrivate *> allInterfaces();

    // convenience:
    QNetworkInterfacePrivate empty;

private:
    QReadWriteLock lock;
    QList<QNetworkInterfacePrivate *> interfaceList;

    void scan();
};


#endif

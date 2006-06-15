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

class QNetworkInterfacePrivate: public QSharedData
{
public:
    QNetworkInterfacePrivate() : index(0), flags(0)
    { }
    ~QNetworkInterfacePrivate()
    { }

    int index;                  // interface index, if know
    QNetworkInterface::InterfaceFlags flags;

    QString name;
    QString hardwareAddress;

    QList<QNetworkAddressEntry> addressEntries;

    static QString makeHwAddress(int len, uchar *data);

private:
    // disallow copying -- avoid detaching
    QNetworkInterfacePrivate &operator=(const QNetworkInterfacePrivate &other);
    QNetworkInterfacePrivate(const QNetworkInterfacePrivate &other);
};

class QNetworkInterfaceManager
{
public:
    QNetworkInterfaceManager();
    ~QNetworkInterfaceManager();

    QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromName(const QString &name);
    QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromIndex(int index);
    QList<QSharedDataPointer<QNetworkInterfacePrivate> > allInterfaces();

    // convenience:
    QNetworkInterfacePrivate empty;

private:
    QList<QNetworkInterfacePrivate *> scan();
};


#endif

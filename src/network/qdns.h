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

#ifndef QDNS_H
#define QDNS_H

#include <QtCore/qlist.h>
#include <QtNetwork/qhostaddress.h>

class QObject;
class QDnsHostInfo;

class Q_NETWORK_EXPORT QDns
{
public:
    static void getHostByName(const QString &name, QObject *receiver, const char *member);
    static QDnsHostInfo getHostByName(const QString &name);
    static QString getHostName();
};

class QDnsHostInfoPrivate;
class Q_NETWORK_EXPORT QDnsHostInfo
{
public:
    QDnsHostInfo();
    QDnsHostInfo(const QDnsHostInfo &d);
    QDnsHostInfo &operator=(const QDnsHostInfo &d);
    ~QDnsHostInfo();

    QString host() const;
    QList<QHostAddress> addresses() const;

    enum Error { NoError, HostNotFound, UnknownError };

    Error error() const;
    QString errorString() const;

    friend class QDnsAgent;
    friend void QDns::getHostByName(const QString &, QObject *, const char *);
    friend QDnsHostInfo QDns::getHostByName(const QString &);

private:
    QDnsHostInfoPrivate *d;
};

#endif

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

#ifndef QHOSTINFO_H
#define QHOSTINFO_H

#include <QtCore/qlist.h>
#include <QtNetwork/qhostaddress.h>

QT_MODULE(Network)

class QObject;
class QHostInfoPrivate;

class Q_NETWORK_EXPORT QHostInfo
{
public:
    enum HostInfoError {
        NoError,
        HostNotFound,
        UnknownError
    };

    QHostInfo(int lookupId = -1);
    QHostInfo(const QHostInfo &d);
    QHostInfo &operator=(const QHostInfo &d);
    ~QHostInfo();

    QString hostName() const;
    void setHostName(const QString &name);

    QList<QHostAddress> addresses() const;
    void setAddresses(const QList<QHostAddress> &addresses);

    HostInfoError error() const;
    void setError(HostInfoError error);

    QString errorString() const;
    void setErrorString(const QString &errorString);

    void setLookupId(int id);
    int lookupId() const;

    static int lookupHost(const QString &name, QObject *receiver, const char *member);
    static void abortHostLookup(int lookupId);

    static QHostInfo fromName(const QString &name);
    static QString localHostName();

private:
    QHostInfoPrivate *d;
};

#endif // QHOSTINFO_H

#ifndef QDNS_H
#define QDNS_H

#include <qobject.h>
#include <qhostaddress.h>
#include <qlist.h>

class Q_NETWORK_EXPORT QDns
{
public:
    static void getHostByName(const QString &name,
                              QObject *receiver, const char *member);
};

class Q_NETWORK_EXPORT QDnsHostInfo
{
public:
    QDnsHostInfo();
    QDnsHostInfo(const QDnsHostInfo &d);

    inline QList<QHostAddress> addresses() const { return addrs; }

    enum Error { NoError, HostNotFound, UnknownError };

    inline Error error() { return err; }
    inline QString errorString() { return errorStr; }

    friend class QDnsAgent;
    friend void QDns::getHostByName(const QString &, QObject *, const char *);

private:
    Error err;
    QString errorStr;
    QList<QHostAddress> addrs;
};

#endif // QDNS_H

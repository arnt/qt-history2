#ifndef QDNS_H
#define QDNS_H

#include <qobject.h>
#include <qhostaddress.h>
#include <qlist.h>

class Q_NETWORK_EXPORT QDns
{
public:
    enum Error { NoError, HostNotFound, UnknownError };

    static void getHostByName(const QString &name,
                              QObject *receiver, const char *member);
};

struct Q_NETWORK_EXPORT QDnsHostInfo
{
    QDnsHostInfo();
    QDnsHostInfo(const QDnsHostInfo &d);

    QDns::Error error;
    QString errorString;
    QList<QHostAddress> addresses;
};

#endif // QDNS_H

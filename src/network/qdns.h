#ifndef QRESOLVER_H
#define QRESOLVER_H

#include <qobject.h>
#include <qhostaddress.h>
#include <qlist.h>

class Q_NETWORK_EXPORT QResolver
{
public:
    enum Error { NoError, HostNotFound, UnknownError };

    static void getHostByName(const QString &name,
                              QObject *receiver, const char *member);
};

struct Q_NETWORK_EXPORT QResolverHostInfo
{
    QResolverHostInfo();
    QResolverHostInfo(const QResolverHostInfo &d);

    QResolver::Error error;
    QString errorString;
    QList<QHostAddress> addresses;
};

#endif // QRESOLVER_H

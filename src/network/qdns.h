#ifndef QDNS_H
#define QDNS_H
#include <qlist.h>
#include <qhostaddress.h>

class QString;
class QObject;
class QDnsHostInfoPrivate;

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
    QDnsHostInfo &operator =(const QDnsHostInfo &d);
    ~QDnsHostInfo();

    QString host() const;
    QList<QHostAddress> addresses() const;

    enum Error { NoError, HostNotFound, UnknownError };

    Error error() const;
    QString errorString() const;

    friend class QDnsAgent;
    friend void QDns::getHostByName(const QString &, QObject *, const char *);

private:
    QDnsHostInfoPrivate *d;
};

#endif // QDNS_H

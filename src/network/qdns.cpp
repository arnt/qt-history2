#include "qdns.h"
#include "qdns_p.h"

#include <private/qspinlock_p.h>
#include <private/qunicodetables_p.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qregexp.h>
#include <qsignal.h>
#include <qsocketdevice.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qurl.h>

static QDnsAgent *agent = 0;

static void cleanup_agent()
{
    delete agent;
}

//#define QDNS_DEBUG

/*! \class QDns
    \brief The QDns class provides a static function for host name lookups.
    \reentrant

\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module network
    \ingroup io

    QDns uses the lookup mechanisms provided by the operating system
    to find the IP addresses of a host name.

    To look up a host's IP address, call getHostByName(), which takes
    the host name and a slot signature as arguments. The lookup is
    asynchronous by default. If Qt is built without thread support,
    this function blocks until the lookup has finished.

    \code
        QDns::getHostByName("www.example.com", this, SLOT(printResults(const QDnsHostInfo &)));
    \endcode

    The slot is invoked once the results are ready.

    QDns supports Internationalized Domain Names (IDNs) through the
    IDNA and Punycode standards.

    \sa QDnsHostInfo
*/

/*! \class QDnsHostInfo
    \brief The QDnsHostInfo class provides information about a host name lookup.
    \reentrant

\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module network
    \ingroup io

    A QDnsHostInfo reference is passed to the slot invoked by
    QDns::getHostByName().

    host() returns the host name that was queried. Call addresses() to
    get the list of host addresses associated with the host name that
    was queried.

    error() returns the type of error that occurred if the lookup
    failed. errorString() gives a human readable description of the
    lookup error.
*/

/*!
    \enum QDns::Error
    \value NoError
    \value HostNotFound
    \value UnknownError
*/

/*!
    Looks up the hostname (IP address) \a name. When the result of the
    lookup is ready, the slot or signal \a member in \a receiver is
    called with a QDnsHostInfo argument. The QDnsHostInfo struct can
    then be inspected to get the results of the lookup.

    Example:

    \code
        QDns::getHostByName("www.trolltech.com", this, SLOT(lookedUp(const QDnsHostInfo&)));
    \endcode

    And here is the implementation of the slot:

    \code
        void MyWidget::lookedUp(const QDnsHostInfo &hosts)
        {
            if (hosts.error != QDns::NoError) {
                qDebug("Lookup failed: %s", hosts.errorString.latin1());
                return;
            }

            for (int i = 0; i < hosts.addresses.count(); ++i)
                qDebug("Got address: %s", hosts.addresses.at(i).toString().latin1());
        }
    \endcode
*/
void QDns::getHostByName(const QString &name, QObject *receiver,
                              const char *member)
{
#if defined QDNS_DEBUG
    qDebug("QDns::getHostByName(\"%s\", %p, %s)", name.latin1(), receiver, member ? member + 1 : 0);
#endif

#if defined Q_OS_WIN32
    QSocketDevice bust; // ### makes sure WSAStartup was callled
#endif

    // Don't start a thread if we don't have to do any lookup.
    QHostAddress addr;
    if (addr.setAddress(name)) {
        if (!member || !member[0]) {
            qWarning("QDns::getHostByName() called with invalid slot [%s]", member);
            return;
        }

        QByteArray arr(member + 1);
        if (!arr.contains('(')) {
            qWarning("QDns::getHostByName() called with invalid slot [%s]", member);
            return;
        }

        QDnsHostInfo info;
        info.d->addrs << addr;
        arr.resize(arr.indexOf('('));

        // To mimic the same behavior that the lookup would have if it was not
        // an IP, we need to choose a QueuedConnection if there is thread support;
        // otherwise DirectConnection.
        if (!qInvokeSlot(receiver, arr,
#if !defined QT_NO_THREAD
                         Qt::QueuedConnection,
#else
                         Qt::DirectConnection,
#endif
                         QGenericArgument("QDnsHostInfo", &info))) {
            qWarning("QDns::getHostByName() called with invalid slot (qInvokeSlot failed)");
        }
        return;
    }

    qRegisterMetaType<QDnsHostInfo>("QDnsHostInfo");

    if (!agent) {
        static QStaticSpinLock spinLock = 0;

        QSpinLockLocker locker(spinLock);
        if (!agent) {
            agent = new QDnsAgent();
            qAddPostRoutine(cleanup_agent);
        }
    }

    // Support for IDNA by first splitting the name into labels, then
    // running the punycode decoder on each part, then merging
    // together before passing the name to the lookup agent.
    QString lookup;
    const unsigned short delimiters[] = {0x2e, 0x3002, 0xff0e, 0xff61, 0};
    QStringList labels = name.split(QRegExp("[" + QString::fromUtf16(delimiters) + "]"));
    for (int i = 0; i < labels.count(); ++i) {
        if (i != 0) lookup += '.';
        QString label = QUnicodeTables::normalize(labels.at(i), QUnicodeTables::NormalizationMode_KC, QChar::Unicode_3_1);
        lookup += QString::fromAscii(QUrl::toPunycode(label));
    }

    agent->addHostName(lookup, receiver, member);

#if !defined QT_NO_THREAD
    if (!agent->isRunning())
        agent->start();
#else
    agent->run();
#endif
}

QDnsHostInfo::QDnsHostInfo()
    : d(new QDnsHostInfoPrivate)
{
}

QDnsHostInfo::QDnsHostInfo(const QDnsHostInfo &hostInfo)
{
    QDnsHostInfoPrivate *x = new QDnsHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
}

QDnsHostInfo &QDnsHostInfo::operator =(const QDnsHostInfo &hostInfo)
{
    QDnsHostInfoPrivate *x = new QDnsHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
    delete x;

    return *this;
}

QDnsHostInfo::~QDnsHostInfo()
{
    delete d;
}

QList<QHostAddress> QDnsHostInfo::addresses() const
{
    return d->addrs;
}

QString QDnsHostInfo::host() const
{
    return d->hostName;
}

QDnsHostInfo::Error QDnsHostInfo::error() const
{
    return d->err;
}

QString QDnsHostInfo::errorString() const
{
    return d->errorStr;
}

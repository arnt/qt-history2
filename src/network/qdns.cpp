#include "qdns.h"
#include "qdns_p.h"
#include <qmetaobject.h>
#include <qtimer.h>
#include <qsignal.h>
#include <qsocketdevice.h>
#include <qsignal.h>
#include <private/qspinlock_p.h>

static QDnsAgent *agent = 0;

/*! \class QDns
    \brief The QDns class provides a static function for hostname lookups.

    To look up a host's IP address, call getHostByName(), which takes
    the host name and a signal/slot signature as arguments. The lookup
    is asynchronous by default. If Qt is built without thread support,
    this function blocks until the lookup has finished.
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

    Here is the implementation of the slot:
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
        info.addresses << addr;
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
        if (!agent)
            agent = new QDnsAgent();
    }
    
    agent->addHostName(name, receiver, member);

#if !defined QT_NO_THREAD
    if (!agent->isRunning())
        agent->start();
#else
    agent->run();
#endif
}

QDnsHostInfo::QDnsHostInfo()
    : error( QDns::NoError ), errorString( "Unknown error" )
{
}

QDnsHostInfo::QDnsHostInfo(const QDnsHostInfo &d)
    : error( d.error ), errorString( d.errorString ), addresses( d.addresses )
{
}

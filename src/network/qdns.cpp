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

    

*/

/*!
    Looks up the hostname \a name. When the results of the lookup are
    ready, the slot or signal \a member in \a receiver is invoked.

    To look up the IP address of a host, call getHostByName(), which
    takes the host name and a signal/slot signature as arguments. The
    lookup is by default asynchronous. If Qt is built without thread
    support, this function blocks until the lookup has succeeded.

    When the lookup is done, the user-defined signal or slot passed as
    argument to getHostByName() is called. The QDnsHostInfo struct can
    then be inspected to get the results of the query.

    In this example, we use QDns::getHostByName() to look up the host
    "www.trolltech.com". When the results are done, the
    displayResults() slot is called:
    
    \code
        QDns::getHostByName("www.trolltech.com", this, SLOT(displayResults(const QDnsHostInfo &)));
    \endcode

    And here is the implementation of the slot:
    
    \code
        void A::displayResults(const QDnsHostInfo &results)
        {
            if (results.error != QDns::NoError) {
                qDebug("Lookup failed: %s", results.errorString.latin1());
                return;
            }
        
            for (int i = 0; i < results.addresses.count(); ++i)
                qDebug("Got address: %s", results.addresses.at(i).toString().latin1());
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

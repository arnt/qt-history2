#include "qdns.h"
#include "qdns_p.h"
#include <qmetaobject.h>
#include <qtimer.h>
#include <qsignal.h>
#include <qsocketdevice.h>

/*!
    Looks up the hostname \a name. When the results of the lookup are
    ready, the slot or signal \a resultsReady in \a receiver is invoked.
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

    QDnsAgent *agent = new QDnsAgent(name);
    qRegisterMetaType<QDnsHostInfo>("QDnsHostInfo");

    QObject::connect(agent, SIGNAL(resultsReady(QDnsHostInfo)), receiver, member);

#if !defined QT_NO_THREAD
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

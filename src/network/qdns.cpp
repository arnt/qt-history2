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

#include "qdns.h"
#include "qdns_p.h"

#include <private/qunicodetables_p.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qregexp.h>
#include <qsignal.h>
#include <private/qsocketlayer_p.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qurl.h>

#ifdef Q_OS_UNIX
#  include <unistd.h>
#endif

Q_GLOBAL_STATIC(QDnsAgent, agent)

//#define QDNS_DEBUG

/*! \class QDns
    \brief The QDns class provides static functions for host name lookups.
    \reentrant

    \module network
    \ingroup io

    QDns uses the lookup mechanisms provided by the operating system
    to find the IP addresses of a host name. It provides two static
    convenience functions, one that works asynchronously and emits a
    signal once the host is found, and one that blocks.

    To look up a host's IP address asynchronously, call
    getHostByName(), which takes the host name and a slot signature as
    arguments. The lookup is asynchronous by default. If Qt is built
    without thread support, this function blocks until the lookup has
    finished.

    \code
        QDns::getHostByName("www.example.com", this, SLOT(printResults(const QDnsHostInfo &)));
    \endcode

    The slot is invoked once the results are ready.

    If you want a blocking lookup use the overloaded getHostByName()
    that takes a single string argument (the hostname).

    QDns supports Internationalized Domain Names (IDNs) through the
    IDNA and Punycode standards.

    \sa QDnsHostInfo \l{http://ietf.org/rfc/rfc3492}{RFC 3492}
*/

/*!
    Looks up the hostname (IP address) \a name. When the result of the
    lookup is ready, the slot or signal \a member in \a receiver is
    called with a QDnsHostInfo argument. The QDnsHostInfo object can
    then be inspected to get the results of the lookup.

    Example:

    The lookup is performed by a single function call:

    \code
        QDns::getHostByName("www.trolltech.com", this, SLOT(lookedUp(const QDnsHostInfo &)));
    \endcode

    The implementation of the slot prints basic information about the
    addresses returned by the lookup, or reports an error if it failed:

    \code
        void MyWidget::lookedUp(const QDnsHostInfo &host)
        {
            if (host.error() != QDns::NoError) {
                qDebug("Lookup failed: %s", host.errorString().latin1());
                return;
            }

            QList<QHostAddress> addresses = host.addresses();
            for (int i = 0; i < addresses.count(); ++i)
                qDebug("Got address: %s", addresses.at(i).toString().latin1());
        }
    \endcode
*/
void QDns::getHostByName(const QString &name, QObject *receiver,
                              const char *member)
{
#if defined QDNS_DEBUG
    qDebug("QDns::getHostByName(\"%s\", %p, %s)", name.latin1(), receiver, member ? member + 1 : 0);
#endif

    qRegisterMetaType<QDnsHostInfo>("QDnsHostInfo");

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
        // an IP, we need to choose a Qt::QueuedConnection if there is thread support;
        // otherwise Qt::DirectConnection.
        if (!qInvokeMetaMember(receiver, arr,
#if !defined QT_NO_THREAD
                         Qt::QueuedConnection,
#else
                         Qt::DirectConnection,
#endif
                         QGenericArgument("QDnsHostInfo", &info))) {
            qWarning("QDns::getHostByName() called with invalid slot (qInvokeMetaMember failed)");
        }
        return;
    }

#if defined Q_OS_WIN32
    QSocketLayer bust; // makes sure WSAStartup was callled
#endif

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

    QDnsAgent *agent = ::agent();

    QDnsResult *result = new QDnsResult;
    QObject::connect(result, SIGNAL(resultsReady(const QDnsHostInfo &)),
                     receiver, member);
    QObject::connect(result, SIGNAL(resultsReady(const QDnsHostInfo &)),
                     result, SLOT(deleteLater()));
    agent->addHostName(lookup, result);

#if !defined QT_NO_THREAD
    if (!agent->isRunning())
        agent->start();
#else
    if (!agent->isRunning())
        agent->run();
    else
        agent->wakeOne();
#endif
}

/*!
    \overload

    This function looks up the IP address for the given host \a name.
    The function blocks during the lookup which means that execution
    of the program is suspended until the results of the lookup are
    ready. Returns the result of the lookup.
*/
QDnsHostInfo QDns::getHostByName(const QString &name)
{
#if defined QDNS_DEBUG
    qDebug("QDns::getHostByName(\"%s\")", name.latin1());
#endif

    // If the address string is an IP address, don't do a lookup.
    QHostAddress addr;
    if (addr.setAddress(name)) {
        QDnsHostInfo info;
        info.d->addrs << addr;
        return info;
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

    return QDnsAgent::getHostByName(lookup);
}

/*!
    \internal
    Pops a query off the queries list, performs a blocking call to
    QDnsAgent::getHostByName(), and emits the resultsReady()
    signal. This process repeats until the queries list is empty.
*/
void QDnsAgent::run()
{
    forever {
        QDnsQuery *query;
        {
            // the queries list is shared between threads. lock all
            // access to it.
            QMutexLocker locker(&mutex);
            if (!quit && queries.isEmpty())
                cond.wait(&mutex);
            if (quit)
                break;
            query = queries.takeFirst();
        }

#if defined(QDNS_DEBUG)
        qDebug("QDnsAgent::run(%p): looking up \"%s\"", this, query->hostName.latin1());
#endif

        query->object->emitResultsReady(getHostByName(query->hostName));
        query->object = 0;
        delete query;
    }
}

/*! \class QDnsHostInfo
    \brief The QDnsHostInfo class provides information about a host name lookup.
    \reentrant

    \module network
    \ingroup io

    A QDnsHostInfo is passed to the slot invoked by
    QDns::getHostByName(). It contains the result of the lookup.

    host() returns the host name that was looked up. Call addresses()
    to get the list of IP addresses for the host.

    If the lookup failed, error() returns the type of error that
    occurred. errorString() gives a human-readable description of the
    lookup error.

    \sa QDns
*/

/*!
    \enum QDnsHostInfo::Error

    \value NoError The lookup was successful.
    \value HostNotFound No IP addresses were found for the host.
    \value UnknownError An unknown error occurred.
*/

/*!
    Constructs an empty host info object.
*/
QDnsHostInfo::QDnsHostInfo()
    : d(new QDnsHostInfoPrivate)
{
}

/*!
    Copy constructor. Copies the data of \a hostInfo.
*/
QDnsHostInfo::QDnsHostInfo(const QDnsHostInfo &hostInfo)
{
    QDnsHostInfoPrivate *x = new QDnsHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
}

/*!
    Assigns the data of the \a hostInfo object to this host info object,
    and returns a reference to it.
*/
QDnsHostInfo &QDnsHostInfo::operator =(const QDnsHostInfo &hostInfo)
{
    QDnsHostInfoPrivate *x = new QDnsHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
    delete x;

    return *this;
}

/*!
    Destroys the host info object.
*/
QDnsHostInfo::~QDnsHostInfo()
{
    delete d;
}

/*!
    Returns the list of IP addresses returned by the host name lookup;
    this list may be empty.

    \sa host()
*/
QList<QHostAddress> QDnsHostInfo::addresses() const
{
    return d->addrs;
}

/*!
    Returns the name of the host whose IP addresses were looked up.
*/
QString QDnsHostInfo::host() const
{
    return d->hostName;
}

/*!
    If the host name lookup failed, this function returns the type of
    error that occurred; otherwise NoError is returned.

    \sa QDnsHostInfo::Error errorString()
*/
QDnsHostInfo::Error QDnsHostInfo::error() const
{
    return d->err;
}

/*!
    If the lookup failed, this function returns a human readable
    description of the error; otherwise "Unknown error" is returned.

    \sa error()
*/
QString QDnsHostInfo::errorString() const
{
    return d->errorStr;
}

/*!
    Returns the host name of this machine.
*/
QString QDns::getHostName()
{
    char hostName[512];
    if (gethostname(hostName, sizeof(hostName)) == -1)
        return QString::null;
    hostName[sizeof(hostName) - 1] = '\0';
    return QString(hostName);
}

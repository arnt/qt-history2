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

#include "qhostinfo.h"
#include "qhostinfo_p.h"

#include <qabstracteventdispatcher.h>
#include <private/qunicodetables_p.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qregexp.h>
#include <qsignal.h>
#include <private/qsocketlayer_p.h>
#include <qstringlist.h>
#include <qthread.h>
#include <qtimer.h>
#include <qurl.h>

#ifdef Q_OS_UNIX
#  include <unistd.h>
#endif

Q_GLOBAL_STATIC(QHostInfoAgent, agent)
   
//#define QHOSTINFO_DEBUG

/*!         
    \class QHostInfo

    \brief The QHostInfo class provides static functions for host name lookups
    via standard Domain Name System services.
    \reentrant

    \module network
    \ingroup io

    QHostInfo uses the lookup mechanisms provided by the operating system
    to find the IP addresses of a host name. It provides two static
    convenience functions, one that works asynchronously and emits a
    signal once the host is found, and one that blocks.

    To look up a host's IP address asynchronously, call lookupHost(), which
    takes the host name and a slot signature as arguments, and returns an
    ID. The lookup is asynchronous by default, and you can abort it by calling
    abortHostLookup(), passing the lookup ID. If Qt is built without thread
    support, lookupHost() blocks until the lookup has finished.

    \code
        QHostInfo::lookupHost("www.example.com", this, SLOT(printResults(const QHostInfo &)));
    \endcode

    The slot is invoked once the results are ready.

    If you want a blocking lookup use the overloaded lookupHost()
    that takes a single string argument (the hostname).

    QHostInfo supports Internationalized Domain Names (IDNs) through the
    IDNA and Punycode standards.

    \sa QHostInfo \link http://ietf.org/rfc/rfc3492 RFC 3492\endlink
*/

static QBasicAtomic idCounter = Q_ATOMIC_INIT(1);
static int qt_qhostinfo_newid()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}

/*!
    Looks up the hostname (IP address) \a name, and returns an ID
    for the lookup. When the result of the
    lookup is ready, the slot or signal \a member in \a receiver is
    called with a QHostInfo argument. The QHostInfo object can
    then be inspected to get the results of the lookup.

    Example:

    The lookup is performed by a single function call:

    \code
        QHostInfo::lookupHost("www.trolltech.com", this, SLOT(lookedUp(const QHostInfo &)));
    \endcode

    The implementation of the slot prints basic information about the
    addresses returned by the lookup, or reports an error if it failed:

    \code
        void MyWidget::lookedUp(const QHostInfo &host)
        {
            if (host.error() != QHostInfo::NoError) {
                qDebug() << "Lookup failed:" << host.errorString();
                return;
            }

            QList<QHostAddress> addresses = host.addresses();
            for (int i = 0; i < addresses.count(); ++i)
                qDebug() << "Got address:" << addresses.at(i).toString();
        }
    \endcode

    \sa abortHostLookup(), fromName()
*/
int QHostInfo::lookupHost(const QString &name, QObject *receiver,
                          const char *member)
{
#if defined QHOSTINFO_DEBUG
    qDebug("QHostInfo::lookupHost(\"%s\", %p, %s)",
           name.toLatin1().constData(), receiver, member ? member + 1 : 0);
#endif
    if (!QAbstractEventDispatcher::instance(QThread::currentThread())) {
        qWarning("QHostInfo::lookupHost() called with no event dispatcher");
        return -1;
    }

    qRegisterMetaType<QHostInfo>("QHostInfo");

    // Don't start a thread if we don't have to do any lookup.
    QHostAddress addr;
    if (addr.setAddress(name)) {
        if (!member || !member[0]) {
            qWarning("QHostInfo::lookupHost() called with invalid slot [%s]", member);
            return -1;
        }

        QByteArray arr(member + 1);
        if (!arr.contains('(')) {
            qWarning("QHostInfo::lookupHost() called with invalid slot [%s]", member);
            return -1;
        }

        QHostInfo info(::qt_qhostinfo_newid());
        info.setAddresses(QList<QHostAddress>() << addr);
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
                         QGenericArgument("QHostInfo", &info))) {
            qWarning("QHostInfo::lookupHost() called with invalid slot (qInvokeMetaMember failed)");
        }
        return info.lookupId();
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
        QString label = QUnicodeTables::normalize(labels.at(i), QString::NormalizationForm_KC, QChar::Unicode_3_1);
        lookup += QString::fromAscii(QUrl::toPunycode(label));
    }

    QHostInfoAgent *agent = ::agent();

    QHostInfoResult *result = new QHostInfoResult;
    QObject::connect(result, SIGNAL(resultsReady(const QHostInfo &)),
                     receiver, member);
    QObject::connect(result, SIGNAL(resultsReady(const QHostInfo &)),
                     result, SLOT(deleteLater()));
    result->lookupId = ::qt_qhostinfo_newid();
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
    return result->lookupId;
}

/*!
    Aborts the host lookup with the ID \a id, as returned by lookupHost().

    \sa lookupHost()
*/
void QHostInfo::abortHostLookup(int id)
{
    QHostInfoAgent *agent = ::agent();
    agent->abortLookup(id);
}

/*!
    \overload

    This function looks up the IP address for the given host \a name.
    The function blocks during the lookup which means that execution
    of the program is suspended until the results of the lookup are
    ready. Returns the result of the lookup.
*/
QHostInfo QHostInfo::fromName(const QString &name)
{
#if defined QHOSTINFO_DEBUG
    qDebug("QHostInfo::fromName(\"%s\")",name.toLatin1().constData());
#endif

    // If the address string is an IP address, don't do a lookup.
    QHostAddress addr;
    if (addr.setAddress(name)) {
        QHostInfo info;
        info.setAddresses(QList<QHostAddress>() << addr);
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
        QString label = QUnicodeTables::normalize(labels.at(i), QString::NormalizationForm_KC, QChar::Unicode_3_1);
        lookup += QString::fromAscii(QUrl::toPunycode(label));
    }

    return QHostInfoAgent::fromName(lookup);
}

/*!
    \internal
    Pops a query off the queries list, performs a blocking call to
    QHostInfoAgent::lookupHost(), and emits the resultsReady()
    signal. This process repeats until the queries list is empty.
*/
void QHostInfoAgent::run()
{
    forever {
        QHostInfoQuery *query;
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

#if defined(QHOSTINFO_DEBUG)
        qDebug("QHostInfoAgent::run(%p): looking up \"%s\"", this,
               query->hostName.toLatin1().constData());
#endif

        QHostInfo info = fromName(query->hostName);
        info.setLookupId(query->object->lookupId);
        query->object->emitResultsReady(info);
        query->object = 0;
        delete query;
    }
}

/*! \class QHostInfo
    \brief The QHostInfo class provides information about a host name lookup.
    \reentrant

    \module network
    \ingroup io

    A QHostInfo is passed to the slot invoked by
    QHostInfo::lookupHost(). It contains the result of the lookup.

    host() returns the host name that was looked up. Call addresses()
    to get the list of IP addresses for the host.

    If the lookup failed, error() returns the type of error that
    occurred. errorString() gives a human-readable description of the
    lookup error.

    \sa QHostInfo
*/

/*!
    \enum QHostInfo::Error

    \value NoError The lookup was successful.
    \value HostNotFound No IP addresses were found for the host.
    \value UnknownError An unknown error occurred.
*/

/*!
    Constructs an empty host info object.
*/
QHostInfo::QHostInfo(int lookupId)
    : d(new QHostInfoPrivate)
{
    d->lookupId = lookupId;
}

/*!
    Copy constructor. Copies the data of \a hostInfo.
*/
QHostInfo::QHostInfo(const QHostInfo &hostInfo)
{
    QHostInfoPrivate *x = new QHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
}

/*!
    Assigns the data of the \a hostInfo object to this host info object,
    and returns a reference to it.
*/
QHostInfo &QHostInfo::operator =(const QHostInfo &hostInfo)
{
    QHostInfoPrivate *x = new QHostInfoPrivate;
    *x = *hostInfo.d;
    x = qAtomicSetPtr(&d, x);
    delete x;

    return *this;
}

/*!
    Destroys the host info object.
*/
QHostInfo::~QHostInfo()
{
    delete d;
}

/*!
    Returns the list of IP addresses returned by the host name lookup;
    this list may be empty.

    \sa host()
*/
QList<QHostAddress> QHostInfo::addresses() const
{
    return d->addrs;
}

/*!
    Sets the list of addresses in this QHostInfo to \a addresses.

    \sa addresses()
*/
void QHostInfo::setAddresses(const QList<QHostAddress> &addresses)
{
    d->addrs = addresses;
}

/*!
    Returns the name of the host whose IP addresses were looked up.
*/
QString QHostInfo::hostName() const
{
    return d->hostName;
}

/*!
    Sets the host name of this QHostInfo to \a hostName.

    \sa hostName()
*/
void QHostInfo::setHostName(const QString &hostName)
{
    d->hostName = hostName;
}

/*!
    If the host name lookup failed, this function returns the type of
    error that occurred; otherwise NoError is returned.

    \sa QHostInfo::Error errorString()
*/
QHostInfo::HostInfoError QHostInfo::error() const
{
    return d->err;
}

/*!
    Sets the error type of this QHostInfo to \a error.

    \sa error()
*/
void QHostInfo::setError(HostInfoError error)
{
    d->err = error;
}

/*!
    Returns the ID of this lookup.

    \sa setLookupId(), abortHostLookup()
*/
int QHostInfo::lookupId() const
{
    return d->lookupId;
}

/*!
    Sets the ID of this lookup to \a id.

    \sa lookupId(), lookupHost()
*/
void QHostInfo::setLookupId(int id)
{
    d->lookupId = id;
}

/*!
    If the lookup failed, this function returns a human readable
    description of the error; otherwise "Unknown error" is returned.

    \sa error()
*/
QString QHostInfo::errorString() const
{
    return d->errorStr;
}

/*!
    Sets the human readable description of the error that occurred to \a str
    if the lookup failed.

    \sa errorString(), setError()
*/
void QHostInfo::setErrorString(const QString &str)
{
    d->errorStr = str;
}

/*!
    \fn QString QHostInfo::getHostName()

    Returns the host name of this machine.
*/


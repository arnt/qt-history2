/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include <private/qnativesocketengine_p.h>
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
    \brief The QHostInfo class provides static functions for host name lookups.

    \reentrant
    \module network
    \ingroup io

    QHostInfo uses the lookup mechanisms provided by the operating
    system to find the IP address(es) associated with a host name,
    or the host name associated with an IP address.
    The class provides two static convenience functions: one that
    works asynchronously and emits a signal once the host is found,
    and one that blocks and returns a QHostInfo object.

    To look up a host's IP addresses asynchronously, call lookupHost(),
    which takes the host name or IP address, a receiver object, and a slot
    signature as arguments and returns an ID. You can abort the
    lookup by calling abortHostLookup() with the lookup ID.

    Example:

    \code
        // To find the IP address of www.trolltech.com
        QHostInfo::lookupHost("www.trolltech.com",
                              this, SLOT(printResults(QHostInfo)));

        // To find the host name for 4.2.2.1
        QHostInfo::lookupHost("4.2.2.1",
                              this, SLOT(printResults(QHostInfo)));
    \endcode


    The slot is invoked when the results are ready. (If you use
    Qtopia Core and disabled multithread support by defining \c
    QT_NO_THREAD, lookupHost() will block until the lookup has
    finished.) The results are stored in a QHostInfo object. Call
    addresses() to get the list of IP addresses for the host, and
    hostName() to get the host name that was looked up.

    If the lookup failed, error() returns the type of error that
    occurred. errorString() gives a human-readable description of the
    lookup error.

    If you want a blocking lookup, use the QHostInfo::fromName() function:

    \code
        QHostInfo info = QHostInfo::fromName("www.trolltech.com");
    \endcode

    QHostInfo supports Internationalized Domain Names (IDNs) through the
    IDNA and Punycode standards.

    To retrieve the name of the local host, use the static
    QHostInfo::localHostName() function.

    \sa QAbstractSocket, {http://www.rfc-editor.org/rfc/rfc3492.txt}{RFC 3492}
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
    Looks up the IP address(es) associated with host name \a name, and
    returns an ID for the lookup. When the result of the lookup is
    ready, the slot or signal \a member in \a receiver is called with
    a QHostInfo argument. The QHostInfo object can then be inspected
    to get the results of the lookup.

    The lookup is performed by a single function call, for example:

    \code
        QHostInfo::lookupHost("www.kde.org",
                              this, SLOT(lookedUp(QHostInfo)));
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

            foreach (QHostAddress address, host.addresses())
                qDebug() << "Found address:" << address.toString();
        }
    \endcode

    If you pass a literal IP address to \a name instead of a host name,
    QHostInfo will search for the domain name for the IP (i.e., QHostInfo will
    perform a \e reverse lookup). On success, the resulting QHostInfo will
    contain both the resolved domain name and IP addresses for the host
    name. Example:

    \code
        QHostInfo::lookupHost("4.2.2.1",
                              this, SLOT(lookedUp(QHostInfo)));
    \endcode

    \sa abortHostLookup(), addresses(), error(), fromName()
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

#if defined Q_OS_WIN32
    QWindowsSockInit bust; // makes sure WSAStartup was callled
#endif

    // Support for IDNA by first splitting the name into labels, then
    // running the punycode decoder on each part, then merging
    // together before passing the name to the lookup agent.
    QString lookup = QString::fromLatin1(QUrl::toAce(name));

    QHostInfoAgent *agent = ::agent();

    QHostInfoResult *result = new QHostInfoResult;
    QObject::connect(result, SIGNAL(resultsReady(QHostInfo)),
                     receiver, member);
    int id = result->lookupId = ::qt_qhostinfo_newid();
    agent->addHostName(lookup, result);

#if !defined QT_NO_THREAD
    if (!agent->isRunning())
        agent->start();
#else
//    if (!agent->isRunning())
	agent->run();
//    else
//	agent->wakeOne();
#endif
    return id;
}

/*!
    Aborts the host lookup with the ID \a id, as returned by lookupHost().

    \sa lookupHost(), lookupId()
*/
void QHostInfo::abortHostLookup(int id)
{
    QHostInfoAgent *agent = ::agent();
    agent->abortLookup(id);
}

/*!
    Looks up the IP address(es) for the given host \a name. The
    function blocks during the lookup which means that execution of
    the program is suspended until the results of the lookup are
    ready. Returns the result of the lookup in a QHostInfo object.

    If you pass a literal IP address to \a name instead of a host name,
    QHostInfo will search for the domain name for the IP (i.e., QHostInfo will
    perform a \e reverse lookup). On success, the returned QHostInfo will
    contain both the resolved domain name and IP addresses for the host name.

    \sa lookupHost()
*/
QHostInfo QHostInfo::fromName(const QString &name)
{
#if defined QHOSTINFO_DEBUG
    qDebug("QHostInfo::fromName(\"%s\")",name.toLatin1().constData());
#endif

    return QHostInfoAgent::fromName(QLatin1String(QUrl::toAce(name)));
}

/*!
    \internal
    Pops a query off the queries list, performs a blocking call to
    QHostInfoAgent::lookupHost(), and emits the resultsReady()
    signal. This process repeats until the queries list is empty.
*/
void QHostInfoAgent::run()
{
#ifndef QT_NO_THREAD
    forever
#endif
    {
        QHostInfoQuery *query;
        {
#ifndef QT_NO_THREAD
            // the queries list is shared between threads. lock all
            // access to it.
            QMutexLocker locker(&mutex);
            if (!quit && queries.isEmpty())
                cond.wait(&mutex);
            if (quit) {
                // Reset the quit variable in case QCoreApplication is
                // destroyed and recreated.
                quit = false;
                break;
            }
	    if (queries.isEmpty())
		continue;
#else
	    if (queries.isEmpty())
		return;
#endif
            query = queries.takeFirst();
            pendingQueryId = query->object->lookupId;
        }

#if defined(QHOSTINFO_DEBUG)
        qDebug("QHostInfoAgent::run(%p): looking up \"%s\"", this,
               query->hostName.toLatin1().constData());
#endif

        QHostInfo info = fromName(query->hostName);
        int id = query->object->lookupId;
        info.setLookupId(id);
        if (pendingQueryId == id)
            query->object->emitResultsReady(info);
        delete query;
    }
}

/*!
    \enum QHostInfo::HostInfoError

    This enum describes the various errors that can occur when trying
    to resolve a host name.

    \value NoError The lookup was successful.
    \value HostNotFound No IP addresses were found for the host.
    \value UnknownError An unknown error occurred.

    \sa error(), setError()
*/

/*!
    Constructs an empty host info object with lookup ID \a id.

    \sa lookupId()
*/
QHostInfo::QHostInfo(int id)
    : d(new QHostInfoPrivate)
{
    d->lookupId = id;
}

/*!
    Constructs a copy of \a other.
*/
QHostInfo::QHostInfo(const QHostInfo &other)
    : d(new QHostInfoPrivate(*other.d))
{
}

/*!
    Assigns the data of the \a other object to this host info object,
    and returns a reference to it.
*/
QHostInfo &QHostInfo::operator=(const QHostInfo &other)
{
    *d = *other.d;
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
    Returns the list of IP addresses associated with hostName(). This
    list may be empty.

    Example:

    \code
        QHostInfo info;
        ...
        if (!info.addresses().isEmpty()) {
            QHostAddress address = info.addresses().first();
            // use the first IP address
        }
    \endcode

    \sa hostName(), error()
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

    \sa localHostName()
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
    Returns the type of error that occurred if the host name lookup
    failed; otherwise returns NoError.

    \sa setError(), errorString()
*/
QHostInfo::HostInfoError QHostInfo::error() const
{
    return d->err;
}

/*!
    Sets the error type of this QHostInfo to \a error.

    \sa error(), errorString()
*/
void QHostInfo::setError(HostInfoError error)
{
    d->err = error;
}

/*!
    Returns the ID of this lookup.

    \sa setLookupId(), abortHostLookup(), hostName()
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

    \sa setErrorString(), error()
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
    \fn QString QHostInfo::localHostName()

    Returns the host name of this machine.

    \sa hostName()
*/

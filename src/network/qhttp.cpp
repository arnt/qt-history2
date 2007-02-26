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

//#define QHTTP_DEBUG

#include <qplatformdefs.h>
#include "qhttp.h"

#ifndef QT_NO_HTTP

#include "private/qobject_p.h"
#include "qtcpsocket.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qlist.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qbuffer.h"
#include "private/qringbuffer_p.h"
#include "qcoreevent.h"
#include "qurl.h"

class QHttpRequest
{
public:
    QHttpRequest()
    { id = idCounter.fetchAndAdd(1); }
    virtual ~QHttpRequest()
    { }

    virtual void start(QHttp *) = 0;
    virtual bool hasRequestHeader();
    virtual QHttpRequestHeader requestHeader();

    virtual QIODevice *sourceDevice() = 0;
    virtual QIODevice *destinationDevice() = 0;

    int id;

private:
    static QBasicAtomic idCounter;
};

class QHttpPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QHttp)

    inline QHttpPrivate() : socket(0), reconnectAttempts(2),
          deleteSocket(0), state(QHttp::Unconnected),
          error(QHttp::NoError), port(0), toDevice(0),
          postDevice(0), bytesDone(0), chunkedSize(-1)
    {
    }

    inline ~QHttpPrivate()
    {
        while (!pending.isEmpty())
            delete pending.takeFirst();

        if (deleteSocket)
            delete socket;
    }

    // private slots
    void _q_startNextRequest();
    void _q_slotReadyRead();
    void _q_slotConnected();
    void _q_slotError(QAbstractSocket::SocketError);
    void _q_slotClosed();
    void _q_slotBytesWritten(qint64 numBytes);
    void _q_slotDoFinished();
    void _q_slotSendRequest();

    int addRequest(QHttpRequest *);
    void finishedWithSuccess();
    void finishedWithError(const QString &detail, int errorCode);

    void init();
    void setState(int);
    void closeConn();
    void setSock(QTcpSocket *sock);

    QTcpSocket *socket;
    int reconnectAttempts;
    bool deleteSocket;
    QList<QHttpRequest *> pending;

    QHttp::State state;
    QHttp::Error error;
    QString errorString;

    QString hostName;
    quint16 port;

    QByteArray buffer;
    QIODevice *toDevice;
    QIODevice *postDevice;

    qint64 bytesDone;
    qint64 bytesTotal;
    qint64 chunkedSize;

    QHttpRequestHeader header;

    bool readHeader;
    QString headerStr;
    QHttpResponseHeader response;

    QRingBuffer rba;

    QString userName;
    QString password;

    QString proxyHost;
    int proxyPort;
    QString proxyUser;
    QString proxyPassword;
};

QBasicAtomic QHttpRequest::idCounter = Q_ATOMIC_INIT(1);

bool QHttpRequest::hasRequestHeader()
{
    return false;
}

QHttpRequestHeader QHttpRequest::requestHeader()
{
    return QHttpRequestHeader();
}

/****************************************************
 *
 * QHttpNormalRequest
 *
 ****************************************************/

class QHttpNormalRequest : public QHttpRequest
{
public:
    QHttpNormalRequest(const QHttpRequestHeader &h, QIODevice *d, QIODevice *t) :
        header(h), to(t)
    {
        is_ba = false;
        data.dev = d;
    }

    QHttpNormalRequest(const QHttpRequestHeader &h, QByteArray *d, QIODevice *t) :
        header(h), to(t)
    {
        is_ba = true;
        data.ba = d;
    }

    ~QHttpNormalRequest()
    {
        if (is_ba)
            delete data.ba;
    }

    void start(QHttp *);
    bool hasRequestHeader();
    QHttpRequestHeader requestHeader();

    QIODevice *sourceDevice();
    QIODevice *destinationDevice();

protected:
    QHttpRequestHeader header;

private:
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;
    QIODevice *to;
};

void QHttpNormalRequest::start(QHttp *http)
{
    if (!http->d_func()->socket) http->d_func()->setSock(0);
    http->d_func()->header = header;

    if (is_ba) {
        http->d_func()->buffer = *data.ba;
        if (http->d_func()->buffer.size() > 0)
            http->d_func()->header.setContentLength(http->d_func()->buffer.size());

        http->d_func()->postDevice = 0;
    } else {
        http->d_func()->buffer = QByteArray();

        if (data.dev && (data.dev->isOpen() || data.dev->open(QIODevice::ReadOnly))) {
            http->d_func()->postDevice = data.dev;
            if (http->d_func()->postDevice->size() > 0)
                http->d_func()->header.setContentLength(http->d_func()->postDevice->size());
        } else {
            http->d_func()->postDevice = 0;
        }
    }

    if (to && (to->isOpen() || to->open(QIODevice::WriteOnly)))
        http->d_func()->toDevice = to;
    else
        http->d_func()->toDevice = 0;

    http->d_func()->reconnectAttempts = 2;
    http->d_func()->_q_slotSendRequest();
}

bool QHttpNormalRequest::hasRequestHeader()
{
    return true;
}

QHttpRequestHeader QHttpNormalRequest::requestHeader()
{
    return header;
}

QIODevice *QHttpNormalRequest::sourceDevice()
{
    if (is_ba)
        return 0;
    return data.dev;
}

QIODevice *QHttpNormalRequest::destinationDevice()
{
    return to;
}

/****************************************************
 *
 * QHttpPGHRequest
 * (like a QHttpNormalRequest, but for the convenience
 * functions put(), get() and head() -- i.e. set the
 * host header field correctly before sending the
 * request)
 *
 ****************************************************/

class QHttpPGHRequest : public QHttpNormalRequest
{
public:
    QHttpPGHRequest(const QHttpRequestHeader &h, QIODevice *d, QIODevice *t) :
        QHttpNormalRequest(h, d, t)
    { }

    QHttpPGHRequest(const QHttpRequestHeader &h, QByteArray *d, QIODevice *t) :
        QHttpNormalRequest(h, d, t)
    { }

    ~QHttpPGHRequest()
    { }

    void start(QHttp *);
};

void QHttpPGHRequest::start(QHttp *http)
{
    if (http->d_func()->port && http->d_func()->port != 80)
	header.setValue(QLatin1String("Host"), http->d_func()->hostName + QLatin1Char(':') + QString::number(http->d_func()->port));
    else
	header.setValue(QLatin1String("Host"), http->d_func()->hostName);
    QHttpNormalRequest::start(http);
}

/****************************************************
 *
 * QHttpSetHostRequest
 *
 ****************************************************/

class QHttpSetHostRequest : public QHttpRequest
{
public:
    QHttpSetHostRequest(const QString &h, quint16 p) :
        hostName(h), port(p)
    { }

    void start(QHttp *);

    QIODevice *sourceDevice()
    { return 0; }
    QIODevice *destinationDevice()
    { return 0; }

private:
    QString hostName;
    quint16 port;
};

void QHttpSetHostRequest::start(QHttp *http)
{
    http->d_func()->hostName = hostName;
    http->d_func()->port = port;
    http->d_func()->finishedWithSuccess();
}

/****************************************************
 *
 * QHttpSetUserRequest
 *
 ****************************************************/

class QHttpSetUserRequest : public QHttpRequest
{
public:
    QHttpSetUserRequest(const QString &userName, const QString &password) :
        user(userName), pass(password)
    { }

    void start(QHttp *);

    QIODevice *sourceDevice()
    { return 0; }
    QIODevice *destinationDevice()
    { return 0; }

private:
    QString user;
    QString pass;
};

void QHttpSetUserRequest::start(QHttp *http)
{
    http->d_func()->userName = user;
    http->d_func()->password = pass;
    http->d_func()->finishedWithSuccess();
}

/****************************************************
 *
 * QHttpSetProxyRequest
 *
 ****************************************************/

class QHttpSetProxyRequest : public QHttpRequest
{
public:
    inline QHttpSetProxyRequest(const QString &proxyHost, int proxyPort,
                                const QString &proxyUser, const QString &proxyPassword)
    {
        this->proxyHost = proxyHost;
        this->proxyPort = proxyPort;
        this->proxyUser = proxyUser;
        this->proxyPassword = proxyPassword;
    }

    inline void start(QHttp *http)
    {
        http->d_func()->proxyHost = proxyHost;
        http->d_func()->proxyPort = proxyPort;
        http->d_func()->proxyUser = proxyUser;
        http->d_func()->proxyPassword = proxyPassword;
        http->d_func()->finishedWithSuccess();
    }

    inline QIODevice *sourceDevice()
    { return 0; }
    inline QIODevice *destinationDevice()
    { return 0; }
private:
    QString proxyHost;
    int proxyPort;
    QString proxyUser;
    QString proxyPassword;
};

/****************************************************
 *
 * QHttpSetSocketRequest
 *
 ****************************************************/

class QHttpSetSocketRequest : public QHttpRequest
{
public:
    QHttpSetSocketRequest(QTcpSocket *s) : socket(s)
    { }

    void start(QHttp *);

    QIODevice *sourceDevice()
    { return 0; }
    QIODevice *destinationDevice()
    { return 0; }

private:
    QTcpSocket *socket;
};

void QHttpSetSocketRequest::start(QHttp *http)
{
    http->d_func()->setSock(socket);
    http->d_func()->finishedWithSuccess();
}

/****************************************************
 *
 * QHttpCloseRequest
 *
 ****************************************************/

class QHttpCloseRequest : public QHttpRequest
{
public:
    QHttpCloseRequest()
    { }
    void start(QHttp *);

    QIODevice *sourceDevice()
    { return 0; }
    QIODevice *destinationDevice()
    { return 0; }
};

void QHttpCloseRequest::start(QHttp *http)
{
    http->d_func()->closeConn();
}

class QHttpHeaderPrivate
{
    Q_DECLARE_PUBLIC(QHttpHeader)
public:
    inline virtual ~QHttpHeaderPrivate() {}

    QList<QPair<QString, QString> > values;
    bool valid;
    QHttpHeader *q_ptr;
};

/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

/*!
    \class QHttpHeader
    \brief The QHttpHeader class contains header information for HTTP.

    \ingroup io
    \module network

    In most cases you should use the more specialized derivatives of
    this class, QHttpResponseHeader and QHttpRequestHeader, rather
    than directly using QHttpHeader.

    QHttpHeader provides the HTTP header fields. A HTTP header field
    consists of a name followed by a colon, a single space, and the
    field value. (See RFC 1945.) Field names are case-insensitive. A
    typical header field looks like this:
    \code
    content-type: text/html
    \endcode

    In the API the header field name is called the "key" and the
    content is called the "value". You can get and set a header
    field's value by using its key with value() and setValue(), e.g.
    \code
    header.setValue("content-type", "text/html");
    QString contentType = header.value("content-type");
    \endcode

    Some fields are so common that getters and setters are provided
    for them as a convenient alternative to using \l value() and
    \l setValue(), e.g. contentLength() and contentType(),
    setContentLength() and setContentType().

    Each header key has a \e single value associated with it. If you
    set the value for a key which already exists the previous value
    will be discarded.

    \sa QHttpRequestHeader QHttpResponseHeader
*/

/*!
    \fn int QHttpHeader::majorVersion() const

    Returns the major protocol-version of the HTTP header.
*/

/*!
    \fn int QHttpHeader::minorVersion() const

    Returns the minor protocol-version of the HTTP header.
*/

/*!
        Constructs an empty HTTP header.
*/
QHttpHeader::QHttpHeader()
    : d_ptr(new QHttpHeaderPrivate)
{
    Q_D(QHttpHeader);
    d->q_ptr = this;
    d->valid = true;
}

/*!
        Constructs a copy of \a header.
*/
QHttpHeader::QHttpHeader(const QHttpHeader &header)
    : d_ptr(new QHttpHeaderPrivate)
{
    Q_D(QHttpHeader);
    d->q_ptr = this;
    d->valid = header.d_func()->valid;
    d->values = header.d_func()->values;
}

/*!
    Constructs a HTTP header for \a str.

    This constructor parses the string \a str for header fields and
    adds this information. The \a str should consist of one or more
    "\r\n" delimited lines; each of these lines should have the format
    key, colon, space, value.
*/
QHttpHeader::QHttpHeader(const QString &str)
    : d_ptr(new QHttpHeaderPrivate)
{
    Q_D(QHttpHeader);
    d->q_ptr = this;
    d->valid = true;
    parse(str);
}

/*! \internal
 */
QHttpHeader::QHttpHeader(QHttpHeaderPrivate &dd, const QString &str)
    : d_ptr(&dd)
{
    Q_D(QHttpHeader);
    d->q_ptr = this;
    d->valid = true;
    if (!str.isEmpty())
        parse(str);
}

/*! \internal
 */
QHttpHeader::QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header)
    : d_ptr(&dd)
{
    Q_D(QHttpHeader);
    d->q_ptr = this;
    d->valid = header.d_func()->valid;
    d->values = header.d_func()->values;
}
/*!
    Destructor.
*/
QHttpHeader::~QHttpHeader()
{
    delete d_ptr;
}

/*!
    Assigns \a h and returns a reference to this http header.
*/
QHttpHeader &QHttpHeader::operator=(const QHttpHeader &h)
{
    Q_D(QHttpHeader);
    d->values = h.d_func()->values;
    d->valid = h.d_func()->valid;
    return *this;
}

/*!
    Returns true if the HTTP header is valid; otherwise returns false.

    A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
    Q_D(const QHttpHeader);
    return d->valid;
}

/*! \internal
    Parses the HTTP header string \a str for header fields and adds
    the keys/values it finds. If the string is not parsed successfully
    the QHttpHeader becomes \link isValid() invalid\endlink.

    Returns true if \a str was successfully parsed; otherwise returns false.

    \sa toString()
*/
bool QHttpHeader::parse(const QString &str)
{
    Q_D(QHttpHeader);
    QStringList lst;
    int pos = str.indexOf(QLatin1Char('\n'));
    if (pos > 0 && str.at(pos - 1) == QLatin1Char('\r'))
        lst = str.trimmed().split(QLatin1String("\r\n"));
    else
        lst = str.trimmed().split(QLatin1String("\n"));
    lst.removeAll(QString()); // No empties

    if (lst.isEmpty())
        return true;

    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for(; it != lst.end(); ++it) {
        if (!(*it).isEmpty()) {
            if ((*it)[0].isSpace()) {
                if (!lines.isEmpty()) {
                    lines.last() += QLatin1Char(' ');
                    lines.last() += (*it).trimmed();
                }
            } else {
                lines.append((*it));
            }
        }
    }

    int number = 0;
    it = lines.begin();
    for(; it != lines.end(); ++it) {
        if (!parseLine(*it, number++)) {
            d->valid = false;
            return false;
        }
    }
    return true;
}

/*! \internal
*/
void QHttpHeader::setValid(bool v)
{
    Q_D(QHttpHeader);
    d->valid = v;
}

/*!
    Returns the first value for the entry with the given \a key. If no entry
    has this \a key, an empty string is returned.

    \sa setValue() removeValue() hasKey() keys()
*/
QString QHttpHeader::value(const QString &key) const
{
    Q_D(const QHttpHeader);
    QString lowercaseKey = key.toLower();
    QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
    while (it != d->values.constEnd()) {
        if ((*it).first == lowercaseKey)
            return (*it).second;
        ++it;
    }
    return QString();
}

/*!
    Returns all the entries with the given \a key. If no entry
    has this \a key, an empty string list is returned.
*/
QStringList QHttpHeader::allValues(const QString &key) const
{
    Q_D(const QHttpHeader);
    QString lowercaseKey = key.toLower();
    QStringList valueList;
    QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
    while (it != d->values.constEnd()) {
        if ((*it).first == lowercaseKey)
            valueList.append((*it).second);
        ++it;
    }
    return valueList;
}

/*!
    Returns a list of the keys in the HTTP header.

    \sa hasKey()
*/
QStringList QHttpHeader::keys() const
{
    Q_D(const QHttpHeader);
    QStringList keyList;
    QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
    while (it != d->values.constEnd()) {
        if (!keyList.contains((*it).first))
            keyList.append((*it).first);
        ++it;
    }
    return keyList;
}

/*!
    Returns true if the HTTP header has an entry with the given \a
    key; otherwise returns false.

    \sa value() setValue() keys()
*/
bool QHttpHeader::hasKey(const QString &key) const
{
    Q_D(const QHttpHeader);
    QString lowercaseKey = key.toLower();
    QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
    while (it != d->values.constEnd()) {
        if ((*it).first == lowercaseKey)
            return true;
        ++it;
    }
    return false;
}

/*!
    Sets the value of the entry with the \a key to \a value.

    If no entry with \a key exists, a new entry with the given \a key
    and \a value is created. If an entry with the \a key already
    exists, the first value is discarded and replaced with the given
    \a value.

    \sa value() hasKey() removeValue()
*/
void QHttpHeader::setValue(const QString &key, const QString &value)
{
    Q_D(QHttpHeader);
    QList<QPair<QString, QString> >::Iterator it = d->values.begin();
    while (it != d->values.end()) {
        if ((*it).first == key) {
            (*it).second = value;
            return;
        }
        ++it;
    }
    // not found so add
    addValue(key, value);
}

/*!
    Sets the header entries to be the list of key value pairs in \a values.
*/
void QHttpHeader::setValues(const QList<QPair<QString, QString> > &values)
{
    Q_D(QHttpHeader);
    d->values = values;
}

/*!
    Adds a new entry with the \a key and \a value.
*/
void QHttpHeader::addValue(const QString &key, const QString &value)
{
    Q_D(QHttpHeader);
    d->values.append(qMakePair(key, value));
}

/*!
    Returns all the entries in the header.
*/
QList<QPair<QString, QString> > QHttpHeader::values() const
{
    Q_D(const QHttpHeader);
    return d->values;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue(const QString &key)
{
    Q_D(QHttpHeader);
    QList<QPair<QString, QString> >::Iterator it = d->values.begin();
    while (it != d->values.end()) {
        if ((*it).first == key) {
            d->values.erase(it);
            return;
        }
        ++it;
    }
}

/*!
    Removes all the entries with the key \a key from the HTTP header.
*/
void QHttpHeader::removeAllValues(const QString &key)
{
    Q_D(QHttpHeader);
    QList<QPair<QString, QString> >::Iterator it = d->values.begin();
    while (it != d->values.end()) {
        if ((*it).first == key) {
            it = d->values.erase(it);
            continue;
        }
        ++it;
    }
}

/*! \internal
    Parses the single HTTP header line \a line which has the format
    key, colon, space, value, and adds key/value to the headers. The
    linenumber is \a number. Returns true if the line was successfully
    parsed and the key/value added; otherwise returns false.

    \sa parse()
*/
bool QHttpHeader::parseLine(const QString &line, int)
{
    int i = line.indexOf(QLatin1Char(':'));
    if (i == -1)
        return false;

    addValue(line.left(i).trimmed().toLower(), line.mid(i + 1).trimmed());

    return true;
}

/*!
    Returns a string representation of the HTTP header.

    The string is suitable for use by the constructor that takes a
    QString. It consists of lines with the format: key, colon, space,
    value, "\r\n".
*/
QString QHttpHeader::toString() const
{
    Q_D(const QHttpHeader);
    if (!isValid())
        return QLatin1String("");

    QString ret = QLatin1String("");

    QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
    while (it != d->values.constEnd()) {
        ret += (*it).first + QLatin1String(": ") + (*it).second + QLatin1String("\r\n");
        ++it;
    }
    return ret;
}

/*!
    Returns true if the header has an entry for the special HTTP
    header field \c content-length; otherwise returns false.

    \sa contentLength() setContentLength()
*/
bool QHttpHeader::hasContentLength() const
{
    return hasKey(QLatin1String("content-length"));
}

/*!
    Returns the value of the special HTTP header field \c
    content-length.

    \sa setContentLength() hasContentLength()
*/
uint QHttpHeader::contentLength() const
{
    return value(QLatin1String("content-length")).toUInt();
}

/*!
    Sets the value of the special HTTP header field \c content-length
    to \a len.

    \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength(int len)
{
    setValue(QLatin1String("content-length"), QString::number(len));
}

/*!
    Returns true if the header has an entry for the the special HTTP
    header field \c content-type; otherwise returns false.

    \sa contentType() setContentType()
*/
bool QHttpHeader::hasContentType() const
{
    return hasKey(QLatin1String("content-type"));
}

/*!
    Returns the value of the special HTTP header field \c content-type.

    \sa setContentType() hasContentType()
*/
QString QHttpHeader::contentType() const
{
    QString type = value(QLatin1String("content-type"));
    if (type.isEmpty())
        return QString();

    int pos = type.indexOf(QLatin1Char(';'));
    if (pos == -1)
        return type;

    return type.left(pos).trimmed();
}

/*!
    Sets the value of the special HTTP header field \c content-type to
    \a type.

    \sa contentType() hasContentType()
*/
void QHttpHeader::setContentType(const QString &type)
{
    setValue(QLatin1String("content-type"), type);
}

class QHttpResponseHeaderPrivate : public QHttpHeaderPrivate
{
    Q_DECLARE_PUBLIC(QHttpResponseHeader)
public:
    int statCode;
    QString reasonPhr;
    int majVer;
    int minVer;
};

/****************************************************
 *
 * QHttpResponseHeader
 *
 ****************************************************/

/*!
    \class QHttpResponseHeader
    \brief The QHttpResponseHeader class contains response header information for HTTP.

    \ingroup io
    \module network

    This class is used by the QHttp class to report the header
    information that the client received from the server.

    HTTP responses have a status code that indicates the status of the
    response. This code is a 3-digit integer result code (for details
    see to RFC 1945). In addition to the status code, you can also
    specify a human-readable text that describes the reason for the
    code ("reason phrase"). This class allows you to get the status
    code and the reason phrase.

    \sa QHttpRequestHeader, QHttp, {HTTP Example}
*/

/*!
    Constructs an empty HTTP response header.
*/
QHttpResponseHeader::QHttpResponseHeader()
    : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
    setValid(false);
}

/*!
    Constructs a copy of \a header.
*/
QHttpResponseHeader::QHttpResponseHeader(const QHttpResponseHeader &header)
    : QHttpHeader(*new QHttpResponseHeaderPrivate, header)
{
    Q_D(QHttpResponseHeader);
    d->statCode = header.d_func()->statCode;
    d->reasonPhr = header.d_func()->reasonPhr;
    d->majVer = header.d_func()->majVer;
    d->minVer = header.d_func()->minVer;
}

/*!
    Copies the contents of \a header into this QHttpResponseHeader.
*/
QHttpResponseHeader &QHttpResponseHeader::operator=(const QHttpResponseHeader &header)
{
    Q_D(QHttpResponseHeader);
    QHttpHeader::operator=(header);
    d->statCode = header.d_func()->statCode;
    d->reasonPhr = header.d_func()->reasonPhr;
    d->majVer = header.d_func()->majVer;
    d->minVer = header.d_func()->minVer;
    return *this;
}

/*!
    Constructs a HTTP response header from the string \a str. The
    string is parsed and the information is set. The \a str should
    consist of one or more "\r\n" delimited lines; the first line should be the
    status-line (format: HTTP-version, space, status-code, space,
    reason-phrase); each of remaining lines should have the format key, colon,
    space, value.
*/
QHttpResponseHeader::QHttpResponseHeader(const QString &str)
    : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
    parse(str);
}

/*!
    \since 4.1

    Constructs a QHttpResponseHeader, setting the status code to \a code, the
    reason phrase to \a text and the protocol-version to \a majorVer and \a
    minorVer.

    \sa statusCode() reasonPhrase() majorVersion() minorVersion()
*/
QHttpResponseHeader::QHttpResponseHeader(int code, const QString &text, int majorVer, int minorVer)
    : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
    setStatusLine(code, text, majorVer, minorVer);
}

/*!
    \since 4.1

    Sets the status code to \a code, the reason phrase to \a text and
    the protocol-version to \a majorVer and \a minorVer.

    \sa statusCode() reasonPhrase() majorVersion() minorVersion()
*/
void QHttpResponseHeader::setStatusLine(int code, const QString &text, int majorVer, int minorVer)
{
    Q_D(QHttpResponseHeader);
    setValid(true);
    d->statCode = code;
    d->reasonPhr = text;
    d->majVer = majorVer;
    d->minVer = minorVer;
}

/*!
    Returns the status code of the HTTP response header.

    \sa reasonPhrase() majorVersion() minorVersion()
*/
int QHttpResponseHeader::statusCode() const
{
    Q_D(const QHttpResponseHeader);
    return d->statCode;
}

/*!
    Returns the reason phrase of the HTTP response header.

    \sa statusCode() majorVersion() minorVersion()
*/
QString QHttpResponseHeader::reasonPhrase() const
{
    Q_D(const QHttpResponseHeader);
    return d->reasonPhr;
}

/*!
    Returns the major protocol-version of the HTTP response header.

    \sa minorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::majorVersion() const
{
    Q_D(const QHttpResponseHeader);
    return d->majVer;
}

/*!
    Returns the minor protocol-version of the HTTP response header.

    \sa majorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::minorVersion() const
{
    Q_D(const QHttpResponseHeader);
    return d->minVer;
}

/*! \reimp
*/
bool QHttpResponseHeader::parseLine(const QString &line, int number)
{
    Q_D(QHttpResponseHeader);
    if (number != 0)
        return QHttpHeader::parseLine(line, number);

    QString l = line.simplified();
    if (l.length() < 10)
        return false;

    if (l.left(5) == QLatin1String("HTTP/") && l[5].isDigit() && l[6] == QLatin1Char('.') &&
        l[7].isDigit() && l[8] == QLatin1Char(' ') && l[9].isDigit()) {
        d->majVer = l[5].toLatin1() - '0';
        d->minVer = l[7].toLatin1() - '0';

        int pos = l.indexOf(QLatin1Char(' '), 9);
        if (pos != -1) {
            d->reasonPhr = l.mid(pos + 1);
            d->statCode = l.mid(9, pos - 9).toInt();
        } else {
            d->statCode = l.mid(9).toInt();
            d->reasonPhr.clear();
        }
    } else {
        return false;
    }

    return true;
}

/*! \reimp
*/
QString QHttpResponseHeader::toString() const
{
    Q_D(const QHttpResponseHeader);
    QString ret(QLatin1String("HTTP/%1.%2 %3 %4\r\n%5\r\n"));
    return ret.arg(d->majVer).arg(d->minVer).arg(d->statCode).arg(d->reasonPhr).arg(QHttpHeader::toString());
}

class QHttpRequestHeaderPrivate : public QHttpHeaderPrivate
{
    Q_DECLARE_PUBLIC(QHttpRequestHeader)
public:
    QString m;
    QString p;
    int majVer;
    int minVer;
};

/****************************************************
 *
 * QHttpRequestHeader
 *
 ****************************************************/

/*!
    \class QHttpRequestHeader
    \brief The QHttpRequestHeader class contains request header information for HTTP.

    \ingroup io
    \module network

    This class is used in the QHttp class to report the header
    information if the client requests something from the server.

    HTTP requests have a method which describes the request's action.
    The most common requests are "GET" and "POST". In addition to the
    request method the header also includes a request-URI to specify
    the location for the method to use.

    The method, request-URI and protocol-version can be set using a
    constructor or later using setRequest(). The values can be
    obtained using method(), path(), majorVersion() and
    minorVersion().

    Important inherited functions: setValue() and value().

    \sa QHttpResponseHeader QHttp
*/

/*!
    Constructs an empty HTTP request header.
*/
QHttpRequestHeader::QHttpRequestHeader()
    : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
    setValid(false);
}

/*!
    Constructs a HTTP request header for the method \a method, the
    request-URI \a path and the protocol-version \a majorVer and \a minorVer.
*/
QHttpRequestHeader::QHttpRequestHeader(const QString &method, const QString &path, int majorVer, int minorVer)
    : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
    Q_D(QHttpRequestHeader);
    d->m = method;
    d->p = path;
    d->majVer = majorVer;
    d->minVer = minorVer;
}

/*!
    Constructs a copy of \a header.
*/
QHttpRequestHeader::QHttpRequestHeader(const QHttpRequestHeader &header)
    : QHttpHeader(*new QHttpRequestHeaderPrivate, header)
{
    Q_D(QHttpRequestHeader);
    d->m = header.d_func()->m;
    d->p = header.d_func()->p;
    d->majVer = header.d_func()->majVer;
    d->minVer = header.d_func()->minVer;
}

/*!
    Copies the content of \a header into this QHttpRequestHeader
*/
QHttpRequestHeader &QHttpRequestHeader::operator=(const QHttpRequestHeader &header)
{
    Q_D(QHttpRequestHeader);
    QHttpHeader::operator=(header);
    d->m = header.d_func()->m;
    d->p = header.d_func()->p;
    d->majVer = header.d_func()->majVer;
    d->minVer = header.d_func()->minVer;
    return *this;
}

/*!
    Constructs a HTTP request header from the string \a str. The \a
    str should consist of one or more "\r\n" delimited lines; the first line
    should be the request-line (format: method, space, request-URI, space
    HTTP-version); each of the remaining lines should have the format key,
    colon, space, value.
*/
QHttpRequestHeader::QHttpRequestHeader(const QString &str)
    : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
    parse(str);
}

/*!
    This function sets the request method to \a method, the
    request-URI to \a path and the protocol-version to \a majorVer and
    \a minorVer.

    \sa method() path() majorVersion() minorVersion()
*/
void QHttpRequestHeader::setRequest(const QString &method, const QString &path, int majorVer, int minorVer)
{
    Q_D(QHttpRequestHeader);
    setValid(true);
    d->m = method;
    d->p = path;
    d->majVer = majorVer;
    d->minVer = minorVer;
}

/*!
    Returns the method of the HTTP request header.

    \sa path() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::method() const
{
    Q_D(const QHttpRequestHeader);
    return d->m;
}

/*!
    Returns the request-URI of the HTTP request header.

    \sa method() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::path() const
{
    Q_D(const QHttpRequestHeader);
    return d->p;
}

/*!
    Returns the major protocol-version of the HTTP request header.

    \sa minorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::majorVersion() const
{
    Q_D(const QHttpRequestHeader);
    return d->majVer;
}

/*!
    Returns the minor protocol-version of the HTTP request header.

    \sa majorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::minorVersion() const
{
    Q_D(const QHttpRequestHeader);
    return d->minVer;
}

/*! \reimp
*/
bool QHttpRequestHeader::parseLine(const QString &line, int number)
{
    Q_D(QHttpRequestHeader);
    if (number != 0)
        return QHttpHeader::parseLine(line, number);

    QStringList lst = line.simplified().split(QLatin1String(" "));
    if (lst.count() > 0) {
        d->m = lst[0];
        if (lst.count() > 1) {
            d->p = lst[1];
            if (lst.count() > 2) {
                QString v = lst[2];
                if (v.length() >= 8 && v.left(5) == QLatin1String("HTTP/") &&
                    v[5].isDigit() && v[6] == QLatin1Char('.') && v[7].isDigit()) {
                    d->majVer = v[5].toLatin1() - '0';
                    d->minVer = v[7].toLatin1() - '0';
                    return true;
                }
            }
        }
    }

    return false;
}

/*! \reimp
*/
QString QHttpRequestHeader::toString() const
{
    Q_D(const QHttpRequestHeader);
    QString first(QLatin1String("%1 %2"));
    QString last(QLatin1String(" HTTP/%3.%4\r\n%5\r\n"));
    return first.arg(d->m).arg(d->p) +
        last.arg(d->majVer).arg(d->minVer).arg(QHttpHeader::toString());
}


/****************************************************
 *
 * QHttp
 *
 ****************************************************/
/*!
    \class QHttp
    \reentrant

    \brief The QHttp class provides an implementation of the HTTP protocol.

    \ingroup io
    \module network
    \mainclass

    This class provides a direct interface to HTTP that allows you to
    have more control over the requests and that allows you to access
    the response header fields.

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "requests"
    in the rest of the documentation) are the following: setHost(),
    get(), post(), head() and request().

    All of these requests return a unique identifier that allows you
    to keep track of the request that is currently executed. When the
    execution of a request starts, the requestStarted() signal with
    the identifier is emitted and when the request is finished, the
    requestFinished() signal is emitted with the identifier and a bool
    that indicates if the request finished with an error.

    To make an HTTP request you must set up suitable HTTP headers. The
    following example demonstrates, how to request the main HTML page
    from the Trolltech home page (i.e. the URL
    \c http://www.trolltech.com/index.html):

    \code
    QHttpRequestHeader header("GET", "/index.html");
    header.setValue("Host", "www.trolltech.com");
    http->setHost("www.trolltech.com");
    http->request(header);
    \endcode

    For the common HTTP requests \c GET, \c POST and \c HEAD, QHttp
    provides the convenience functions get(), post() and head(). They
    already use a reasonable header and if you don't have to set
    special header fields, they are easier to use. The above example
    can also be written as:

    \code
    http->setHost("www.trolltech.com"); // id == 1
    http->get("/index.html");           // id == 2
    \endcode

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \code
    requestStarted(1)
    requestFinished(1, false)

    requestStarted(2)
    stateChanged(Connecting)
    stateChanged(Sending)
    dataSendProgress(77, 77)
    stateChanged(Reading)
    responseHeaderReceived(responseheader)
    dataReadProgress(5388, 0)
    readyRead(responseheader)
    dataReadProgress(18300, 0)
    readyRead(responseheader)
    stateChanged(Connected)
    requestFinished(2, false)

    done(false)

    stateChanged(Closing)
    stateChanged(Unconnected)
    \endcode

    The dataSendProgress() and dataReadProgress() signals in the above
    example are useful if you want to show a \link QProgressBar
    progress bar\endlink to inform the user about the progress of the
    download. The second argument is the total size of data. In
    certain cases it is not possible to know the total amount in
    advance, in which case the second argument is 0. (If you connect
    to a QProgressBar a total of 0 results in a busy indicator.)

    When the response header is read, it is reported with the
    responseHeaderReceived() signal.

    The readyRead() signal tells you that there is data ready to be
    read. The amount of data can then be queried with the
    bytesAvailable() function and it can be read with the read()
    or readAll() functions.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    For example, if you have the following sequence of requests

    \code
    http->setHost("www.foo.bar");       // id == 1
    http->get("/index.html");           // id == 2
    http->post("register.html", data);  // id == 3
    \endcode

    and the get() request fails because the host lookup fails, then
    the post() request is never executed and the signals would look
    like this:

    \code
    requestStarted(1)
    requestFinished(1, false)

    requestStarted(2)
    stateChanged(HostLookup)
    requestFinished(2, true)

    done(true)

    stateChanged(Unconnected)
    \endcode

    You can then get details about the error with the error() and
    errorString() functions. Note that only unexpected behavior, like
    network failure is considered as an error. If the server response
    contains an error status, like a 404 response, this is reported as
    a normal response case. So you should always check the \link
    QHttpResponseHeader::statusCode() status code \endlink of the
    response header.

    The functions currentId() and currentRequest() provide more
    information about the currently executing request.

    The functions hasPendingRequests() and clearPendingRequests()
    allow you to query and clear the list of pending requests.

    \sa QFtp, {HTTP Example}, {Torrent Example}
*/

/*!
    Constructs a QHttp object. The \a parent parameter is passed on
    to the QObject constructor.
*/
QHttp::QHttp(QObject *parent)
    : QObject(*new QHttpPrivate, parent)
{
    Q_D(QHttp);
    d->init();
}

/*!
    Constructs a QHttp object. Subsequent requests are done by
    connecting to the server \a hostName on port \a port.

    The \a parent parameter is passed on to the QObject constructor.

    \sa setHost()
*/
QHttp::QHttp(const QString &hostName, quint16 port, QObject *parent)
    : QObject(*new QHttpPrivate, parent)
{
    Q_D(QHttp);
    d->init();

    d->hostName = hostName;
    d->port = port;
}

void QHttpPrivate::init()
{
    Q_Q(QHttp);
    errorString = QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Unknown error"));
    QMetaObject::invokeMethod(q, "_q_slotDoFinished", Qt::QueuedConnection);
}

/*!
    Destroys the QHttp object. If there is an open connection, it is
    closed.
*/
QHttp::~QHttp()
{
    abort();
}

/*!
    \enum QHttp::State

    This enum is used to specify the state the client is in:

    \value Unconnected There is no connection to the host.
    \value HostLookup A host name lookup is in progress.
    \value Connecting An attempt to connect to the host is in progress.
    \value Sending The client is sending its request to the server.
    \value Reading The client's request has been sent and the client
    is reading the server's response.
    \value Connected The connection to the host is open, but the client is
    neither sending a request, nor waiting for a response.
    \value Closing The connection is closing down, but is not yet
    closed. (The state will be \c Unconnected when the connection is
    closed.)

    \sa stateChanged() state()
*/

/*!  \enum QHttp::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value HostNotFound The host name lookup failed.
    \value ConnectionRefused The server refused the connection.
    \value UnexpectedClose The server closed the connection unexpectedly.
    \value InvalidResponseHeader The server sent an invalid response header.
    \value WrongContentLength The client could not read the content correctly
    because an error with respect to the content length occurred.
    \value Aborted The request was aborted with abort().
    \value UnknownError An error other than those specified above
    occurred.

    \sa error()
*/

/*!
    \fn void QHttp::stateChanged(int state)

    This signal is emitted when the state of the QHttp object changes.
    The argument \a state is the new state of the connection; it is
    one of the \l State values.

    This usually happens when a request is started, but it can also
    happen when the server closes the connection or when a call to
    close() succeeded.

    \sa get() post() head() request() close() state() State
*/

/*!
    \fn void QHttp::responseHeaderReceived(const QHttpResponseHeader &resp);

    This signal is emitted when the HTTP header of a server response
    is available. The header is passed in \a resp.

    \sa get() post() head() request() readyRead()
*/

/*!
    \fn void QHttp::readyRead(const QHttpResponseHeader &resp)

    This signal is emitted when there is new response data to read.

    If you specified a device in the request where the data should be
    written to, then this signal is \e not emitted; instead the data
    is written directly to the device.

    The response header is passed in \a resp.

    You can read the data with the readAll() or read() functions

    This signal is useful if you want to process the data in chunks as
    soon as it becomes available. If you are only interested in the
    complete data, just connect to the requestFinished() signal and
    read the data then instead.

    \sa get() post() request() readAll() read() bytesAvailable()
*/

/*!
    \fn void QHttp::dataSendProgress(int done, int total)

    This signal is emitted when this object sends data to a HTTP
    server to inform it about the progress of the upload.

    \a done is the amount of data that has already arrived and \a
    total is the total amount of data. It is possible that the total
    amount of data that should be transferred cannot be determined, in
    which case \a total is 0.(If you connect to a QProgressBar, the
    progress bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa dataReadProgress(), post(), request(), QProgressBar
*/

/*!
    \fn void QHttp::dataReadProgress(int done, int total)

    This signal is emitted when this object reads data from a HTTP
    server to indicate the current progress of the download.

    \a done is the amount of data that has already arrived and \a
    total is the total amount of data. It is possible that the total
    amount of data that should be transferred cannot be determined, in
    which case \a total is 0.(If you connect to a QProgressBar, the
    progress bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa dataSendProgress() get() post() request() QProgressBar
*/

/*!
    \fn void QHttp::requestStarted(int id)

    This signal is emitted when processing the request identified by
    \a id starts.

    \sa requestFinished() done()
*/

/*!
    \fn void QHttp::requestFinished(int id, bool error)

    This signal is emitted when processing the request identified by
    \a id has finished. \a error is true if an error occurred during
    the processing; otherwise \a error is false.

    \sa requestStarted() done() error() errorString()
*/

/*!
    \fn void QHttp::done(bool error)

    This signal is emitted when the last pending request has finished;
    (it is emitted after the last request's requestFinished() signal).
    \a error is true if an error occurred during the processing;
    otherwise \a error is false.

    \sa requestFinished() error() errorString()
*/

/*!
    Aborts the current request and deletes all scheduled requests.

    For the current request, the requestFinished() signal with the \c
    error argument \c true is emitted. For all other requests that are
    affected by the abort(), no signals are emitted.

    Since this slot also deletes the scheduled requests, there are no
    requests left and the done() signal is emitted (with the \c error
    argument \c true).

    \sa clearPendingRequests()
*/
void QHttp::abort()
{
    Q_D(QHttp);
    if (d->pending.isEmpty())
        return;

    d->finishedWithError(tr("Request aborted"), Aborted);
    clearPendingRequests();
    if (d->socket)
        d->socket->abort();
    d->closeConn();
}

/*!
    Returns the number of bytes that can be read from the response
    content at the moment.

    \sa get() post() request() readyRead() read() readAll()
*/
qint64 QHttp::bytesAvailable() const
{
    Q_D(const QHttp);
#if defined(QHTTP_DEBUG)
    qDebug("QHttp::bytesAvailable(): %d bytes", (int)d->rba.size());
#endif
    return qint64(d->rba.size());
}

/*! \fn qint64 QHttp::readBlock(char *data, quint64 maxlen)

    Use read() instead.
*/

/*!
    Reads \a maxlen bytes from the response content into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() post() request() readyRead() bytesAvailable() readAll()
*/
qint64 QHttp::read(char *data, qint64 maxlen)
{
    Q_D(QHttp);
    if (data == 0 && maxlen != 0) {
        qWarning("QHttp::read: Null pointer error");
        return -1;
    }
    if (maxlen >= d->rba.size())
        maxlen = d->rba.size();
    int readSoFar = 0;
    while (!d->rba.isEmpty() && readSoFar < maxlen) {
        int nextBlockSize = d->rba.nextDataBlockSize();
        int bytesToRead = qMin<qint64>(maxlen - readSoFar, nextBlockSize);
        memcpy(data + readSoFar, d->rba.readPointer(), bytesToRead);
        d->rba.free(bytesToRead);
        readSoFar += bytesToRead;
    }

    d->bytesDone += maxlen;
#if defined(QHTTP_DEBUG)
    qDebug("QHttp::read(): read %lld bytes (%lld bytes done)", maxlen, d->bytesDone);
#endif
    return maxlen;
}

/*!
    Reads all the bytes from the response content and returns them.

    \sa get() post() request() readyRead() bytesAvailable() read()
*/
QByteArray QHttp::readAll()
{
    qint64 avail = bytesAvailable();
    QByteArray tmp;
    tmp.resize(int(avail));
    qint64 got = read(tmp.data(), int(avail));
    tmp.resize(got);
    return tmp;
}

/*!
    Returns the identifier of the HTTP request being executed or 0 if
    there is no request being executed (i.e. they've all finished).

    \sa currentRequest()
*/
int QHttp::currentId() const
{
    Q_D(const QHttp);
    if (d->pending.isEmpty())
        return 0;
    return d->pending.first()->id;
}

/*!
    Returns the request header of the HTTP request being executed. If
    the request is one issued by setHost() or close(), it
    returns an invalid request header, i.e.
    QHttpRequestHeader::isValid() returns false.

    \sa currentId()
*/
QHttpRequestHeader QHttp::currentRequest() const
{
    Q_D(const QHttp);
    if (!d->pending.isEmpty()) {
        QHttpRequest *r = d->pending.first();
        if (r->hasRequestHeader())
            return r->requestHeader();
    }
    return QHttpRequestHeader();
}

/*!
    Returns the received response header of the most recently finished HTTP
    request. If no response has yet been received
    QHttpResponseHeader::isValid() will return false.

    \sa currentRequest()
*/
QHttpResponseHeader QHttp::lastResponse() const
{
    Q_D(const QHttp);
    return d->response;
}

/*!
    Returns the QIODevice pointer that is used as the data source of the HTTP
    request being executed. If there is no current request or if the request
    does not use an IO device as the data source, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the requestFinished() signal.

    \sa currentDestinationDevice() post() request()
*/
QIODevice *QHttp::currentSourceDevice() const
{
    Q_D(const QHttp);
    if (d->pending.isEmpty())
        return 0;
    return d->pending.first()->sourceDevice();
}

/*!
    Returns the QIODevice pointer that is used as to store the data of the HTTP
    request being executed. If there is no current request or if the request
    does not store the data to an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the requestFinished() signal.

    \sa currentSourceDevice() get() post() request()
*/
QIODevice *QHttp::currentDestinationDevice() const
{
    Q_D(const QHttp);
    if (d->pending.isEmpty())
        return 0;
    return d->pending.first()->destinationDevice();
}

/*!
    Returns true if there are any requests scheduled that have not yet
    been executed; otherwise returns false.

    The request that is being executed is \e not considered as a
    scheduled request.

    \sa clearPendingRequests() currentId() currentRequest()
*/
bool QHttp::hasPendingRequests() const
{
    Q_D(const QHttp);
    return d->pending.count() > 1;
}

/*!
    Deletes all pending requests from the list of scheduled requests.
    This does not affect the request that is being executed. If
    you want to stop this this as well, use abort().

    \sa hasPendingRequests() abort()
*/
void QHttp::clearPendingRequests()
{
    Q_D(QHttp);
    // delete all entires except the first one
    while (d->pending.count() > 1)
        delete d->pending.takeLast();
}

/*!
    Sets the HTTP server that is used for requests to \a hostName on
    port \a port.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa get() post() head() request() requestStarted() requestFinished() done()
*/
int QHttp::setHost(const QString &hostName, quint16 port)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpSetHostRequest(hostName, port));
}

/*!
    Replaces the internal QTcpSocket that QHttp uses with \a
    socket. This is useful if you want to use your own custom QTcpSocket
    subclass instead of the plain QTcpSocket that QHttp uses by default.
    QHttp does not take ownership of the socket, and will not delete \a
    socket when destroyed.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    Note: If QHttp is used in a non-GUI thread that runs its own event
    loop, you must move \a socket to that thread before calling setSocket().

    \sa QObject::moveToThread(), {Thread Support in Qt}
*/
int QHttp::setSocket(QTcpSocket *socket)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpSetSocketRequest(socket));
}

/*!
    This function sets the user name \a userName and password \a
    password for web pages that require authentication.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.
*/
int QHttp::setUser(const QString &userName, const QString &password)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpSetUserRequest(userName, password));
}

/*!
    Enables HTTP proxy support, using the proxy server \a host on port \a
    port. \a username and \a password can be provided if the proxy server
    requires authentication.

    Example:

    \code
      void Ticker::getTicks()
      {
        http = new QHttp(this);
        connect(http, SIGNAL(done(bool)), this, SLOT(showPage()));
        http->setProxy("proxy.example.com", 3128);
        http->setHost("ticker.example.com");
        http->get("/ticks.asp");
      }

      void Ticker::showPage()
      {
        display(http->readAll());
      }
    \endcode

    QHttp supports non-transparent web proxy servers only, such as the Squid
    Web proxy cache server (from \l http://www.squid.org/). For transparent
    proxying, such as SOCKS5, use QNetworkProxy instead.

    \sa QFtp::setProxy()
*/
int QHttp::setProxy(const QString &host, int port,
                    const QString &username, const QString &password)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpSetProxyRequest(host, port, username, password));
}

/*!
    Sends a get request for \a path to the server set by setHost() or
    as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() post() head() request() requestStarted() requestFinished() done()
*/
int QHttp::get(const QString &path, QIODevice *to)
{
    Q_D(QHttp);
    QHttpRequestHeader header(QLatin1String("GET"), path);
    header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    return d->addRequest(new QHttpPGHRequest(header, (QIODevice *) 0, to));
}

/*!
    Sends a post request for \a path to the server set by setHost() or
    as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    The incoming data comes via the \a data IO device.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() head() request() requestStarted() requestFinished() done()
*/
int QHttp::post(const QString &path, QIODevice *data, QIODevice *to )
{
    Q_D(QHttp);
    QHttpRequestHeader header(QLatin1String("POST"), path);
    header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    return d->addRequest(new QHttpPGHRequest(header, data, to));
}

/*!
    \overload

    \a data is used as the content data of the HTTP request.
*/
int QHttp::post(const QString &path, const QByteArray &data, QIODevice *to)
{
    Q_D(QHttp);
    QHttpRequestHeader header(QLatin1String("POST"), path);
    header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    return d->addRequest(new QHttpPGHRequest(header, new QByteArray(data), to));
}

/*!
    Sends a header request for \a path to the server set by setHost()
    or as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() post() request() requestStarted() requestFinished() done()
*/
int QHttp::head(const QString &path)
{
    Q_D(QHttp);
    QHttpRequestHeader header(QLatin1String("HEAD"), path);
    header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    return d->addRequest(new QHttpPGHRequest(header, (QIODevice*)0, 0));
}

/*!
    Sends a request to the server set by setHost() or as specified in
    the constructor. Uses the \a header as the HTTP request header.
    You are responsible for setting up a header that is appropriate
    for your request.

    The incoming data comes via the \a data IO device.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() post() head() requestStarted() requestFinished() done()
*/
int QHttp::request(const QHttpRequestHeader &header, QIODevice *data, QIODevice *to)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpNormalRequest(header, data, to));
}

/*!
    \overload

    \a data is used as the content data of the HTTP request.
*/
int QHttp::request(const QHttpRequestHeader &header, const QByteArray &data, QIODevice *to )
{
    Q_D(QHttp);
    return d->addRequest(new QHttpNormalRequest(header, new QByteArray(data), to));
}

/*!
    Closes the connection; this is useful if you have a keep-alive
    connection and want to close it.

    For the requests issued with get(), post() and head(), QHttp sets
    the connection to be keep-alive. You can also do this using the
    header you pass to the request() function. QHttp only closes the
    connection to the HTTP server if the response header requires it
    to do so.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    If you want to close the connection immediately, you have to use
    abort() instead.

    \sa stateChanged() abort() requestStarted() requestFinished() done()
*/
int QHttp::close()
{
    Q_D(QHttp);
    return d->addRequest(new QHttpCloseRequest());
}

/*!
    \obsolete

    Behaves the same as close().
*/
int QHttp::closeConnection()
{
    Q_D(QHttp);
    return d->addRequest(new QHttpCloseRequest());
}

int QHttpPrivate::addRequest(QHttpRequest *req)
{
    Q_Q(QHttp);
    pending.append(req);

    if (pending.count() == 1) {
        // don't emit the requestStarted() signal before the id is returned
        QMetaObject::invokeMethod(q, "_q_startNextRequest", Qt::QueuedConnection);
    }
    return req->id;
}

void QHttpPrivate::_q_startNextRequest()
{
    Q_Q(QHttp);
    if (pending.isEmpty())
        return;
    QHttpRequest *r = pending.first();

    error = QHttp::NoError;
    errorString = QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Unknown error"));

    if (q->bytesAvailable() != 0)
        q->readAll(); // clear the data
    emit q->requestStarted(r->id);
    r->start(q);
}

void QHttpPrivate::_q_slotSendRequest()
{
    // Proxy support. Insert the Proxy-Authorization item into the
    // header before it's sent off to the proxy.
    if (!proxyHost.isEmpty()) {
        QUrl proxyUrl;
        proxyUrl.setScheme(QLatin1String("http"));
        proxyUrl.setHost(hostName);
        if (port && port != 80) proxyUrl.setPort(port);
        QString request = QString::fromAscii(proxyUrl.resolved(QUrl::fromEncoded(header.path().toLatin1())).toEncoded());

        header.setRequest(header.method(), request, header.majorVersion(), header.minorVersion());

        if (!proxyUser.isEmpty()) {
            QByteArray pass = proxyUser.toAscii();
            if (!proxyPassword.isEmpty()) {
                pass += ':';
                pass += proxyPassword.toAscii();
            }
            header.setValue(QLatin1String("Proxy-Authorization"), QLatin1String("Basic " + pass.toBase64()));
        }
    }

    // Username support. Insert the user and password into the query
    // string.
    if (!userName.isEmpty()) {
        QByteArray pass = userName.toAscii();
        if (!password.isEmpty()) {
            pass += ':';
            pass += password.toAscii();
        }
        header.setValue(QLatin1String("Authorization"), QLatin1String("Basic " + pass.toBase64()));
    }

    if (hostName.isNull()) {
        finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "No server set to connect to")),
                          QHttp::UnknownError);
        return;
    }

    // Do we need to setup a new connection or can we reuse an
    // existing one?
    if (socket->peerName() != hostName || socket->peerPort() != port
        || socket->state() != QTcpSocket::ConnectedState) {
        socket->blockSignals(true);
        socket->abort();
        socket->blockSignals(false);

        setState(QHttp::Connecting);
        if (proxyHost.isEmpty())
            socket->connectToHost(hostName, port);
        else
            socket->connectToHost(proxyHost, proxyPort);
    } else {
        _q_slotConnected();
    }

}

void QHttpPrivate::finishedWithSuccess()
{
    Q_Q(QHttp);
    if (pending.isEmpty())
        return;
    QHttpRequest *r = pending.first();

    emit q->requestFinished(r->id, false);
    pending.removeFirst();
    delete r;

    if (pending.isEmpty()) {
        emit q->done(false);
    } else {
        _q_startNextRequest();
    }
}

void QHttpPrivate::finishedWithError(const QString &detail, int errorCode)
{
    Q_Q(QHttp);
    if (pending.isEmpty())
        return;
    QHttpRequest *r = pending.first();

    error = QHttp::Error(errorCode);
    errorString = detail;
    emit q->requestFinished(r->id, true);

    while (!pending.isEmpty())
        delete pending.takeFirst();
    emit q->done(true);
}

void QHttpPrivate::_q_slotClosed()
{
    Q_Q(QHttp);
    if (state == QHttp::Closing)
        return;

    if (state == QHttp::Reading) {
        if (response.hasKey(QLatin1String("content-length"))) {
            // We got Content-Length, so did we get all bytes?
            if (bytesDone + q->bytesAvailable() != response.contentLength()) {
                finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Wrong content length")), QHttp::WrongContentLength);
            }
        }
    } else if (state == QHttp::Connecting || state == QHttp::Sending) {
        finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Server closed connection unexpectedly")), QHttp::UnexpectedClose);
    }

    postDevice = 0;
    setState(QHttp::Closing);
    QMetaObject::invokeMethod(q, "_q_slotDoFinished", Qt::QueuedConnection);
}

void QHttpPrivate::_q_slotConnected()
{
    if (state != QHttp::Sending) {
        bytesDone = 0;
        setState(QHttp::Sending);
    }

    QString str = header.toString();
    bytesTotal = str.length();
    socket->write(str.toLatin1(), bytesTotal);
#if defined(QHTTP_DEBUG)
    qDebug("QHttp: write request header %p:\n---{\n%s}---", &header, str.toLatin1().constData());
#endif

    if (postDevice) {
        bytesTotal += postDevice->size();
    } else {
        bytesTotal += buffer.size();
        socket->write(buffer, buffer.size());
        buffer = QByteArray(); // save memory
    }
}

void QHttpPrivate::_q_slotError(QAbstractSocket::SocketError err)
{
    Q_Q(QHttp);
    postDevice = 0;

    if (state == QHttp::Connecting || state == QHttp::Reading || state == QHttp::Sending) {
        switch (err) {
        case QTcpSocket::ConnectionRefusedError:
            finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Connection refused")), QHttp::ConnectionRefused);
            break;
        case QTcpSocket::HostNotFoundError:
            finishedWithError(QString::fromLatin1(QT_TRANSLATE_NOOP("QHttp", "Host %1 not found"))
                              .arg(socket->peerName()), QHttp::HostNotFound);
            break;
        case QTcpSocket::RemoteHostClosedError:
            if (state == QHttp::Sending && reconnectAttempts--) {
                setState(QHttp::Closing);
                setState(QHttp::Unconnected);
                socket->blockSignals(true);
                socket->abort();
                socket->blockSignals(false);
                QMetaObject::invokeMethod(q, "_q_slotSendRequest", Qt::QueuedConnection);
                return;
            }
            break;
        default:
            finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "HTTP request failed")), QHttp::UnknownError);
            break;
        }
    }

    closeConn();
}

void QHttpPrivate::_q_slotBytesWritten(qint64 written)
{
    Q_Q(QHttp);
    bytesDone += written;
    emit q->dataSendProgress(bytesDone, bytesTotal);

    if (!postDevice)
        return;

    if (socket->bytesToWrite() == 0) {
        int max = qMin<qint64>(4096, postDevice->size() - postDevice->pos());
        QByteArray arr;
        arr.resize(max);

        int n = postDevice->read(arr.data(), max);
        if (n <= 0) {
            qWarning("Could not read enough bytes from the device");
            closeConn();
            return;
        }
        if (postDevice->atEnd()) {
            postDevice = 0;
        }

        socket->write(arr, n);
    }
}

void QHttpPrivate::_q_slotReadyRead()
{
    Q_Q(QHttp);
    if (state != QHttp::Reading) {
        setState(QHttp::Reading);
        buffer = QByteArray();
        readHeader = true;
        headerStr = QLatin1String("");
        bytesDone = 0;
        chunkedSize = -1;
    }

    while (readHeader) {
        bool end = false;
        QString tmp;
        while (!end && socket->canReadLine()) {
            tmp = QString::fromAscii(socket->readLine());
            if (tmp == QLatin1String("\r\n") || tmp == QLatin1String("\n"))
                end = true;
            else
                headerStr += tmp;
        }

        if (!end)
            return;

#if defined(QHTTP_DEBUG)
        qDebug("QHttp: read response header:\n---{\n%s}---", headerStr.toLatin1().constData());
#endif
        response = QHttpResponseHeader(headerStr);
        headerStr = QLatin1String("");
#if defined(QHTTP_DEBUG)
        qDebug("QHttp: read response header:\n---{\n%s}---", response.toString().toLatin1().constData());
#endif
        // Check header
        if (!response.isValid()) {
            finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Invalid HTTP response header")),
                              QHttp::InvalidResponseHeader);
            closeConn();
            return;
        }

        // The 100-continue header is ignored, because when using the
        // POST method, we send both the request header and data in
        // one chunk.
        if (response.statusCode() != 100) {
            readHeader = false;
            if (response.hasKey(QLatin1String("transfer-encoding")) &&
                response.value(QLatin1String("transfer-encoding")).toLower().contains(QLatin1String("chunked")))
                chunkedSize = 0;

            emit q->responseHeaderReceived(response);
        }
    }

    if (!readHeader) {
        bool everythingRead = false;

        if (q->currentRequest().method() == QLatin1String("HEAD")) {
            everythingRead = true;
        } else {
            qint64 n = socket->bytesAvailable();
            QByteArray *arr = 0;
            if (chunkedSize != -1) {
                // transfer-encoding is chunked
                for (;;) {
                    // get chunk size
                    if (chunkedSize == 0) {
                        if (!socket->canReadLine())
                            break;
                        QString sizeString = QString::fromAscii(socket->readLine());
                        int tPos = sizeString.indexOf(QLatin1Char(';'));
                        if (tPos != -1)
                            sizeString.truncate(tPos);
                        bool ok;
                        chunkedSize = sizeString.toInt(&ok, 16);
                        if (!ok) {
                            finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Invalid HTTP chunked body")),
                                              QHttp::WrongContentLength);
                            closeConn();
                            delete arr;
                            return;
                        }
                        if (chunkedSize == 0) // last-chunk
                            chunkedSize = -2;
                    }

                    // read trailer
                    while (chunkedSize == -2 && socket->canReadLine()) {
                        QString read = QString::fromAscii(socket->readLine());
                        if (read == QLatin1String("\r\n") || read == QLatin1String("\n"))
                            chunkedSize = -1;
                    }
                    if (chunkedSize == -1) {
                        everythingRead = true;
                        break;
                    }

                    // make sure that you can read the terminating CRLF,
                    // otherwise wait until next time...
                    n = socket->bytesAvailable();
                    if (n == 0)
                        break;
                    if (n == chunkedSize || n == chunkedSize+1) {
                        n = chunkedSize - 1;
                        if (n == 0)
                            break;
                    }

                    // read data
                    qint64 toRead = chunkedSize < 0 ? n : qMin(n, chunkedSize);
                    if (!arr)
                        arr = new QByteArray;
                    uint oldArrSize = arr->size();
                    arr->resize(oldArrSize + toRead);
                    qint64 read = socket->read(arr->data()+oldArrSize, toRead);
                    arr->resize(oldArrSize + read);

                    chunkedSize -= read;

                    if (chunkedSize == 0 && n - read >= 2) {
                        // read terminating CRLF
                        char tmp[2];
                        socket->read(tmp, 2);
                        if (tmp[0] != '\r' || tmp[1] != '\n') {
                            finishedWithError(QLatin1String(QT_TRANSLATE_NOOP("QHttp", "Invalid HTTP chunked body")),
                                              QHttp::WrongContentLength);
                            closeConn();
                            delete arr;
                            return;
                        }
                    }
                }
            } else if (response.hasContentLength()) {
                n = qMin(qint64(response.contentLength() - bytesDone), n);
                if (n > 0) {
                    arr = new QByteArray;
                    arr->resize(n);
                    qint64 read = socket->read(arr->data(), n);
                    arr->resize(read);
                }
                if (bytesDone + q->bytesAvailable() + n == response.contentLength())
                    everythingRead = true;
            } else if (n > 0) {
                // workaround for VC++ bug
                QByteArray temp = socket->readAll();
                arr = new QByteArray(temp);
            }

            if (arr) {
                n = arr->size();
                if (toDevice) {
                    toDevice->write(*arr, n);
                    delete arr;
                    arr = 0;
                    bytesDone += n;
#if defined(QHTTP_DEBUG)
                    qDebug("QHttp::_q_slotReadyRead(): read %lld bytes (%lld bytes done)", n, bytesDone);
#endif
                    if (response.hasContentLength())
                        emit q->dataReadProgress(bytesDone, response.contentLength());
                    else
                        emit q->dataReadProgress(bytesDone, 0);
                } else {
                    char *ptr = rba.reserve(arr->size());
                    memcpy(ptr, arr->data(), arr->size());
                    delete arr;
                    arr = 0;
#if defined(QHTTP_DEBUG)
                    qDebug("QHttp::_q_slotReadyRead(): read %lld bytes (%lld bytes done)", n, bytesDone + q->bytesAvailable());
#endif
                    if (response.hasContentLength())
                        emit q->dataReadProgress(bytesDone + q->bytesAvailable(), response.contentLength());
                    else
                        emit q->dataReadProgress(bytesDone + q->bytesAvailable(), 0);
                    emit q->readyRead(response);
                }
            }
        }

        if (everythingRead) {
            // Handle "Connection: close"
            if (response.value(QLatin1String("connection")).toLower() == QLatin1String("close")) {
                closeConn();
            } else {
                setState(QHttp::Connected);
                // Start a timer, so that we emit the keep alive signal
                // "after" this method returned.
                QMetaObject::invokeMethod(q, "_q_slotDoFinished", Qt::QueuedConnection);
            }
        }
    }
}

void QHttpPrivate::_q_slotDoFinished()
{
    if (state == QHttp::Connected) {
        finishedWithSuccess();
    } else if (state != QHttp::Unconnected) {
        setState(QHttp::Unconnected);
        finishedWithSuccess();
    }
}


/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
QHttp::State QHttp::state() const
{
    Q_D(const QHttp);
    return d->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what happened when receiving a requestFinished() or a done()
    signal with the \c error argument \c true.

    If you start a new request, the error status is reset to \c NoError.
*/
QHttp::Error QHttp::error() const
{
    Q_D(const QHttp);
    return d->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful to present a error message to the user
    when receiving a requestFinished() or a done() signal with the \c
    error argument \c true.
*/
QString QHttp::errorString() const
{
    Q_D(const QHttp);
    return d->errorString;
}

void QHttpPrivate::setState(int s)
{
    Q_Q(QHttp);
#if defined(QHTTP_DEBUG)
    qDebug("QHttp state changed %d -> %d", state, s);
#endif
    state = QHttp::State(s);
    emit q->stateChanged(s);
}

void QHttpPrivate::closeConn()
{
    Q_Q(QHttp);
    // If no connection is open -> ignore
    if (state == QHttp::Closing || state == QHttp::Unconnected)
        return;

    postDevice = 0;
    setState(QHttp::Closing);

    // Already closed ?
    if (!socket || !socket->isOpen()) {
        QMetaObject::invokeMethod(q, "_q_slotDoFinished", Qt::QueuedConnection);
    } else {
        // Close now.
        socket->close();

        // Did close succeed immediately ?
        if (socket->state() == QTcpSocket::UnconnectedState) {
            // Prepare to emit the requestFinished() signal.
            QMetaObject::invokeMethod(q, "_q_slotDoFinished", Qt::QueuedConnection);
        }
    }
}

void QHttpPrivate::setSock(QTcpSocket *sock)
{
    Q_Q(const QHttp);

    // disconnect all existing signals
    if (socket) socket->disconnect();

    // use the new QTcpSocket socket, or create one if socket is 0.
    deleteSocket = (sock == 0);
    socket = sock ? sock : new QTcpSocket();

    // connect all signals
    QObject::connect(socket, SIGNAL(connected()), q, SLOT(_q_slotConnected()));
    QObject::connect(socket, SIGNAL(disconnected()), q, SLOT(_q_slotClosed()));
    QObject::connect(socket, SIGNAL(readyRead()), q, SLOT(_q_slotReadyRead()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), q, SLOT(_q_slotError(QAbstractSocket::SocketError)));
    QObject::connect(socket, SIGNAL(bytesWritten(qint64)),
                     q, SLOT(_q_slotBytesWritten(qint64)));
}


#include "moc_qhttp.cpp"

#endif

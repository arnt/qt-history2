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

#include "qhttp.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "private/qobject_p.h"
#include "qtcpsocket.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qlist.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qbuffer.h"
#include "qtimer.h"
#include "private/qinternal_p.h"
#include "qcoreevent.h"
#include "qurl.h"

//#define QHTTP_DEBUG

class QHttpRequest
{
public:
    QHttpRequest()
    { id = nextId(); }
    virtual ~QHttpRequest()
    { }

    virtual void start(QHttp *) = 0;
    virtual bool hasRequestHeader();
    virtual QHttpRequestHeader requestHeader();

    virtual QIODevice *sourceDevice() = 0;
    virtual QIODevice *destinationDevice() = 0;

    int id;

private:
    static int idCounter;
    static int nextId();
};

class QHttpPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QHttp)

    inline QHttpPrivate() : socket(0), state(QHttp::Unconnected),
          error(QHttp::NoError), port(0), toDevice(0),
          postDevice(0), bytesDone(0), chunkedSize(-1),
          idleTimer(0)
    {
    }

    inline ~QHttpPrivate()
    {
        while (!pending.isEmpty())
            delete pending.takeFirst();

        delete socket;
    }

    // private slots
    void startNextRequest();
    void slotReadyRead();
    void slotConnected();
    void slotError(int);
    void slotClosed();
    void slotBytesWritten(Q_LONGLONG numBytes);

    int addRequest(QHttpRequest *);
    void sendRequest();
    void finishedWithSuccess();
    void finishedWithError(const QString &detail, int errorCode);

    void killIdleTimer();

    void init();
    void setState(int);
    void closeConn();
    void setSock(QTcpSocket *sock);

    QTcpSocket *socket;
    QList<QHttpRequest *> pending;

    QHttp::State state;
    QHttp::Error error;
    QString errorString;

    QString hostname;
    Q_UINT16 port;

    QByteArray buffer;
    QIODevice *toDevice;
    QIODevice *postDevice;

    Q_LONGLONG bytesDone;
    Q_LONGLONG bytesTotal;
    Q_LONGLONG chunkedSize;

    QHttpRequestHeader header;

    bool readHeader;
    QString headerStr;
    QHttpResponseHeader response;

    int idleTimer;

    QMembuf rba;

    QString userName;
    QString password;

    QString proxyHost;
    int proxyPort;
    QString proxyUser;
    QString proxyPassword;
};

int QHttpRequest::idCounter = 0;
int QHttpRequest::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (q_atomic_test_and_set_int(&idCounter, id, id + 1))
            break;
    }
    return id;
}

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

    http->d_func()->sendRequest();
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
	header.setValue("Host", http->d_func()->hostname + ":" + QString::number(http->d_func()->port));
    else
	header.setValue("Host", http->d_func()->hostname);
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
    QHttpSetHostRequest(const QString &h, Q_UINT16 p) :
        hostname(h), port(p)
    { }

    void start(QHttp *);

    QIODevice *sourceDevice()
    { return 0; }
    QIODevice *destinationDevice()
    { return 0; }

private:
    QString hostname;
    Q_UINT16 port;
};

void QHttpSetHostRequest::start(QHttp *http)
{
    http->d_func()->hostname = hostname;
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
    QMap<QString,QString> values;
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

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif

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
    int pos = str.indexOf('\n');
    if (pos > 0 && str.at(pos - 1) == '\r')
        lst = str.trimmed().split("\r\n");
    else
        lst = str.trimmed().split("\n");
    lst.removeAll(QString()); // No empties

    if (lst.isEmpty())
        return true;

    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for(; it != lst.end(); ++it) {
        if (!(*it).isEmpty()) {
            if ((*it)[0].isSpace()) {
                if (!lines.isEmpty()) {
                    lines.last() += " ";
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
    Returns the value for the entry with the given \a key. If no entry
    has this \a key, an empty string is returned.

    \sa setValue() removeValue() hasKey() keys()
*/
QString QHttpHeader::value(const QString &key) const
{
    Q_D(const QHttpHeader);
    return d->values[key.toLower()];
}

/*!
    Returns a list of the keys in the HTTP header.

    \sa hasKey()
*/
QStringList QHttpHeader::keys() const
{
    Q_D(const QHttpHeader);
    return d->values.keys();
}

/*!
    Returns true if the HTTP header has an entry with the given \a
    key; otherwise returns false.

    \sa value() setValue() keys()
*/
bool QHttpHeader::hasKey(const QString &key) const
{
    Q_D(const QHttpHeader);
    return d->values.contains(key.toLower());
}

/*!
    Sets the value of the entry with the \a key to \a value.

    If no entry with \a key exists, a new entry with the given \a key
    and \a value is created. If an entry with the \a key already
    exists, its value is discarded and replaced with the given \a
    value.

    \sa value() hasKey() removeValue()
*/
void QHttpHeader::setValue(const QString &key, const QString &value)
{
    Q_D(QHttpHeader);
    d->values[key.toLower()] = value;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue(const QString &key)
{
    Q_D(QHttpHeader);
    d->values.remove(key.toLower());
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
    Q_D(QHttpHeader);
    int i = line.indexOf(':');
    if (i == -1)
        return false;

    d->values.insert(line.left(i).trimmed().toLower(), line.mid(i + 1).trimmed());

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
        return "";

    QString ret = "";

    QMap<QString,QString>::ConstIterator it = d->values.begin();
    for(; it != d->values.end(); ++it)
        ret += it.key() + ": " + (*it) + "\r\n";

    return ret;
}

/*!
    Returns true if the header has an entry for the special HTTP
    header field \c content-length; otherwise returns false.

    \sa contentLength() setContentLength()
*/
bool QHttpHeader::hasContentLength() const
{
    return hasKey("content-length");
}

/*!
    Returns the value of the special HTTP header field \c
    content-length.

    \sa setContentLength() hasContentLength()
*/
uint QHttpHeader::contentLength() const
{
    Q_D(const QHttpHeader);
    return d->values["content-length"].toUInt();
}

/*!
    Sets the value of the special HTTP header field \c content-length
    to \a len.

    \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength(int len)
{
    Q_D(QHttpHeader);
    d->values["content-length"] = QString::number(len);
}

/*!
    Returns true if the header has an entry for the the special HTTP
    header field \c content-type; otherwise returns false.

    \sa contentType() setContentType()
*/
bool QHttpHeader::hasContentType() const
{
    return hasKey("content-type");
}

/*!
    Returns the value of the special HTTP header field \c content-type.

    \sa setContentType() hasContentType()
*/
QString QHttpHeader::contentType() const
{
    Q_D(const QHttpHeader);
    QString type = d->values["content-type"];
    if (type.isEmpty())
        return QString();

    int pos = type.indexOf(';');
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
    Q_D(QHttpHeader);
    d->values["content-type"] = type;
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

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif

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

    \sa QHttpRequestHeader QHttp
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

    if (l.left(5) == "HTTP/" && l[5].isDigit() && l[6] == '.' &&
            l[7].isDigit() && l[8] == ' ' && l[9].isDigit()) {
        d->majVer = l[5].latin1() - '0';
        d->minVer = l[7].latin1() - '0';

        int pos = l.indexOf(' ', 9);
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
    QString ret("HTTP/%1.%2 %3 %4\r\n%5\r\n");
    return ret.arg(d->majVer).arg(d->minVer).arg(d->statCode).arg(d->reasonPhr).arg(QHttpHeader::toString());
}

class QHttpRequestHeaderPrivate : public QHttpHeaderPrivate
{
    Q_DECLARE_PUBLIC(QHttpRequestHeader);
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

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif

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

    QStringList lst = line.simplified().split(" ");
    if (lst.count() > 0) {
        d->m = lst[0];
        if (lst.count() > 1) {
            d->p = lst[1];
            if (lst.count() > 2) {
                QString v = lst[2];
                if (v.length() >= 8 && v.left(5) == "HTTP/" &&
                        v[5].isDigit() && v[6] == '.' && v[7].isDigit()) {
                    d->majVer = v[5].latin1() - '0';
                    d->minVer = v[7].latin1() - '0';
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
    QString first("%1 %2");
    QString last(" HTTP/%3.%4\r\n%5\r\n");
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

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif

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
    http://www.trolltech.com/index.html):

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
    progressbar\endlink to inform the user about the progress of the
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

    For example, if you have the following sequence of reqeusts

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

    \sa \l{networking.html}{Networking Overview}, QFtp
*/

/*!
    Constructs a QHttp object. The \a parent and \a name parameters
    are passed on to the QObject constructor.
*/
QHttp::QHttp(QObject *parent)
    : QObject(*new QHttpPrivate, parent)
{
    Q_D(QHttp);
    d->init();
}

/*!
    Constructs a QHttp object. Subsequent requests are done by
    connecting to the server \a hostname on port \a port.

    The \a parent and \a name parameters are passed on to the
    QObject constructor.

    \sa setHost()
*/
QHttp::QHttp(const QString &hostname, Q_UINT16 port, QObject *parent)
    : QObject(*new QHttpPrivate, parent)
{
    Q_D(QHttp);
    d->init();

    d->hostname = hostname;
    d->port = port;
}

void QHttpPrivate::init()
{
    Q_Q(QHttp);
    errorString = QT_TRANSLATE_NOOP(QHttp, "Unknown error");
    idleTimer = q->startTimer(0);
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
    \fn int QHttp::supportedOperations() const

    \internal
*/

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
    \fn void QHttp::responseHeaderReceived(const QHttpResponseHeaer &resp)

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

    \sa dataReadProgress() post() request() QProgressBar::setProgress()
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

    \sa dataSendProgress() get() post() request() QProgressBar::setProgress()
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
Q_LONGLONG QHttp::bytesAvailable() const
{
    Q_D(const QHttp);
#if defined(QHTTP_DEBUG)
    qDebug("QHttp::bytesAvailable(): %d bytes", (int)d->rba.size());
#endif
    return Q_LONGLONG(d->rba.size());
}

/*! \fn Q_LONGLONG QHttp::readBlock(char *data, Q_ULONG maxlen)

    Use read() instead.
*/

/*!
    Reads \a maxlen bytes from the response content into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() post() request() readyRead() bytesAvailable() readAll()
*/
Q_LONGLONG QHttp::read(char *data, Q_LONGLONG maxlen)
{
    Q_D(QHttp);
    if (data == 0 && maxlen != 0) {
        qWarning("QHttp::read: Null pointer error");
        return -1;
    }
    if (maxlen >= d->rba.size())
        maxlen = d->rba.size();
    d->rba.consumeBytes(maxlen, data);

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
    Q_LONGLONG avail = bytesAvailable();
    QByteArray tmp;
    tmp.resize(int(avail));
    Q_LONGLONG got = read(tmp.data(), int(avail));
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

    \sa currentDestinationDevice() get() post() request()
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
    Sets the HTTP server that is used for requests to \a hostname on
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
int QHttp::setHost(const QString &hostname, Q_UINT16 port)
{
    Q_D(QHttp);
    return d->addRequest(new QHttpSetHostRequest(hostname, port));
}

/*!
    Replaces the internal QSocket that QHttp uses with the given \a
    socket. This is useful if you want to use your own custom QSocket
    subclass instead of the plain QSocket that QHttp uses by default.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.
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
    Enables proxy support, using the proxy server \a host on port \a
    port. \a username and \a password can be provided if the proxy
    server requires authentication.
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
    QHttpRequestHeader header("GET", path);
    header.setValue("Connection", "Keep-Alive");
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
    QHttpRequestHeader header("POST", path);
    header.setValue("Connection", "Keep-Alive");
    return d->addRequest(new QHttpPGHRequest(header, data, to));
}

/*!
    \overload

    \a data is used as the content data of the HTTP request.
*/
int QHttp::post(const QString &path, const QByteArray &data, QIODevice *to)
{
    Q_D(QHttp);
    QHttpRequestHeader header("POST", path);
    header.setValue("Connection", "Keep-Alive");
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
    QHttpRequestHeader header("HEAD", path);
    header.setValue("Connection", "Keep-Alive");
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
        QTimer::singleShot(0, q, SLOT(startNextRequest()));
    }
    return req->id;
}

void QHttpPrivate::startNextRequest()
{
    Q_Q(QHttp);
    if (pending.isEmpty())
        return;
    QHttpRequest *r = pending.first();

    error = QHttp::NoError;
    errorString = QT_TRANSLATE_NOOP(QHttp, "Unknown error");

    if (q->bytesAvailable() != 0)
        q->readAll(); // clear the data
    emit q->requestStarted(r->id);
    r->start(q);
}

void QHttpPrivate::sendRequest()
{
    // Proxy support. Insert the Proxy-Authorization item into the
    // header before it's sent off to the proxy.
    if (!proxyHost.isEmpty()) {
        QUrl proxyUrl;
        proxyUrl.setScheme("http");
        proxyUrl.setHost(hostname);
        if (port && port != 80) proxyUrl.setPort(port);
        QString request = proxyUrl.resolved(QUrl(header.path())).toEncoded();

        header = QHttpRequestHeader(header.method(), request,
                                    header.majorVersion(),
                                    header.minorVersion());
        if (!proxyUser.isEmpty()) {
            QByteArray pass = proxyUser.toAscii();
            if (!proxyPassword.isEmpty()) {
                pass += ":";
                pass += proxyPassword.toAscii();
            }
            header.setValue("Proxy-Authorization", pass.toBase64());
        }

        hostname = proxyHost;
    }

    // Username support. Insert the user and password into the query
    // string.
    if (!userName.isEmpty()) {
        QByteArray pass = userName.toAscii();
        if (!password.isEmpty()) {
            pass += ":";
            pass += password.toAscii();
        }
        header.setValue("Authorization", "Basic " + pass.toBase64());
    }

    if (hostname.isNull()) {
        finishedWithError(QT_TRANSLATE_NOOP(QHttp, "No server set to connect to"),
                          QHttp::UnknownError);
        return;
    }

    killIdleTimer();

    // Do we need to setup a new connection or can we reuse an
    // existing one?
    if (socket->peerName() != hostname || socket->peerPort() != port
        || socket->socketState() != Qt::ConnectedState) {
        setState(QHttp::Connecting);
        if (proxyHost.isEmpty())
            socket->connectToHost(hostname, port);
        else
            socket->connectToHost(proxyHost, proxyPort);
    } else {
        slotConnected();
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
        startNextRequest();
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

void QHttpPrivate::slotClosed()
{
    Q_Q(QHttp);
    if (state == QHttp::Closing)
        return;

    if (state == QHttp::Reading) {
        if (response.hasKey("content-length")) {
            // We got Content-Length, so did we get all bytes?
            if (bytesDone + q->bytesAvailable() != response.contentLength()) {
                finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Wrong content length"), QHttp::WrongContentLength);
            }
        }
    } else if (state == QHttp::Connecting || state == QHttp::Sending) {
        finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Server closed connection unexpectedly"), QHttp::UnexpectedClose);
    }

    postDevice = 0;
    setState(QHttp::Closing);
    idleTimer = q->startTimer(0);
}

void QHttpPrivate::slotConnected()
{
    if (state != QHttp::Sending) {
        bytesDone = 0;
        setState(QHttp::Sending);
    }

    QString str = header.toString();
    bytesTotal = str.length();
    socket->write(str.latin1(), bytesTotal);
#if defined(QHTTP_DEBUG)
    qDebug("QHttp: write request header %p:\n---{\n%s}---", &header, str.latin1());
#endif

    if (postDevice) {
        bytesTotal += postDevice->size();
    } else {
        bytesTotal += buffer.size();
        socket->write(buffer, buffer.size());
        buffer = QByteArray(); // save memory
    }
}

void QHttpPrivate::slotError(int err)
{
    postDevice = 0;

    if (state == QHttp::Connecting || state == QHttp::Reading || state == QHttp::Sending) {
        switch (err) {
        case Qt::ConnectionRefusedError:
                finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Connection refused"), QHttp::ConnectionRefused);
                break;
        case Qt::HostNotFoundError:
                finishedWithError(QString(QT_TRANSLATE_NOOP(QHttp, "Host %1 not found"))
                                  .arg(socket->peerName()), QHttp::HostNotFound);
                break;
            default:
                finishedWithError(QT_TRANSLATE_NOOP(QHttp, "HTTP request failed"), QHttp::UnknownError);
                break;
        }
    }

    closeConn();
}

void QHttpPrivate::slotBytesWritten(Q_LONGLONG written)
{
    Q_Q(QHttp);
    bytesDone += written;
    emit q->dataSendProgress(bytesDone, bytesTotal);

    if (!postDevice)
        return;

    if (socket->bytesToWrite() == 0) {
        int max = qMin(4096, postDevice->size() - postDevice->pos());
        QByteArray arr;
        arr.resize(max);

        int n = postDevice->read(arr.data(), max);
        if (n != max) {
            qWarning("Could not read enough bytes from the device");
            closeConn();
            return;
        }
        if (postDevice->atEnd()) {
            postDevice = 0;
        }

        socket->write(arr, max);
    }
}

void QHttpPrivate::slotReadyRead()
{
    Q_Q(QHttp);
    if (state != QHttp::Reading) {
        setState(QHttp::Reading);
        buffer = QByteArray();
        readHeader = true;
        headerStr = "";
        bytesDone = 0;
        chunkedSize = -1;
    }

    while (readHeader) {
        bool end = false;
        QString tmp;
        while (!end && socket->canReadLine()) {
            tmp = socket->readLine();
            if (tmp == "\r\n" || tmp == "\n")
                end = true;
            else
                headerStr += tmp;
        }

        if (!end)
            return;

#if defined(QHTTP_DEBUG)
        qDebug("QHttp: read response header:\n---{\n%s}---", headerStr.latin1());
#endif
        response = QHttpResponseHeader(headerStr);
        headerStr = "";
#if defined(QHTTP_DEBUG)
        qDebug("QHttp: read response header:\n---{\n%s}---", response.toString().latin1());
#endif
        // Check header
        if (!response.isValid()) {
            finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Invalid HTTP response header"),
                              QHttp::InvalidResponseHeader);
            closeConn();
            return;
        }

        // The 100-continue header is ignored, because when using the
        // POST method, we send both the request header and data in
        // one chunk.
        if (response.statusCode() != 100) {
            readHeader = false;
            if (response.hasKey("transfer-encoding") &&
                 response.value("transfer-encoding").toLower().contains("chunked"))
                chunkedSize = 0;

            emit q->responseHeaderReceived(response);
        }
    }

    if (!readHeader) {
        bool everythingRead = false;

        if (q->currentRequest().method() == "HEAD") {
            everythingRead = true;
        } else {
            Q_LONGLONG n = socket->bytesAvailable();
            QByteArray *arr = 0;
            if (chunkedSize != -1) {
                // transfer-encoding is chunked
                for (;;) {
                    // get chunk size
                    if (chunkedSize == 0) {
                        if (!socket->canReadLine())
                            break;
                        QString sizeString = socket->readLine();
                        int tPos = sizeString.indexOf(';');
                        if (tPos != -1)
                            sizeString.truncate(tPos);
                        bool ok;
                        chunkedSize = sizeString.toInt(&ok, 16);
                        if (!ok) {
                            finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Invalid HTTP chunked body"),
                                              QHttp::WrongContentLength);
                            closeConn();
                            return;
                        }
                        if (chunkedSize == 0) // last-chunk
                            chunkedSize = -2;
                    }

                    // read trailer
                    while (chunkedSize == -2 && socket->canReadLine()) {
                        QString read = socket->readLine();
                        if (read == "\r\n" || read == "\n")
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
                    Q_LONGLONG toRead = qMin(n, chunkedSize);
                    if (!arr)
                        arr = new QByteArray;
                    uint oldArrSize = arr->size();
                    arr->resize(oldArrSize + toRead);
                    Q_LONGLONG read = socket->read(arr->data()+oldArrSize, toRead);
                    arr->resize(oldArrSize + read);

                    chunkedSize -= read;

                    if (chunkedSize == 0 && n - read >= 2) {
                        // read terminating CRLF
                        char tmp[2];
                        socket->read(tmp, 2);
                        if (tmp[0] != '\r' || tmp[1] != '\n') {
                            finishedWithError(QT_TRANSLATE_NOOP(QHttp, "Invalid HTTP chunked body"),
                                              QHttp::WrongContentLength);
                            closeConn();
                            return;
                        }
                    }
                }
            } else if (response.hasContentLength()) {
                n = qMin(Q_LONGLONG(response.contentLength() - bytesDone), n);
                if (n > 0) {
                    arr = new QByteArray;
                    arr->resize(n);
                    Q_LONGLONG read = socket->read(arr->data(), n);
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
                    bytesDone += n;
#if defined(QHTTP_DEBUG)
                    qDebug("QHttp::slotReadyRead(): read %lld bytes (%lld bytes done)", n, bytesDone);
#endif
                    if (response.hasContentLength())
                        emit q->dataReadProgress(bytesDone, response.contentLength());
                    else
                        emit q->dataReadProgress(bytesDone, 0);
                } else {
                    rba.append(arr);
#if defined(QHTTP_DEBUG)
                    qDebug("QHttp::slotReadyRead(): read %lld bytes (%lld bytes done)", n, bytesDone + q->bytesAvailable());
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
            if (response.value("connection").toLower() == "close") {
                closeConn();
            } else {
                setState(QHttp::Connected);
                // Start a timer, so that we emit the keep alive signal
                // "after" this method returned.
                idleTimer = q->startTimer(0);
            }
        }
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

/*! \reimp
*/
void QHttp::timerEvent(QTimerEvent *e)
{
    Q_D(QHttp);
    if (e->timerId() == d->idleTimer) {
        killTimer(d->idleTimer);
        d->idleTimer = 0;

        if (d->state == Connected) {
            d->finishedWithSuccess();
        } else if (d->state != Unconnected) {
            d->setState(Unconnected);
            d->finishedWithSuccess();
        }
    } else {
        QObject::timerEvent(e);
    }
}

void QHttpPrivate::killIdleTimer()
{
    Q_Q(QHttp);
    if (idleTimer > 0)
        q->killTimer(idleTimer);
    idleTimer = 0;
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
        idleTimer = q->startTimer(0);
    } else {
        // Close now.
        socket->close();

        // Did close succeed immediately ?
        if (socket->socketState() == Qt::UnconnectedState) {
            // Prepare to emit the requestFinished() signal.
            idleTimer = q->startTimer(0);
        }
    }
}

void QHttpPrivate::setSock(QTcpSocket *sock)
{
    Q_Q(const QHttp);

    // disconnect all existing signals
    if (socket) socket->disconnect();

    // use the new QSocket socket, or create one if socket is 0.
    socket = sock ? sock : new QTcpSocket();

    // connect all signals
    QObject::connect(socket, SIGNAL(connected()), q, SLOT(slotConnected()));
    QObject::connect(socket, SIGNAL(closed()), q, SLOT(slotClosed()));
    QObject::connect(socket, SIGNAL(readyRead()), q, SLOT(slotReadyRead()));
    QObject::connect(socket, SIGNAL(error(int)), q, SLOT(slotError(int)));
    QObject::connect(socket, SIGNAL(bytesWritten(Q_LONGLONG)),
                     q, SLOT(slotBytesWritten(Q_LONGLONG)));
}

#define d d_func()
#include "moc_qhttp.cpp"

#endif

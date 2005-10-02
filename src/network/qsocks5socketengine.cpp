#include "qsocks5socketengine_p.h"
#include "qtcpsocket.h"
#include "qudpsocket.h"
#include "qtcpserver.h"
#include "qdebug.h"
#include "qhash.h"
#include "qqueue.h"
#include "qdatetime.h"

//#define QSOCKS5SOCKETLAYER_DEBUG

#define MAX_DATA_DUMP 256

#define Q_INIT_CHECK(returnValue) do { \
    if (!d->data) { \
        return returnValue; \
    } } while (0)

#ifdef QSOCKS5SOCKETLAYER_DEBUG
#  define QSOCKS5_DEBUG qDebug() << "[QSocks5]"
#else
#  define QSOCKS5_DEBUG if (0) qDebug()
#endif 

#define S5_VERSION_5 0x05
#define S5_CONNECT 0x01
#define S5_BIND 0x02
#define S5_UDP_ASSOCIATE 0x03
#define S5_IP_V4 0x01
#define S5_DOMAINNAME 0x03
#define S5_IP_V6 0x04
#define S5_SUCCESS 0x00
#define S5_R_ERROR_SOCKS_FAILURE 0x01
#define S5_R_ERROR_CON_NOT_ALLOWED 0x02
#define S5_R_ERROR_NET_UNREACH 0x03
#define S5_R_ERROR_HOST_UNREACH 0x04
#define S5_R_ERROR_CONN_REFUSED 0x05
#define S5_R_ERROR_TTL 0x06
#define S5_R_ERROR_CMD_NOT_SUPPORTED 0x07
#define S5_R_ERROR_ADD_TYPE_NOT_SUPORTED 0x08

static QString s5RequestErrorToString(int s5_r_error)
{
    QString ret;
    switch(s5_r_error) {
    case 0x01 : ret = QLatin1String("general SOCKS server failure"); break;
    case 0x02 : ret = QLatin1String("connection not allowed by ruleset"); break;
    case 0x03 : ret = QLatin1String("Network unreachable"); break;
    case 0x04 : ret = QLatin1String("Host unreachable"); break;
    case 0x05 : ret = QLatin1String("Connection refused"); break;
    case 0x06 : ret = QLatin1String("TTL expired"); break;
    case 0x07 : ret = QLatin1String("Command not supported"); break;
    case 0x08 : ret = QLatin1String("Address type not supported"); break;
    default   : ret = QLatin1String("unassigned error code"); break;
    }
    return ret;
}

static QString makeErrorString(const QString & e)
{
    return "Socks 5 - " + e;
}

static QAbstractSocket::SocketError s5RAsSocketError(int s5_r_error)
{
    QAbstractSocket::SocketError ret;
    switch(s5_r_error) {
    case 0x01 : ret = QAbstractSocket::NetworkError; break;
    case 0x02 : ret = QAbstractSocket::SocketAccessError; break;
    case 0x03 : ret = QAbstractSocket::NetworkError; break;
    case 0x04 : ret = QAbstractSocket::HostNotFoundError; break;
    case 0x05 : ret = QAbstractSocket::ConnectionRefusedError; break;
    case 0x06 : ret = QAbstractSocket::NetworkError; break;
    case 0x07 : ret = QAbstractSocket::UnsupportedSocketOperationError; break;
    case 0x08 : ret = QAbstractSocket::UnsupportedSocketOperationError; break;
    default   : ret = QAbstractSocket::NetworkError; break;
    }
    return ret;
}

static QString s5StateToString(QSocks5SocketEnginePrivate::Socks5State s)
{
    switch (s) {
    case QSocks5SocketEnginePrivate::unInitialized : return "unInitialized";
    case QSocks5SocketEnginePrivate::AuthenticationMethodsSent : return "AuthenticationMethodsSent";
    case QSocks5SocketEnginePrivate::Authenticating : return "Authenticating";
    case QSocks5SocketEnginePrivate::RequestMethodSent : return "RequestMethodSent";
    case QSocks5SocketEnginePrivate::RequestSuccess : return "RequestSuccess";
    case QSocks5SocketEnginePrivate::RequestError : return "RequestError";
    case QSocks5SocketEnginePrivate::Connected : return "Connected";
    case QSocks5SocketEnginePrivate::ConnectError : return "BindSuccess";
    case QSocks5SocketEnginePrivate::BindSuccess : return "unInitialized";
    case QSocks5SocketEnginePrivate::BindError : return "BindError";
    case QSocks5SocketEnginePrivate::ControlSocketError : return "ControlSocketError";
    case QSocks5SocketEnginePrivate::SocksError : return "SocksError";
    default : break;
    }
    return "unknown state";
}

static QString dump(const QByteArray &buf)
{
    QString data;
    for (int i = 0; i < qMin<int>(MAX_DATA_DUMP, buf.size()); ++i) {
        if (i) data += " ";
        uint val = (unsigned char)buf.at(i);
       // data += QString("0x%1").arg(val, 3, 16, QLatin1Char('0'));
        data += QString::number(val);
    }
    if (buf.size() > MAX_DATA_DUMP)
        data += " ...";

    return QString("size: %1 data: { %2 }").arg(buf.size()).arg(data);
}

/*
   inserets the host address in buf at pos and updates pos.
   if the func fails the data in buf and the vallue of pos is undefined
*/
static bool qt_socks5_set_host_address_and_port(const QHostAddress &address, quint16 port, QByteArray *pBuf, int *pPos)
{
    bool ret = false;
    int pos = *pPos;

    QSOCKS5_DEBUG << "setting [" << address << ":" << port << "]";

    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        int spaceAvailable = pBuf->size() - pos;
        if (spaceAvailable < 5)
            pBuf->resize(pBuf->size() + 5 - spaceAvailable);
        char *buf = pBuf->data();
        buf[pos++] = S5_IP_V4;
        quint32 add = qhton<quint32>(address.toIPv4Address());
        char *ipv4 = reinterpret_cast<char *>(&add);
        buf[pos++] = ipv4[0];
        buf[pos++] = ipv4[1];
        buf[pos++] = ipv4[2];
        buf[pos++] = ipv4[3];
        ret = true;
    } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        int spaceAvailable = pBuf->size() - pos;
        if (spaceAvailable < 17)
            pBuf->resize(pBuf->size() + 17 - spaceAvailable);
        char *buf = pBuf->data();
        buf[pos++] = S5_IP_V6;
        QIPv6Address ipv6 = address.toIPv6Address();
        for (int i = 0; i < 16; ++i)
            buf[pos++] = ipv6[i];
        ret = true;
    } else {
        // domain name.
        ret = false;
    }

    if (ret) {
        int spaceAvailable = pBuf->size() - pos;
        if (spaceAvailable < 2)
            pBuf->resize(pBuf->size() + 2 - spaceAvailable);
        char *buf = pBuf->data();
        quint16 nPort = qhton<quint16>(port);
        char *pport = reinterpret_cast<char *>(&nPort);
        buf[pos++] = pport[0];
        buf[pos++] = pport[1];
    }

    if (ret)
        *pPos = pos;

    return ret;
}

/*
   retrives the host address in buf at pos and updates pos.
   if the func fails the value of the address and the pos is undefined
*/
static bool qt_socks5_get_host_address_and_port(const QByteArray &buf, QHostAddress *pAddress, quint16 *pPort, int *pPos)
{
    bool ret = false;
    int pos = *pPos;
    const char *pBuf = buf.constData();
    QHostAddress address;
    quint16 port = 0;

    if (buf.size() - pos < 1) {
        QSOCKS5_DEBUG << "need more data address/port";
        return false;
    }
    if (pBuf[pos] == S5_IP_V4) {
        pos++;
        if (buf.size() - pos < 4) {
            QSOCKS5_DEBUG << "need more data for ip4 address";
            return false;
        }
        address.setAddress(qntoh<quint32>(*(reinterpret_cast<const quint32 *>(&pBuf[pos]))));
        pos += 4;
        ret = true;
    } else if (pBuf[pos] == S5_IP_V6) {
        pos++;
        if (buf.size() - pos < 16) {
            QSOCKS5_DEBUG << "need more data for ip6 address";
            return false;
        }
        QIPv6Address add;
        for (int i = 0; i < 16; ++i)
            add[i] = buf[pos++];
        ret = true;
    } else if (pBuf[pos] == S5_DOMAINNAME){
        pos++;
        // domain name.
        ret = false;
    } else {
        QSOCKS5_DEBUG << "invalid address type" << (int)pBuf[pos];
        ret = false;
    }

    if (ret) {
        if (buf.size() - pos < 2) {
            QSOCKS5_DEBUG << "need more data for port";
            return false;
        }
        port = qntoh<quint16>(*(reinterpret_cast<const quint16 *>(&pBuf[pos])));
        pos += 2;
    }

    if (ret) {
        QSOCKS5_DEBUG << "got [" << address << ":" << port << "]";
        *pAddress = address;
        *pPort = port;
        *pPos = pos;
    }

    return ret;
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
    if (msecs == -1)
        return -1;

    int timeout = msecs - elapsed;
    return timeout < 0 ? 0 : timeout;
}


struct QSocks5Data
{
    QTcpSocket *controlSocket;
    QSocks5Authenticator *authenticator;
};

struct QSocks5ConnectData : public QSocks5Data
{
    QByteArray readBuffer;
};

struct QSocks5BindData : public QSocks5Data
{
    QHostAddress localAddress;
    quint16 localPort;
    QHostAddress peerAddress;
    quint16 peerPort;
    QDateTime timeStamp;
};

struct QSocks5RevivedDatagram
{
    QByteArray data;
    QHostAddress address;
    quint16 port;
};

struct QSocks5UdpAssociateData : public QSocks5Data
{
    QUdpSocket *udpSocket;
    QHostAddress associateAddress;
    quint16 associatePort;
    QQueue<QSocks5RevivedDatagram> pendingDatagrams;
};

// needs to be thread safe
class QSocks5BindStore : public QObject
{
public:
    QSocks5BindStore();
    ~QSocks5BindStore();

    int add(QSocks5BindData *bindData);
    bool contains(int socketDescriptor);
    QSocks5BindData *retive(int socketDescriptor);

protected:
    void timerEvent(QTimerEvent * event);

    int sweepTimerId;

    //socket descriptor, data, timestamp
    QHash<int, QSocks5BindData *> store;
};

Q_GLOBAL_STATIC(QSocks5BindStore, socks5BindStore);

QSocks5BindStore::QSocks5BindStore()
    : sweepTimerId(-1)
{
}

QSocks5BindStore::~QSocks5BindStore()
{
}

int QSocks5BindStore::add(QSocks5BindData *bindData)
{
    if (store.contains(bindData->controlSocket->socketDescriptor())) {
        qDebug() << "shit delete it";
    }
    bindData->timeStamp = QDateTime::currentDateTime();
    store.insert(bindData->controlSocket->socketDescriptor(), bindData);
    // start sweep timer if not started
    if (sweepTimerId == -1)
        sweepTimerId = startTimer(60000);
    return bindData->controlSocket->socketDescriptor();
}

bool QSocks5BindStore::contains(int socketDescriptor)
{
    return store.contains(socketDescriptor);
}

QSocks5BindData *QSocks5BindStore::retive(int socketDescriptor)
{
    QSocks5BindData *bindData = store.value(socketDescriptor, 0);
    if (!bindData) {
        qDebug() << "shit what happened";
    }
    // stop the sweep timer if not needed
    if (store.isEmpty()) {
        killTimer(sweepTimerId);
        sweepTimerId = -1;
    }
    return bindData;
}

void QSocks5BindStore::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == sweepTimerId) {
        QSOCKS5_DEBUG << "QSocks5BindStore performing sweep";
        QMutableHashIterator<int, QSocks5BindData *> it(store);
        while (it.hasNext()) {
            it.next();
            if (it.value()->timeStamp.secsTo(QDateTime::currentDateTime()) > 350) {
                QSOCKS5_DEBUG << "QSocks5BindStore removing JJJJ";
                it.remove();
            }
        }
    }
}

QSocks5Authenticator::QSocks5Authenticator()
{
}

QSocks5Authenticator::~QSocks5Authenticator()
{
}

char QSocks5Authenticator::methodId()
{
    return 0x00;
}

bool QSocks5Authenticator::beginAuthenticate(QTcpSocket *socket, bool *completed)
{
    Q_UNUSED(socket);
    *completed = true;
    return true;
}

bool QSocks5Authenticator::continueAuthenticate(QTcpSocket *socket, bool *completed)
{
    Q_UNUSED(socket);
    *completed = true;
    return true;
}

bool QSocks5Authenticator::seal(const QByteArray buf, QByteArray *sealedBuf)
{
    *sealedBuf = buf;
    return true;
}

bool QSocks5Authenticator::unSeal(const QByteArray sealedBuf, QByteArray *buf)
{
    *buf = sealedBuf;
    return true;
}

bool QSocks5Authenticator::unSeal(QTcpSocket *sealedSocket, QByteArray *buf)
{
    return unSeal(sealedSocket->readAll(), buf);
}

QSocks5PasswordAuthenticator::QSocks5PasswordAuthenticator(const QString &userName, const QString &password)
{
    this->userName = userName;
    this->password = password;
}

char QSocks5PasswordAuthenticator::methodId()
{
    return 0x02;
}

bool QSocks5PasswordAuthenticator::beginAuthenticate(QTcpSocket *socket, bool *completed)
{
    *completed = false;
    QByteArray buf = "aap1ap1";
    buf[0] = 0x01;
    buf[1] = 0x02;
    buf[4] = 0x02;
    return socket->write(buf) == buf.size();
}

bool QSocks5PasswordAuthenticator::continueAuthenticate(QTcpSocket *socket, bool *completed)
{
    *completed = false;

    if (socket->bytesAvailable() < 2)
        return true;

    QByteArray buf = socket->read(2);
    if (buf.at(0) == 0x01) {
        *completed = true;
        return buf.at(1) == 0x00;
    }
    return false;
}



QSocks5SocketEnginePrivate::QSocks5SocketEnginePrivate()
    : socks5State(unInitialized)
    , readNotificationEnabled(false)
    , writeNotificationEnabled(false)
    , exceptNotificationEnabled(false)
    , data(0)
    , connectData(0)
    , udpData(0)
    , bindData(0)
    , readNotificationActivated(false)
    , writeNotificationActivated(false)
{
    mode = NoMode;
}

QSocks5SocketEnginePrivate::~QSocks5SocketEnginePrivate()
{
}

void QSocks5SocketEnginePrivate::initialize(Socks5Mode socks5Mode)
{
    Q_Q(QSocks5SocketEngine);

    mode = socks5Mode;
    if (mode == ConnectMode) {
        connectData = new QSocks5ConnectData;
        data = connectData;
    } else if (mode == UdpAssociateMode) {
        udpData = new QSocks5UdpAssociateData;
        data = udpData;
        udpData->udpSocket = new QUdpSocket(q);
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        udpData->udpSocket->setProxy(proxy);
        QObject::connect(udpData->udpSocket, SIGNAL(readyRead()), q, SLOT(udpSocketReadNotification()));
    } else if (mode == BindMode) {
        bindData = new QSocks5BindData;
        data = bindData;
    }

    data->controlSocket = new QTcpSocket(q);
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    data->controlSocket->setProxy(proxy);
    QObject::connect(data->controlSocket, SIGNAL(connected()), q, SLOT(controlSocketConnected()));
    QObject::connect(data->controlSocket, SIGNAL(readyRead()), q, SLOT(controlSocketReadNotification()));
    QObject::connect(data->controlSocket, SIGNAL(bytesWritten(qint64)), q, SLOT(controlSocketBytesWritten()));
    QObject::connect(data->controlSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
                     q, SLOT(controlSocketError(QAbstractSocket::SocketError)));
    //### this should be some where else
    data->authenticator = new QSocks5Authenticator();
}

void QSocks5SocketEnginePrivate::parseAuthenticationMethodReply()
{
    // not enough data to begin
    if (data->controlSocket->bytesAvailable() < 2)
        return;
    
    QByteArray buf(2, 0);
    if (data->controlSocket->read(buf.data(), 2) != 2) {
        qDebug() << "serrious error i gues1";
        return;
    }
    if (buf.at(0) != S5_VERSION_5) {
        qDebug() << " serrios error i gues2";
    }
    if (buf.at(1) == 0xFF) {
        // set local error / error string and fire write notification
        //setError(QAbstractSocket::SocketAccessError, "Socks5 did not like the auth methods");
        //### fire notfiy and bla
        qDebug() << "error i gues3";
        return;
    }
    if (buf.at(1) == 0x00) {
        // use defulat no auth
        QSOCKS5_DEBUG << "no auth required";
    }
    // find the correct auth
    if (buf.at(1) != data->authenticator->methodId()) {
        qDebug() << "error i gues2";
    }
    bool AuthComplete = false;
    if (!data->authenticator->beginAuthenticate(data->controlSocket, &AuthComplete)) {
        // authentication error
        qDebug() << "auth error";
        return;
    }
    if (AuthComplete) {
        sendRequestMethod();
        return;
    }
    socks5State = Authenticating;
}

void QSocks5SocketEnginePrivate::parseAuthenticatingReply()
{
    bool authComplete = false;
    if (!data->authenticator->continueAuthenticate(data->controlSocket, &authComplete)) {
        // authentication error
        qDebug() << "auth error";
        return;
    }
    if (authComplete)
        sendRequestMethod();
}

void QSocks5SocketEnginePrivate::sendRequestMethod()
{
    QHostAddress address;
    quint16 port;
    char command = 0;
    if (mode == ConnectMode) {
        command = S5_CONNECT;
        address = peerAddress;
        port = peerPort;
    } else if (mode == BindMode) {
        command = S5_BIND;
        address = localAddress;
        port = localPort;
    } else {
        command = S5_UDP_ASSOCIATE;
        address = localAddress;
        port = localPort;
    }

    QByteArray buf;
    buf.reserve(270); // big enough for domain name;
    int pos = 0;
    buf[pos++] = S5_VERSION_5;
    buf[pos++] = command;
    buf[pos++] = 0x00;
    if (!qt_socks5_set_host_address_and_port(address, port, &buf, &pos)) {
        QSOCKS5_DEBUG << "error setting address" << address << " : " << port;
        //### set error code ....
        return;
    }
    QSOCKS5_DEBUG << "sending" << dump(buf);
    QByteArray sealedBuf;
    if (!data->authenticator->seal(buf, &sealedBuf))
        qDebug() << "shit";
    data->controlSocket->write(sealedBuf);
    data->controlSocket->flush();
    socks5State = RequestMethodSent;
}

void QSocks5SocketEnginePrivate::parseRequestMethodReply()
{
    QByteArray inBuf;
    if (!data->authenticator->unSeal(data->controlSocket, &inBuf)) {
        // ### check error and not just not enough data
        QSOCKS5_DEBUG << "unSeal failed, needs more data";
        return;
    }
    QSOCKS5_DEBUG << dump(inBuf);
    int pos = 0;
    const char *buf = inBuf.constData();
    if (inBuf.size() < 2) {
        QSOCKS5_DEBUG << "need more data for request reply header .. put this data somewhere";
        return;
    }
    if (buf[pos++] != S5_VERSION_5) {
        QSOCKS5_DEBUG << "totaly lost";
    }
    if (buf[pos++] != S5_SUCCESS ) {
        socks5Error = Socks5Error(buf[pos-1]);
        socks5State = RequestError;
        socks5ErrorString = s5RequestErrorToString(socks5Error);
        QSOCKS5_DEBUG <<  "Request error :" << s5RequestErrorToString(socks5Error);
        emitWriteNotification();
        return;
    }
    if (buf[pos++] != 0x00) {
        QSOCKS5_DEBUG << "totaly lost";
    }
    if (!qt_socks5_get_host_address_and_port(inBuf, &localAddress, &localPort, &pos)) {
        QSOCKS5_DEBUG << "error getting address";
        //### set error code ....
        return;
    }

    // need a better place to keep this stuff and any others untill connect called again
    // should use peek
    inBuf.remove(0, pos);
    for (int i = inBuf.size() - 1; i >= 0 ; --i)
        data->controlSocket->ungetChar(inBuf.at(i));

    socks5State = RequestSuccess; 

    // fire writeNotifier for connect and wait for the next call to conectTOHost
    if (mode == ConnectMode)
        emitWriteNotification();
}

void QSocks5SocketEnginePrivate::parseNewConnection()
{
    // only emit readyRead if in listening state ...
    QByteArray inBuf;
    if (!data->authenticator->unSeal(data->controlSocket, &inBuf)) {
        // ### check error and not just not enough data
        QSOCKS5_DEBUG << "unSeal failed, needs more data";
        return;
    }
    QSOCKS5_DEBUG << dump(inBuf);
    int pos = 0;
    const char *buf = inBuf.constData();
    if (inBuf.length() < 2) {
        QSOCKS5_DEBUG << "need more data for request reply header .. put this data somewhere";
        return;
    }
    if (buf[pos++] != S5_VERSION_5) {
        QSOCKS5_DEBUG << "totaly lost";
    }
    if (buf[pos++] != S5_SUCCESS) {
        QSOCKS5_DEBUG <<  "Request error :" << s5RequestErrorToString(buf[pos-1]);
        socks5State = BindError;
        socks5Error = Socks5Error(buf[pos-1]);
        socks5ErrorString = s5RequestErrorToString(socks5Error);
        // #### now what
        return;
    }
    if (buf[pos++] != 0x00) {
        QSOCKS5_DEBUG << "totaly lost";
    }
    if (!qt_socks5_get_host_address_and_port(inBuf, &bindData->peerAddress, &bindData->peerPort, &pos)) {
        QSOCKS5_DEBUG << "error getting address";
        //### set error code ....
        return;
    }

    // need a better p�lace to keep this stuff and any others untill connect called again
    // should use peek
    inBuf.remove(0, pos);
    for (int i = inBuf.size() - 1; i >= 0 ; --i)
        data->controlSocket->ungetChar(inBuf.at(i));

    // got a successfull reply
    socks5State = BindSuccess; 
    if (socketState == QAbstractSocket::ListeningState)
        emitReadNotification();
}

void QSocks5SocketEnginePrivate::emitReadNotification()
{
    Q_Q(QSocks5SocketEngine);
    readNotificationActivated = true;
    if (readNotificationEnabled) {
        //###qDebug() << this << "queing readNotification";
        emit q->readNotification();
        //###QMetaObject::invokeMethod(q, "readNotification", Qt::QueuedConnection);
    }
}

void QSocks5SocketEnginePrivate::emitWriteNotification()
{
    Q_Q(QSocks5SocketEngine);
    writeNotificationActivated = true;
    if (writeNotificationEnabled)
        QMetaObject::invokeMethod(q, "writeNotification", Qt::QueuedConnection);
}

QSocks5SocketEngine::QSocks5SocketEngine(QObject *parent)
:QAbstractSocketEngine(*new QSocks5SocketEnginePrivate(), parent)
{
}

QSocks5SocketEngine::~QSocks5SocketEngine()
{
    Q_D(QSocks5SocketEngine);
    
    if (d->data) {
        delete d->data->authenticator;
        delete d->data->controlSocket;
    }
    if (d->connectData)
        delete d->connectData;
    if (d->udpData) {
        delete d->udpData->udpSocket;
        delete d->udpData;
    }
    if (d->bindData)
        delete d->bindData;
}

bool QSocks5SocketEngine::initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol)
{
    Q_D(QSocks5SocketEngine);

    d->socketType = type;
    d->socketProtocol = protocol;

    return true; 
}

bool QSocks5SocketEngine::initialize(int socketDescriptor, QAbstractSocket::SocketState socketState)
{
    Q_D(QSocks5SocketEngine);

    // this is only valid for the other side of a bind, nothing else is supported

    if (socketState != QAbstractSocket::ConnectedState) {
        //### must be connected state ???
        return false;
    }

    QSocks5BindData *bindData = socks5BindStore()->retive(socketDescriptor);
    if (bindData) {
               
        d->socketState = socketState;
        d->socketType = QAbstractSocket::TcpSocket;
        d->connectData = new QSocks5ConnectData;
        d->data = d->connectData;
        d->mode = QSocks5SocketEnginePrivate::ConnectMode;
        d->data->controlSocket = bindData->controlSocket;
        bindData->controlSocket = 0;
        d->data->controlSocket->setParent(this);
        QObject::connect(d->data->controlSocket, SIGNAL(connected()), this, SLOT(controlSocketConnected()));
        QObject::connect(d->data->controlSocket, SIGNAL(readyRead()), this, SLOT(controlSocketReadNotification()));
        QObject::connect(d->data->controlSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(controlSocketBytesWritten()));
        QObject::connect(d->data->controlSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(controlSocketError(QAbstractSocket::SocketError)));
        d->data->authenticator = bindData->authenticator;
        bindData->authenticator = 0;
        d->localPort = bindData->localPort;
        d->localAddress = bindData->localAddress;
        d->peerPort = bindData->peerPort;
        d->peerAddress = bindData->peerAddress;
        delete bindData;

        //### if there is data then emit readyRead in single shoot

        return true;
    }
    return false; 
}

void QSocks5SocketEngine::setProxy(const QNetworkProxy &networkProxy)
{
    Q_D(QSocks5SocketEngine);
    d->proxyInfo = networkProxy;
}

int QSocks5SocketEngine::socketDescriptor() const
{
    Q_D(const QSocks5SocketEngine);
    return d->data->controlSocket ? d->data->controlSocket->socketDescriptor() : -1; 
}

bool QSocks5SocketEngine::isValid() const
{
    Q_D(const QSocks5SocketEngine);
    return d->socketType != QAbstractSocket::UnknownSocketType 
           && d->socketProtocol != QAbstractSocket::UnknownNetworkLayerProtocol
           && d->socks5State != QSocks5SocketEnginePrivate::ControlSocketError
           && d->socks5State != QSocks5SocketEnginePrivate::SocksError;
}

bool QSocks5SocketEngine::connectToHost(const QHostAddress &address, quint16 port)
{
    Q_D(QSocks5SocketEngine);

    if (!d->data) {
        if (socketType() == QAbstractSocket::TcpSocket) {
            d->initialize(QSocks5SocketEnginePrivate::ConnectMode);
        } else if (socketType() == QAbstractSocket::UdpSocket) {
            d->initialize(QSocks5SocketEnginePrivate::UdpAssociateMode);
            // all udp needs to be bound
            if (!bind(QHostAddress("0.0.0.0"), 0))
                return false;
            d->peerAddress = address;
            d->peerPort = port;
            setState(QAbstractSocket::ConnectedState);
            return true;
        } else {
            //### something invalied
            return false;
        }
    }
    if (d->socks5State == QSocks5SocketEnginePrivate::unInitialized && d->socketState != QAbstractSocket::ConnectingState) {
        setPeerAddress(address);
        setPeerPort(port);
        setState(QAbstractSocket::ConnectingState);
        d->data->controlSocket->connectToHost(d->proxyInfo.address(), d->proxyInfo.port());
        return false;
    } else if (d->socks5State == QSocks5SocketEnginePrivate::RequestSuccess) {
        setState(QAbstractSocket::ConnectedState);
        d->socks5State = QSocks5SocketEnginePrivate::Connected;
        // check for pending data
        if (d->data->controlSocket->bytesAvailable())
            controlSocketReadNotification();
        return true;
    } else if (d->socks5State == QSocks5SocketEnginePrivate::RequestError) {
        setError(s5RAsSocketError(d->socks5Error), makeErrorString(d->socks5ErrorString));
        setState(QAbstractSocket::UnconnectedState);
        return false;
    } else if (d->socks5State == QSocks5SocketEnginePrivate::ConnectError) {
        setError(d->data->controlSocket->error(), d->data->controlSocket->errorString());
        setState(QAbstractSocket::UnconnectedState);
        return false;
    } else {
        qDebug() << "unexpected call to contectToHost";  
    }
    return false; 
}

void QSocks5SocketEngine::controlSocketConnected()
{
    Q_D(QSocks5SocketEngine);
    QSOCKS5_DEBUG << "controlSocketConnected";
    QByteArray buf(3, 0);
    buf[0] = S5_VERSION_5;
    buf[1] = 0x01;
    buf[2] = d->data->authenticator->methodId();
    d->data->controlSocket->write(buf);
    d->socks5State = QSocks5SocketEnginePrivate::AuthenticationMethodsSent;
}

void QSocks5SocketEngine::controlSocketReadNotification()
{
    Q_D(QSocks5SocketEngine);
    QSOCKS5_DEBUG << "controlSocketReadNotification ... socks5 state" <<  s5StateToString(d->socks5State);
    switch (d->socks5State) {
        case QSocks5SocketEnginePrivate::AuthenticationMethodsSent: 
            d->parseAuthenticationMethodReply();
            break;
        case QSocks5SocketEnginePrivate::Authenticating:
            d->parseAuthenticatingReply();
            break;
        case QSocks5SocketEnginePrivate::RequestMethodSent: 
            d->parseRequestMethodReply();
            break;
        case QSocks5SocketEnginePrivate::RequestSuccess: 
            // only get here if command is bind 
            d->parseNewConnection();
            break;
        case QSocks5SocketEnginePrivate::Connected: {
            QByteArray buf;
            if (!d->data->authenticator->unSeal(d->data->controlSocket, &buf))
                qDebug() << "shit  not enough ??";
            if (buf.size()) {
                QSOCKS5_DEBUG << dump(buf);
                d->connectData->readBuffer += buf;
                d->emitReadNotification();
            }
        }
        default:
            QSOCKS5_DEBUG << "why a controlSocketReadNotification ????";
            break;
    };
}

void QSocks5SocketEngine::controlSocketBytesWritten()
{
    Q_D(QSocks5SocketEngine);

    QSOCKS5_DEBUG << "controlSocketBytesWritten";

    if (d->socks5State != QSocks5SocketEnginePrivate::Connected 
        || (d->mode == QSocks5SocketEnginePrivate::ConnectMode 
        && d->data->controlSocket->bytesToWrite()))
        return;
    d->emitWriteNotification();
    d->writeNotificationActivated = false;
}

void QSocks5SocketEngine::controlSocketError(QAbstractSocket::SocketError error)
{
    Q_D(QSocks5SocketEngine);
    QSOCKS5_DEBUG << "controlSocketErrornnnn" << error << d->data->controlSocket->errorString();

    if (error == QAbstractSocket::RemoteHostClosedError) {
        d->socks5State = QSocks5SocketEnginePrivate::ControlSocketError;
        // clear the read buffer in connect mode so that bytes available returns 0;
        if (d->mode == QSocks5SocketEnginePrivate::ConnectMode)
            d->connectData->readBuffer.clear();
        d->emitReadNotification();
    } else if (error == QAbstractSocket::ConnectionRefusedError
        || error == QAbstractSocket::HostNotFoundError) {
        d->socks5State = QSocks5SocketEnginePrivate::ConnectError;
        d->emitWriteNotification();
    }
}

void QSocks5SocketEngine::udpSocketReadNotification()
{
    Q_D(QSocks5SocketEngine);
    // check some state stuff
    if (!d->udpData->udpSocket->hasPendingDatagrams()) {
        qDebug() << "false read ??";
        return;
    }

    while (d->udpData->udpSocket->hasPendingDatagrams()) {
        QByteArray sealedBuf(d->udpData->udpSocket->pendingDatagramSize(), 0);
        QSOCKS5_DEBUG << "new datagram";
        d->udpData->udpSocket->readDatagram(sealedBuf.data(), sealedBuf.size());
        QByteArray inBuf;
        if (!d->data->authenticator->unSeal(sealedBuf, &inBuf))
            qDebug() << "failed unseal";
        QSOCKS5_DEBUG << dump(inBuf);

        int pos = 0;
        const char *buf = inBuf.constData();
        if (inBuf.size() < 4)
            qDebug() << "we are fucked";
    
        QSocks5RevivedDatagram datagram;
        if (buf[pos++] != 0 || buf[pos++] != 0)
            qDebug() << "in valid";
        if (buf[pos++] != 0) //### add fragmentation reading support
            qDebug() << "dont support fragmentation yet";
        if (!qt_socks5_get_host_address_and_port(inBuf, &datagram.address, &datagram.port, &pos)) {
        }
        datagram.data = QByteArray(inBuf.size() - pos, buf[pos]);
        d->udpData->pendingDatagrams.enqueue(datagram);
    }
    d->emitReadNotification();
}

bool QSocks5SocketEngine::bind(const QHostAddress &address, quint16 port)
{
    Q_D(QSocks5SocketEngine);

    // when bind wee will block until the bind is finished as the info from the proxy server is needed
       
    if (!d->data) {
        if (socketType() == QAbstractSocket::TcpSocket) {
            d->initialize(QSocks5SocketEnginePrivate::BindMode);
        } else if (socketType() == QAbstractSocket::UdpSocket) {
            d->initialize(QSocks5SocketEnginePrivate::UdpAssociateMode);
        } else {
            //### something invalid
            return false;
        }
    }

    if (d->mode == QSocks5SocketEnginePrivate::UdpAssociateMode) {
        if (!d->udpData->udpSocket->bind(address, port)) {
            qDebug() << "dead";
            return false;
        }
        d->localAddress = d->udpData->udpSocket->localAddress();
        d->localPort = d->udpData->udpSocket->localPort();
    } else if (d->mode == QSocks5SocketEnginePrivate::BindMode) {
        d->localAddress = address;
        d->localPort = port;
    } else {
        //### something invalid
        return false;
    }

    int msecs = 3000;
    QTime stopWatch;
    stopWatch.start();
    d->data->controlSocket->connectToHost(d->proxyInfo.address(), d->proxyInfo.port());
    if (!d->data->controlSocket->waitForConnected(qt_timeout_value(msecs, stopWatch.elapsed()))) {
        //### ?
        return false;
    }
    while (d->data->controlSocket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
        // check error
        if (d->socks5State == QSocks5SocketEnginePrivate::RequestSuccess) {
            setState(QAbstractSocket::BoundState);
            if (d->mode == QSocks5SocketEnginePrivate::UdpAssociateMode) {
                d->udpData->associateAddress = d->localAddress;
                d->localAddress = QHostAddress();
                d->udpData->associatePort = d->localPort;
                d->localPort = 0;
                QUdpSocket dummy;
                if (!dummy.bind()
                    || writeDatagram(0,0, d->data->controlSocket->localAddress(), dummy.localPort()) != 0
                    || !dummy.waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))
                    || dummy.readDatagram(0,0, &d->localAddress, &d->localPort) != 0) {
                    //### reset and error
                    return false;
                }
            }
            return true;
        }
        QSOCKS5_DEBUG << "looping";
    }
    // set error string = socks 5 timeout
///###    delete d->udpSocket;
///###    d->udpSocket = 0;
    return false; 
}


bool QSocks5SocketEngine::listen()
{
    Q_D(QSocks5SocketEngine);

    // check that we are in bound and then go to listening.
    if (d->socketState == QAbstractSocket::BoundState) {
        d->socketState = QAbstractSocket::ListeningState;
        // if already have accept then do a singleshot read notification
        return true;
    }
    return false; 
}

int QSocks5SocketEngine::accept()
{
    Q_D(QSocks5SocketEngine);
    // check we are listing ---

    QSOCKS5_DEBUG << "accept()";

    if (d->socks5State == QSocks5SocketEnginePrivate::BindSuccess) {
        QSOCKS5_DEBUG << "BindSuccess";
        d->data->controlSocket->disconnect();
        d->data->controlSocket->setParent(0);
        d->bindData->localAddress = d->localAddress;
        d->bindData->localPort = d->localPort;
        int sd = socks5BindStore()->add(d->bindData);
        d->data = 0;
        d->bindData = 0;
        //### do something about this socket layer ... set it closed and an error about why ...
        // reset state and local port/address
        return sd;
    } else if (d->socks5State == QSocks5SocketEnginePrivate::BindError) {
        // what now
    } else if (d->socks5State == QSocks5SocketEnginePrivate::RequestSuccess) {
        // accept was called to early ...
    } else {
        // what the f.,'
    }
    return -1; 
}

void QSocks5SocketEngine::close()
{
    Q_D(QSocks5SocketEngine);
    if (d->data && d->data->controlSocket) {
        if (d->data->controlSocket->state() == QAbstractSocket::ConnectedState) {
            int msecs = 100;
            QTime stopWatch;
            stopWatch.start();
            while (!d->data->controlSocket->bytesToWrite()) { 
               if (!d->data->controlSocket->waitForBytesWritten(qt_timeout_value(msecs, stopWatch.elapsed())))
                   break;
            }
        }
        d->data->controlSocket->close();
    }
    if (d->udpData && d->udpData->udpSocket)
        d->udpData->udpSocket->close();
}

qint64 QSocks5SocketEngine::bytesAvailable() const
{
    Q_D(const QSocks5SocketEngine);
    if (d->mode == QSocks5SocketEnginePrivate::ConnectMode)
        return d->connectData->readBuffer.size();
    else if (d->mode == QSocks5SocketEnginePrivate::UdpAssociateMode
             && !d->udpData->pendingDatagrams.isEmpty())
        return d->udpData->pendingDatagrams.first().data.size();
    return 0; 
}

qint64 QSocks5SocketEngine::read(char *data, qint64 maxlen)
{
    Q_D(QSocks5SocketEngine);
    QSOCKS5_DEBUG << "read ...";
    if (socketType() == QAbstractSocket::TcpSocket) {
        if (d->connectData->readBuffer.size() == 0 && maxlen != 0) {
            //imitate remote closed
            close();
            setError(QAbstractSocket::RemoteHostClosedError, 
                        "Remote host closed connection###");
            setState(QAbstractSocket::UnconnectedState);
            return -1;
        }
        qint64 copy = qMin<qint64>(d->connectData->readBuffer.size(), maxlen);
        memcpy(data, d->connectData->readBuffer.constData(), copy);
        d->connectData->readBuffer.remove(0, copy);
        QSOCKS5_DEBUG << "read" << dump(QByteArray(data, copy));
        return copy; 
    }
    return 0;
}

qint64 QSocks5SocketEngine::write(const char *data, qint64 len)
{
    Q_D(QSocks5SocketEngine);
    QSOCKS5_DEBUG << "write" << dump(QByteArray(data, len));
    
    if (d->mode == QSocks5SocketEnginePrivate::ConnectMode) {
                
        int msecs = 10;
        QTime stopWatch;
        stopWatch.start();
        qint64 totalWritten = 0;
        
        while (!d->data->controlSocket->bytesToWrite() 
               && totalWritten < len 
               && stopWatch.elapsed() < msecs) {

            QByteArray buf(data + totalWritten, qMin<int>(len - totalWritten, 49152));
            QByteArray sealedBuf;
            if (!d->data->authenticator->seal(buf, &sealedBuf))
                qDebug() << "shit";
            if (d->data->controlSocket->write(sealedBuf) != sealedBuf.size()) {
                return -1; //### ?????
            }
            totalWritten += buf.size();
            while(d->data->controlSocket->bytesToWrite()) {
                if (!d->data->controlSocket->waitForBytesWritten(qt_timeout_value(msecs, stopWatch.elapsed())))
                   break;
            }
        }
        QSOCKS5_DEBUG << "wrote" << totalWritten;
        return totalWritten;
   } else if (d->mode == QSocks5SocketEnginePrivate::UdpAssociateMode) {
        // send to connected address
        return writeDatagram(data, len, d->peerAddress, d->peerPort);
    }
    //### set an error ???
    return -1;
}

qint64 QSocks5SocketEngine::readDatagram(char *data, qint64 maxlen, QHostAddress *addr,
                                        quint16 *port)
{
    Q_D(QSocks5SocketEngine);
    if (d->udpData->pendingDatagrams.isEmpty())
        return 0;

    QSocks5RevivedDatagram datagram = d->udpData->pendingDatagrams.dequeue();
    int copyLen = qMin<int>(maxlen, datagram.data.size());
    memcpy(data, datagram.data.constData(), copyLen);
    if (addr)
        *addr = datagram.address;
    if (port)
        *port = datagram.port;
    return copyLen; 
}

qint64 QSocks5SocketEngine::writeDatagram(const char *data, qint64 len, const QHostAddress &address,
                                         quint16 port)
{
    Q_D(QSocks5SocketEngine);

    QByteArray outBuf;
    outBuf.reserve(270 + len);
    char *buf = outBuf.data();
    int pos = 0;
    buf[pos++] = 0x00;
    buf[pos++] = 0x00;
    buf[pos++] = 0x00;
    if (!qt_socks5_set_host_address_and_port(address, port, &outBuf, &pos)) {
    }
    outBuf += QByteArray(data, len);
    QSOCKS5_DEBUG << "sending" << dump(buf);
    QByteArray sealedBuf;
    if (!d->data->authenticator->seal(outBuf, &sealedBuf))
        qDebug() << "shit";
    if (!d->udpData->udpSocket->writeDatagram(sealedBuf, d->udpData->associateAddress, d->udpData->associatePort))
        qDebug() << "no write datagram";
   
    return len; 
}

bool QSocks5SocketEngine::hasPendingDatagrams() const
{
    Q_D(const QSocks5SocketEngine);
    Q_INIT_CHECK(false);
    return !d->udpData->pendingDatagrams.isEmpty(); 
}

qint64 QSocks5SocketEngine::pendingDatagramSize() const
{
    Q_D(const QSocks5SocketEngine);
    if (!d->udpData->pendingDatagrams.isEmpty())
        return d->udpData->pendingDatagrams.head().data.size();
    return 0; 
}

int QSocks5SocketEngine::option(SocketOption option) const
{
    Q_UNUSED(option);
    return -1; 
}

bool QSocks5SocketEngine::setOption(SocketOption option, int value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
    return false; 
}

bool QSocks5SocketEngine::waitForRead(int msecs, bool *timedOut) const
{
    Q_D(const QSocks5SocketEngine);
    QSOCKS5_DEBUG << "waitForRead" << msecs;

    d->readNotificationActivated = false;
    
    QTime stopWatch;
    stopWatch.start();

    if (socketType() == QAbstractSocket::TcpSocket) {
        // check for pending data
        if (d->data->controlSocket->bytesAvailable())
            const_cast<QSocks5SocketEngine*>(this)->controlSocketReadNotification();
    
        while (!d->readNotificationActivated && d->data->controlSocket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
            QSOCKS5_DEBUG << "looping";
        }
        if (timedOut && d->data->controlSocket->error() == QAbstractSocket::SocketTimeoutError)
        *timedOut = true;
    } else {
        // what about if the tcp socket is disconnected ...
        while (!d->readNotificationActivated && d->udpData->udpSocket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
            QSOCKS5_DEBUG << "looping";
        }
        if (timedOut && d->udpData->udpSocket->error() == QAbstractSocket::SocketTimeoutError)
        *timedOut = true;
    }
    
    
    bool ret = d->readNotificationActivated;
    d->readNotificationActivated = false;
    
    QSOCKS5_DEBUG << "waitForrRead returned" << ret;
    return ret; 
}


bool QSocks5SocketEngine::waitForWrite(int msecs, bool *timedOut) const
{
    Q_D(const QSocks5SocketEngine);
    QSOCKS5_DEBUG << "waitForWrite" << msecs;

    if (d->socketState == QAbstractSocket::ConnectingState) {
        d->writeNotificationActivated = false;

        QSOCKS5_DEBUG << "waitForWrite ... waiting for connected";
        QTime stopWatch;
        stopWatch.start();

        if (!d->data->controlSocket->waitForConnected(qt_timeout_value(msecs, stopWatch.elapsed()))) {
            qDebug() << "failed to connect";
            if (timedOut && d->data->controlSocket->error() == QAbstractSocket::SocketTimeoutError)
                *timedOut = true;
            return false; // ???
        }
        QSOCKS5_DEBUG << "waitForWrite ... waiting for proxy init" << msecs;
        while (!d->writeNotificationActivated 
               && d->data->controlSocket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
            QSOCKS5_DEBUG << "looping";
        }
        if (timedOut && d->data->controlSocket->error() == QAbstractSocket::SocketTimeoutError)
            *timedOut = true;
       
        bool ret = d->writeNotificationActivated;
        d->writeNotificationActivated = false;

        return ret;
    }

    // probably just return true unless we are not set up ??
    if (d->socketState == QAbstractSocket::ConnectedState) {
        if (d->mode == QSocks5SocketEnginePrivate::ConnectMode) {
            //### check for time out;
            while(d->data->controlSocket->bytesToWrite())
                d->data->controlSocket->waitForBytesWritten();
        }
        return true;
    }
    return false;
}


bool QSocks5SocketEngine::waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                                            bool checkRead, bool checkWrite,
                                            int msecs, bool *timedOut) const
{
    Q_UNUSED(checkRead);
    if (!checkWrite) {
        bool canRead = waitForRead(msecs, timedOut);
        if (readyToRead)
            *readyToRead = canRead;
        return canRead;
    }
    
    bool canWrite = waitForWrite(msecs, timedOut);
    if (readyToWrite)
        *readyToWrite = canWrite;
    return canWrite;
}

bool QSocks5SocketEngine::isReadNotificationEnabled() const
{
    Q_D(const QSocks5SocketEngine);
    return d->readNotificationEnabled; 
}

void QSocks5SocketEngine::setReadNotificationEnabled(bool enable)
{
    Q_D(QSocks5SocketEngine);
    d->readNotificationEnabled = enable;
}

bool QSocks5SocketEngine::isWriteNotificationEnabled() const
{
    Q_D(const QSocks5SocketEngine);
    return d->writeNotificationEnabled;
}

void QSocks5SocketEngine::setWriteNotificationEnabled(bool enable)
{
    Q_D(QSocks5SocketEngine);
    d->writeNotificationEnabled = enable;
    if (enable && d->socketState == QAbstractSocket::ConnectedState) {
        if (d->mode == QSocks5SocketEnginePrivate::ConnectMode && d->data->controlSocket->bytesToWrite())
            return; // will be emitted as a result of bytes written
       d->emitWriteNotification();
       d->writeNotificationActivated = false;
    }
}

bool QSocks5SocketEngine::isExceptionNotificationEnabled() const
{
    Q_D(const QSocks5SocketEngine);
    return d->exceptNotificationEnabled;
}

void QSocks5SocketEngine::setExceptionNotificationEnabled(bool enable)
{
    Q_D(QSocks5SocketEngine);
    d->exceptNotificationEnabled = enable;
}

QAbstractSocketEngine *QSocks5SocketEngineHandler::createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent)
{
    Q_UNUSED(socketType);

    QSOCKS5_DEBUG << "createSocketEngine" << address;

    QNetworkProxy proxy;
    // find proxy info
    if (qobject_cast<QAbstractSocket *>(parent)) {
        QAbstractSocket *abstractSocket = qobject_cast<QAbstractSocket *>(parent);
        if (abstractSocket->proxy().type() != QNetworkProxy::AutoProxy)
            proxy = abstractSocket->proxy();
    } else if (qobject_cast<QTcpServer *>(parent)) {
        QTcpServer *server = qobject_cast<QTcpServer *>(parent);
        if (server->proxy().type() != QNetworkProxy::AutoProxy)
            proxy = server->proxy();
    }
    if (proxy.type() == QNetworkProxy::AutoProxy) {
        proxy = QNetworkProxy::proxy();
    }
    if (proxy.type() == QNetworkProxy::AutoProxy || proxy.type() == QNetworkProxy::NoProxy) {
        QSOCKS5_DEBUG << "not proxying";
        return 0;
    }
    QSOCKS5_DEBUG << "use proxy for" << address;
    QSocks5SocketEngine *engine = new QSocks5SocketEngine(parent);
    engine->setProxy(proxy);
    return engine;
}

QAbstractSocketEngine *QSocks5SocketEngineHandler::createSocketEngine(int socketDescripter, QObject *parent)
{
    QSOCKS5_DEBUG << "createSocketEngine" << socketDescripter;
    if (socks5BindStore()->contains(socketDescripter)) {
        QSOCKS5_DEBUG << "bind store contains" << socketDescripter;
        return new QSocks5SocketEngine(parent);
    }
    return 0;
}

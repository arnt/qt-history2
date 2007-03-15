/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qbuffer.h>
#include <qcoreapplication.h>
#include <qfile.h>
#include <qhttp.h>
#include <qlist.h>
#include <qpointer.h>
#include <qtcpsocket.h>
#include <qtcpserver.h>
#ifdef TEST_QNETWORK_PROXY
# include <QNetworkProxy>
#endif

//TESTED_CLASS=
//TESTED_FILES=network/qhttp.h network/qhttp.cpp

class tst_QHttp : public QObject
{
    Q_OBJECT

public:
    tst_QHttp();
    virtual ~tst_QHttp();


public slots:
    void initTestCase_data();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void constructing();
    void get_data();
    void get();
    void head_data();
    void head();
    void authorization_data();
    void authorization();
    void proxy_data();
    void proxy();
    void proxy2();

    void reconnect();
    void setSocket();
    void unexpectedRemoteClose();
    void pctEncodedPath();
    void caseInsensitiveKeys();

protected slots:
    void stateChanged( int );
    void responseHeaderReceived( const QHttpResponseHeader & );
    void readyRead( const QHttpResponseHeader& );
    void dataSendProgress( int, int );
    void dataReadProgress( int , int );

    void requestStarted( int );
    void requestFinished( int, bool );
    void done( bool );

    void reconnect_state(int state);
    void proxy2_slot();

private:
    QHttp *newHttp();
    void addRequest( QHttpRequestHeader, int );
    bool headerAreEqual( const QHttpHeader&, const QHttpHeader& );

    QHttp *http;

    struct RequestResult
    {
	QHttpRequestHeader req;
	QHttpResponseHeader resp;
	int success;
    };
    QMap< int, RequestResult > resultMap;
    typedef QMap<int,RequestResult>::Iterator ResMapIt;
    QList<int> ids; // helper to make sure that all expected signals are emitted

    int current_id;
    int cur_state;
    int done_success;

    QByteArray readyRead_ba;

    int bytesTotalSend;
    int bytesDoneSend;
    int bytesTotalRead;
    int bytesDoneRead;

    int getId;
    int headId;

    int reconnect_state_connect_count;
};

//#define DUMP_SIGNALS

const int bytesTotal_init = -10;
const int bytesDone_init = -10;

tst_QHttp::tst_QHttp()
{
}

tst_QHttp::~tst_QHttp()
{
}

void tst_QHttp::initTestCase_data()
{
    QTest::addColumn<bool>("setProxy");
    QTest::addColumn<int>("proxyType");  
    
    QTest::newRow("WithoutProxy") << false << 0;
#ifdef TEST_QNETWORK_PROXY
    QTest::newRow("WithSocks5Proxy") << true << int(QNetworkProxy::Socks5Proxy);
#endif
}

void tst_QHttp::initTestCase()
{
}

void tst_QHttp::cleanupTestCase()
{
}

void tst_QHttp::init()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, "fluke.troll.no", 1080));
        }
#endif
    }

    http = 0;

    resultMap.clear();
    ids.clear();

    current_id = 0;
    cur_state = QHttp::Unconnected;
    done_success = -1;

    readyRead_ba = QByteArray();

    bytesTotalSend = bytesTotal_init;
    bytesDoneSend = bytesDone_init;
    bytesTotalRead = bytesTotal_init;
    bytesDoneRead = bytesDone_init;

    getId = -1;
    headId = -1;
}

void tst_QHttp::cleanup()
{
    delete http;
    http = 0;

    QCoreApplication::processEvents();
    
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QNetworkProxy::setApplicationProxy(QNetworkProxy::DefaultProxy);
#endif
    }
}

void tst_QHttp::constructing()
{
    //QHeader
    {
        QHttpRequestHeader header;
        header.addValue("key1", "val1");
        header.addValue("key2", "val2");
        header.addValue("key1", "val3");
        QCOMPARE(header.values().size(), 3);
        QCOMPARE(header.allValues("key1").size(), 2);
        QVERIFY(header.hasKey("key2"));
        QCOMPARE(header.keys().count(), 2);

    }

#if QT_VERSION >= 0x040102
    {
        QHttpResponseHeader header(200);
    }
#endif
}

void tst_QHttp::get_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<uint>("port");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("success");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QByteArray>("res");
    QTest::addColumn<bool>("useIODevice");

    // ### move this into external testdata
    QFile file( "rfc3252.txt" );
    QVERIFY( file.open( QIODevice::ReadOnly ) );
    QByteArray rfc3252 = file.readAll();
    file.close();

    file.setFileName( "trolltech" );
    QVERIFY( file.open( QIODevice::ReadOnly ) );
    QByteArray trolltech = file.readAll();
    file.close();

    // test the two get() modes in one routine
    for ( int i=0; i<2; i++ ) {
	QTest::newRow(QString("path_01_%1").arg(i).toLatin1()) << QString("fluke.troll.no") << 80u
	    << QString("/qtest/rfc3252.txt") << 1 << 200 << rfc3252 << (bool)(i==1);
	QTest::newRow( QString("path_02_%1").arg(i).toLatin1() ) << QString("www.ietf.org") << 80u
	    << QString("/rfc/rfc3252.txt") << 1 << 200 << rfc3252 << (bool)(i==1);

	QTest::newRow( QString("uri_01_%1").arg(i).toLatin1() ) << QString("fluke.troll.no") << 80u
	    << QString("http://fluke/qtest/rfc3252.txt") << 1 << 200 << rfc3252 << (bool)(i==1);
	QTest::newRow( QString("uri_02_%1").arg(i).toLatin1() ) << "www.ietf.org" << 80u
	    << QString("http://www.ietf.org/rfc/rfc3252.txt") << 1 << 200 << rfc3252 << (bool)(i==1);

	QTest::newRow( QString("fail_01_%1").arg(i).toLatin1() ) << QString("foo") << 80u
	    << QString("/qtest/rfc3252.txt") << 0 << 0 << QByteArray() << (bool)(i==1);

	QTest::newRow( QString("failprot_01_%1").arg(i).toLatin1() ) << QString("fluke.troll.no") << 80u
	    << QString("/t") << 1 << 404 << QByteArray() << (bool)(i==1);
	QTest::newRow( QString("failprot_02_%1").arg(i).toLatin1() ) << QString("fluke.troll.no") << 80u
	    << QString("qtest/rfc3252.txt") << 1 << 400 << QByteArray() << (bool)(i==1);

  // doc.trolltech.com uses transfer-encoding=chunked
    /* doc.trolltech.com no longer seams to be using chuncked encodig.
    QTest::newRow( QString("chunked_01_%1").arg(i).toLatin1() ) << QString("test.troll.no") << 80u
	    << QString("/") << 1 << 200 << trolltech << (bool)(i==1);
    */
	QTest::newRow( QString("chunked_02_%1").arg(i).toLatin1() ) << QString("fluke.troll.no") << 80u
	    << QString("/qtest/cgi-bin/rfc.cgi") << 1 << 200 << rfc3252 << (bool)(i==1);
    }
}

void tst_QHttp::get()
{
    // for the overload that takes a QIODevice
    QByteArray buf_ba;
    QBuffer buf( &buf_ba );

    QFETCH( QString, host );
    QFETCH( uint, port );
    QFETCH( QString, path );
    QFETCH( bool, useIODevice );

    http = newHttp();
    QCOMPARE( http->currentId(), 0 );
    QCOMPARE( (int)http->state(), (int)QHttp::Unconnected );

    addRequest( QHttpRequestHeader(), http->setHost( host, port ) );
    if ( useIODevice ) {
	buf.open( QIODevice::WriteOnly );
	getId = http->get( path, &buf );
    } else {
	getId = http->get( path );
    }
    addRequest( QHttpRequestHeader(), getId );

    QTestEventLoop::instance().enterLoop( 30 );

    if ( QTestEventLoop::instance().timeout() )
	QFAIL( "Network operation timed out" );

    ResMapIt res = resultMap.find( getId );
    QVERIFY( res != resultMap.end() );
    if ( res.value().success!=1 && host=="www.ietf.org" ) {
	// The nightly tests fail from time to time. In order to make them more
	// stable, add some debug output that might help locate the problem (I
	// can't reproduce the problem in the non-nightly builds).
	qDebug( "Error %d: %s", http->error(), http->errorString().toLatin1().constData() );
    }
    QTEST( res.value().success, "success" );
    if ( res.value().success ) {
	QTEST( res.value().resp.statusCode(), "statusCode" );

	QFETCH( QByteArray, res );
	if ( res.count() > 0 ) {
	    if ( useIODevice ) {
		QCOMPARE(buf_ba, res);
		if ( bytesDoneRead != bytesDone_init )
		    QVERIFY( (int)buf_ba.size() == bytesDoneRead );
	    } else {
		QCOMPARE(readyRead_ba, res);
		if ( bytesDoneRead != bytesDone_init )
		    QVERIFY( (int)readyRead_ba.size() == bytesDoneRead );
	    }
	}
	QVERIFY( bytesTotalRead != bytesTotal_init );
	if ( bytesTotalRead > 0 )
	    QVERIFY( bytesDoneRead == bytesTotalRead );
    } else {
	QVERIFY( !res.value().resp.isValid() );
    }
}

void tst_QHttp::head_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<uint>("port");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("success");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<uint>("contentLength");

    QTest::newRow( "path_01" ) << QString("fluke.troll.no") << 80u
	<< QString("/qtest/rfc3252.txt") << 1 << 200 << 25962u;
    QTest::newRow( "path_02" ) << QString("www.ietf.org") << 80u
	<< QString("/rfc/rfc3252.txt") << 1 << 200 << 25962u;

    QTest::newRow( "uri_01" ) << QString("fluke.troll.no") << 80u
	<< QString("http://fluke/qtest/rfc3252.txt") << 1 << 200 << 25962u;
    QTest::newRow( "uri_02" ) << QString("www.ietf.org") << 80u
	<< QString("http://www.ietf.org/rfc/rfc3252.txt") << 1 << 200 << 25962u;

    QTest::newRow( "fail_01" ) << QString("foo") << 80u
	<< QString("/qtest/rfc3252.txt") << 0 << 0 << 0u;

    QTest::newRow( "failprot_01" ) << QString("fluke.troll.no") << 80u
	<< QString("/t") << 1 << 404 << 0u;
    QTest::newRow( "failprot_02" ) << QString("fluke.troll.no") << 80u
	<< QString("qtest/rfc3252.txt") << 1 << 400 << 0u;

    /* doc.trolltech.com no longer seams to be using chuncked encodig.
    QTest::newRow( "chunked_01" ) << QString("doc.trolltech.com") << 80u
	<< QString("/index.html") << 1 << 200 << 0u;
    */
    QTest::newRow( "chunked_02" ) << QString("fluke.troll.no") << 80u
	<< QString("/qtest/cgi-bin/rfc.cgi") << 1 << 200 << 0u;
}

void tst_QHttp::head()
{
    QFETCH( QString, host );
    QFETCH( uint, port );
    QFETCH( QString, path );
    
    http = newHttp();
    QCOMPARE( http->currentId(), 0 );
    QCOMPARE( (int)http->state(), (int)QHttp::Unconnected );
    
    addRequest( QHttpRequestHeader(), http->setHost( host, port ) );
    headId = http->head( path );
    addRequest( QHttpRequestHeader(), headId );
    
    QTestEventLoop::instance().enterLoop( 30 );
    if ( QTestEventLoop::instance().timeout() )
        QFAIL( "Network operation timed out" );
    
    ResMapIt res = resultMap.find( headId );
    QVERIFY( res != resultMap.end() );
    if ( res.value().success!=1 && host=="www.ietf.org" ) {
        // The nightly tests fail from time to time. In order to make them more
        // stable, add some debug output that might help locate the problem (I
        // can't reproduce the problem in the non-nightly builds).
        qDebug( "Error %d: %s", http->error(), http->errorString().toLatin1().constData() );
    }
    QTEST( res.value().success, "success" );
    if ( res.value().success ) {
        QTEST( res.value().resp.statusCode(), "statusCode" );
        QTEST( res.value().resp.contentLength(), "contentLength" );
        
        QCOMPARE( (uint)readyRead_ba.size(), 0u );
        QVERIFY( bytesTotalRead == bytesTotal_init );
        QVERIFY( bytesDoneRead == bytesDone_init );
    } else {
        QVERIFY( !res.value().resp.isValid() );
    }
}

void tst_QHttp::authorization_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("user");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<int>("result");

    QTest::newRow("correct password") << QString::fromLatin1("ares.troll.no")
                                << QString::fromLatin1("/~ahanssen/secret/")
                                << QString::fromLatin1("qtest")
                                << QString::fromLatin1("qtest")
                                << 200;

    QTest::newRow("no password") << QString::fromLatin1("ares.troll.no")
                                 << QString::fromLatin1("/~ahanssen/secret/")
                                 << QString::fromLatin1("")
                                 << QString::fromLatin1("")
                                 << 401;

    QTest::newRow("wrong password") << QString::fromLatin1("ares.troll.no")
                                 << QString::fromLatin1("/~ahanssen/secret/")
                                 << QString::fromLatin1("maliciu0s")
                                 << QString::fromLatin1("h4X0r")
                                 << 401;
}

void tst_QHttp::authorization()
{
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(QString, user);
    QFETCH(QString, pass);
    QFETCH(int, result);

    http = newHttp();

    QCOMPARE(http->currentId(), 0);
    QCOMPARE((int)http->state(), (int)QHttp::Unconnected);

    if (!user.isEmpty())
        addRequest(QHttpRequestHeader(), http->setUser(user, pass));
    addRequest(QHttpRequestHeader(), http->setHost(host));
    getId = http->get(path);
    addRequest(QHttpRequestHeader(), getId);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
	QFAIL("Network operation timed out");

    ResMapIt res = resultMap.find(getId);
    QVERIFY(res != resultMap.end());
    QCOMPARE(res.value().resp.statusCode(), result);
}

void tst_QHttp::proxy_data()
{
    QTest::addColumn<QString>("proxyhost");
    QTest::addColumn<int>("port");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("proxyuser");
    QTest::addColumn<QString>("proxypass");

    QTest::newRow("shusaku.troll.no") << QString::fromLatin1("shusaku.troll.no") << 3128
                                 << QString::fromLatin1("www.vg.no") << QString::fromLatin1("/")
                                 << QString::fromLatin1("") << QString::fromLatin1("");
    QTest::newRow("shusaku.troll.no pct") << QString::fromLatin1("shusaku.troll.no") << 3128
                                 << QString::fromLatin1("www.vg.no") << QString::fromLatin1("/%69ndex.html")
                                 << QString::fromLatin1("") << QString::fromLatin1("");
}

void tst_QHttp::proxy()
{
    QFETCH(QString, proxyhost);
    QFETCH(int, port);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(QString, proxyuser);
    QFETCH(QString, proxypass);

    http = newHttp();

    QCOMPARE(http->currentId(), 0);
    QCOMPARE((int)http->state(), (int)QHttp::Unconnected);

    addRequest(QHttpRequestHeader(), http->setProxy(proxyhost, port, proxyuser, proxypass));
    addRequest(QHttpRequestHeader(), http->setHost(host));
    getId = http->get(path);
    addRequest(QHttpRequestHeader(), getId);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
	QFAIL("Network operation timed out");

    ResMapIt res = resultMap.find(getId);
    QVERIFY(res != resultMap.end());
    QVERIFY(res.value().success);
    QCOMPARE(res.value().resp.statusCode(), 200);
}

void tst_QHttp::proxy2()
{
    readyRead_ba.clear();
    
    QHttp http;
    http.setProxy("shusaku.troll.no", 3128);
    http.setHost("intern.troll.no");
    http.get("/index.cgi");
    http.get("/index.cgi");

    connect(&http, SIGNAL(requestFinished(int, bool)),
            this, SLOT(proxy2_slot()));
    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(readyRead_ba.count("trolllogin"), 2);

    readyRead_ba.clear();
}

void tst_QHttp::proxy2_slot()
{
    QHttp *http = static_cast<QHttp *>(sender());
    readyRead_ba.append(http->readAll());
    if (!http->hasPendingRequests())
        QTestEventLoop::instance().exitLoop();
}

// test QHttp::currentId() and QHttp::currentRequest()
#define CURRENTREQUEST_TEST \
    { \
	ResMapIt res = resultMap.find( http->currentId() ); \
	QVERIFY( res != resultMap.end() ); \
	if ( http->currentId() == getId ) { \
	    QCOMPARE( http->currentRequest().method(), QString("GET") ); \
	} else if ( http->currentId() == headId ) { \
	    QCOMPARE( http->currentRequest().method(), QString("HEAD") ); \
	} else { \
	    QVERIFY( headerAreEqual( http->currentRequest(), res.value().req ) ); \
	} \
    }

void tst_QHttp::requestStarted( int id )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:requestStarted( %d )", http->currentId(), id );
#endif
    // make sure that the requestStarted and requestFinished are nested correctly
    QVERIFY( current_id == 0 );
    current_id = id;

    QVERIFY( !ids.isEmpty() );
    QVERIFY( ids.first() == id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }

    QVERIFY( http->currentId() == id );
    QVERIFY( cur_state == http->state() );




    CURRENTREQUEST_TEST;

    QVERIFY( http->error() == QHttp::NoError );
}

void tst_QHttp::requestFinished( int id, bool error )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:requestFinished( %d, %d ) -- errorString: '%s'",
	    http->currentId(), id, (int)error, http->errorString().latin1() );
#endif
    // make sure that the requestStarted and requestFinished are nested correctly
    QVERIFY( current_id == id );
    current_id = 0;

    QVERIFY( !ids.isEmpty() );
    QVERIFY( ids.first() == id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }

    if ( error ) {
	QVERIFY( http->error() != QHttp::NoError );
	ids.clear();
    } else {
	QVERIFY( http->error() == QHttp::NoError );
	ids.pop_front();
    }

    QVERIFY( http->currentId() == id );
    QVERIFY( cur_state == http->state() );
    CURRENTREQUEST_TEST;

    ResMapIt res = resultMap.find( http->currentId() );
    QVERIFY( res != resultMap.end() );
    QVERIFY( res.value().success == -1 );
    if ( error )
	res.value().success = 0;
    else
	res.value().success = 1;
}

void tst_QHttp::done( bool error )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:done( %d )", http->currentId(), (int)error );
#endif
    QVERIFY( http->currentId() == 0 );
    QVERIFY( current_id == 0 );
    QVERIFY( ids.isEmpty() );
    QVERIFY( cur_state == http->state() );
    QVERIFY( !http->hasPendingRequests() );

    QVERIFY( done_success == -1 );
    if ( error ) {
	QVERIFY( http->error() != QHttp::NoError );
	done_success = 0;
    } else {
	QVERIFY( http->error() == QHttp::NoError );
	done_success = 1;
    }
    QTestEventLoop::instance().exitLoop();
}

void tst_QHttp::stateChanged( int state )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:  stateChanged( %d )", http->currentId(), state );
#endif
    QCOMPARE( http->currentId(), current_id );
    if ( ids.count() > 0 )
	CURRENTREQUEST_TEST;

    QVERIFY( state != cur_state );
    QVERIFY( state == http->state() );
    if ( state != QHttp::Unconnected ) {
	// make sure that the states are always emitted in the right order (for
	// this, we assume an ordering on the enum values, which they have at
	// the moment)
        QVERIFY( cur_state < state );
    }
    cur_state = state;
}

void tst_QHttp::responseHeaderReceived( const QHttpResponseHeader &header )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:  responseHeaderReceived(\n---{\n%s}---)", http->currentId(), header.toString().latin1() );
#endif
    QCOMPARE( http->currentId(), current_id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }
    CURRENTREQUEST_TEST;

    resultMap[ http->currentId() ].resp = header;
}

void tst_QHttp::readyRead( const QHttpResponseHeader & )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:  readyRead()", http->currentId() );
#endif
    QCOMPARE( http->currentId(), current_id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }
    QVERIFY( cur_state == http->state() );
    CURRENTREQUEST_TEST;

    if ( QTest::currentTestFunction() != QLatin1String("bytesAvailable") ) {
	int oldSize = readyRead_ba.size();
	quint64 bytesAvail = http->bytesAvailable();
	QByteArray ba = http->readAll();
	QVERIFY( (quint64) ba.size() == bytesAvail );
	readyRead_ba.resize( oldSize + ba.size() );
	memcpy( readyRead_ba.data()+oldSize, ba.data(), ba.size() );

	if ( bytesTotalRead > 0 ) {
	    QVERIFY( (int)readyRead_ba.size() <= bytesTotalRead );
	}
	QVERIFY( (int)readyRead_ba.size() == bytesDoneRead );
    }
}

void tst_QHttp::dataSendProgress( int done, int total )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:  dataSendProgress( %d, %d )", http->currentId(), done, total );
#endif
    QCOMPARE( http->currentId(), current_id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }
    QVERIFY( cur_state == http->state() );
    CURRENTREQUEST_TEST;

    if ( bytesTotalSend == bytesTotal_init ) {
	bytesTotalSend = total;
    } else {
	QVERIFY( bytesTotalSend == total );
    }

    QVERIFY( bytesTotalSend != bytesTotal_init );
    QVERIFY( bytesDoneSend <= done );
    bytesDoneSend = done;
    if ( bytesTotalSend > 0 ) {
	QVERIFY( bytesDoneSend <= bytesTotalSend );
    }

    if ( QTest::currentTestFunction() == QLatin1String("abort") ) {
	// ### it would be nice if we could specify in our testdata when to do
	// the abort
	if ( done >= total/100000 ) {
	    if ( ids.count() != 1 ) {
		// do abort only once
		int tmpId = ids.first();
		ids.clear();
		ids << tmpId;
		http->abort();
	    }
	}
    }
}

void tst_QHttp::dataReadProgress( int done, int total )
{
#if defined( DUMP_SIGNALS )
    qDebug( "%d:  dataReadProgress( %d, %d )", http->currentId(), done, total );
#endif
    QCOMPARE( http->currentId(), current_id );
    if ( ids.count() > 1 ) {
	QVERIFY( http->hasPendingRequests() );
    } else {
	QVERIFY( !http->hasPendingRequests() );
    }
    QVERIFY( cur_state == http->state() );
    CURRENTREQUEST_TEST;

    if ( bytesTotalRead == bytesTotal_init )
	bytesTotalRead = total;
    else {
	QVERIFY( bytesTotalRead == total );
    }

    QVERIFY( bytesTotalRead != bytesTotal_init );
    QVERIFY( bytesDoneRead <= done );
    bytesDoneRead = done;
    if ( bytesTotalRead > 0 ) {
	QVERIFY( bytesDoneRead <= bytesTotalRead );
    }

    if ( QTest::currentTestFunction() == QLatin1String("abort") ) {
	// ### it would be nice if we could specify in our testdata when to do
	// the abort
	if ( done >= total/100000 ) {
	    if ( ids.count() != 1 ) {
		// do abort only once
		int tmpId = ids.first();
		ids.clear();
		ids << tmpId;
		http->abort();
	    }
	}
    }
}


QHttp *tst_QHttp::newHttp()
{
    QHttp *nHttp = new QHttp( 0 );
    connect( nHttp, SIGNAL(requestStarted(int)),
	    SLOT(requestStarted(int)) );
    connect( nHttp, SIGNAL(requestFinished(int,bool)),
	    SLOT(requestFinished(int,bool)) );
    connect( nHttp, SIGNAL(done(bool)),
	    SLOT(done(bool)) );
    connect( nHttp, SIGNAL(stateChanged(int)),
	    SLOT(stateChanged(int)) );
    connect( nHttp, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),
	    SLOT(responseHeaderReceived(const QHttpResponseHeader&)) );
    connect( nHttp, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    SLOT(readyRead(const QHttpResponseHeader&)) );
    connect( nHttp, SIGNAL(dataSendProgress(int,int)),
	    SLOT(dataSendProgress(int,int)) );
    connect( nHttp, SIGNAL(dataReadProgress(int,int)),
	    SLOT(dataReadProgress(int,int)) );

    return nHttp;
}

void tst_QHttp::addRequest( QHttpRequestHeader header, int id )
{
    ids << id;
    RequestResult res;
    res.req = header;
    res.success = -1;
    resultMap[ id ] = res;
}

bool tst_QHttp::headerAreEqual( const QHttpHeader &h1, const QHttpHeader &h2 )
{
    if ( !h1.isValid() )
	return !h2.isValid();
    if ( !h2.isValid() )
	return !h1.isValid();

    return h1.toString() == h2.toString();
}


void tst_QHttp::reconnect()
{
    reconnect_state_connect_count = 0;

    QHttp http;

    QObject::connect(&http, SIGNAL(stateChanged(int)), this, SLOT(reconnect_state(int)));
    http.setHost("trolltech.com", 80);
    http.get("/company/index.html");
    http.setHost("trolltech.com", 8080);
    http.get("/company/index.html");

    QTestEventLoop::instance().enterLoop(60);
    if (QTestEventLoop::instance().timeout())
	QFAIL("Network operation timed out");

    QCOMPARE(reconnect_state_connect_count, 1);

    QTestEventLoop::instance().enterLoop(60);
    if (QTestEventLoop::instance().timeout())
	QFAIL("Network operation timed out");

    QCOMPARE(reconnect_state_connect_count, 2);
}

void tst_QHttp::reconnect_state(int state)
{
    if (state == QHttp::Connecting) {
        ++reconnect_state_connect_count;
        QTestEventLoop::instance().exitLoop();
    }
}

void tst_QHttp::setSocket()
{
    QHttp *http = new QHttp;
    QPointer<QTcpSocket> replacementSocket = new QTcpSocket;
    http->setSocket(replacementSocket);
    QCoreApplication::processEvents();
    delete http;
    QVERIFY(replacementSocket);
    delete replacementSocket;
}

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server()
    {
        connect(this, SIGNAL(newConnection()),
                this, SLOT(serveConnection()));
    }

private slots:
    void serveConnection()
    {
        QTcpSocket *socket = nextPendingConnection();
        socket->write("HTTP/1.1 404 Not found\r\n"
                      "content-length: 4\r\n\r\nabcd");
        socket->disconnectFromHost();
    }
};

void tst_QHttp::unexpectedRemoteClose()
{
#ifdef TEST_QNETWORK_PROXY
    QFETCH_GLOBAL(int, proxyType);
    if (proxyType == QNetworkProxy::Socks5Proxy) {
        // This test doesn't make sense for SOCKS5
        return;
    }
#endif

    Server server;
    server.listen();
    QCoreApplication::instance()->processEvents();

    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    
    QHttp http;
    QObject::connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));
    QSignalSpy finishedSpy(&http, SIGNAL(requestFinished(int, bool)));
    QSignalSpy doneSpy(&http, SIGNAL(done(bool)));
    
    http.setHost("localhost", server.serverPort());
    http.get("/");
    http.get("/");
    http.get("/");

    loop.exec();

    QCOMPARE(finishedSpy.count(), 4);
    QVERIFY(!finishedSpy.at(1).at(1).toBool());
    QVERIFY(!finishedSpy.at(2).at(1).toBool());
    QVERIFY(!finishedSpy.at(3).at(1).toBool());
    QCOMPARE(doneSpy.count(), 1);
    QVERIFY(!doneSpy.at(0).at(0).toBool());
}

void tst_QHttp::pctEncodedPath()
{
    QHttpRequestHeader header;
    header.setRequest("GET", "/index.asp/a=%20&b=%20&c=%20");
    QCOMPARE(header.toString(), QString("GET /index.asp/a=%20&b=%20&c=%20 HTTP/1.1\r\n\r\n"));
}

void tst_QHttp::caseInsensitiveKeys()
{
    QHttpResponseHeader header("HTTP/1.1 200 OK\r\nContent-Length: 213\r\nX-Been-There: True\r\nLocation: http://www.TrollTech.com/\r\n\r\n");
    QVERIFY(header.hasKey("Content-Length"));
    QVERIFY(header.hasKey("X-Been-There"));
    QVERIFY(header.hasKey("Location"));
    QVERIFY(header.hasKey("content-length"));
    QVERIFY(header.hasKey("x-been-there"));
    QVERIFY(header.hasKey("location"));
    QCOMPARE(header.value("Content-Length"), QString("213"));
    QCOMPARE(header.value("X-Been-There"), QString("True"));
    QCOMPARE(header.value("Location"), QString("http://www.TrollTech.com/"));
    QCOMPARE(header.value("content-length"), QString("213"));
    QCOMPARE(header.value("x-been-there"), QString("True"));
    QCOMPARE(header.value("location"), QString("http://www.TrollTech.com/"));
    QCOMPARE(header.allValues("location"), QStringList("http://www.TrollTech.com/"));

    header.addValue("Content-Length", "213");
    header.addValue("Content-Length", "214");
    header.addValue("Content-Length", "215");
    qDebug() << header.toString();
}

QTEST_MAIN(tst_QHttp)
#include "tst_qhttp.moc"

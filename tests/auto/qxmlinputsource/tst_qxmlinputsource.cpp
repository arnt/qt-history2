/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDomDocument>
#include <QHttp>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QtDebug>
#include <QtTest>
#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

//TESTED_CLASS=QXmlInputSource
//TESTED_FILES=xml/qxml.h xml/qxml.cpp

class tst_QXmlInputSource : public QObject
{
    Q_OBJECT

private slots:
    void reset() const;
    void resetSimplified() const;
    void waitForReadyIODevice() const;
};

/*!
  \internal
  \since 4.4

  See task 166278.
 */
void tst_QXmlInputSource::reset() const
{
    const QString input(QString::fromLatin1("<element attribute1='value1' attribute2='value2'/>"));
   
    QXmlSimpleReader reader;
    QXmlDefaultHandler handler;
    reader.setContentHandler(&handler);

    QXmlInputSource source;
    source.setData(input);

    QCOMPARE(source.data(), input);

    source.reset();
    QCOMPARE(source.data(), input);

    source.reset();
    QVERIFY(reader.parse(source));
    source.reset();
    QCOMPARE(source.data(), input);
}

/*!
  \internal
  \since 4.4

  See task 166278.
 */
void tst_QXmlInputSource::resetSimplified() const
{
    const QString input(QString::fromLatin1("<element/>"));
   
    QXmlSimpleReader reader;

    QXmlInputSource source;
    source.setData(input);

    QVERIFY(reader.parse(source));
    source.reset();
    QCOMPARE(source.data(), input);
}

class ServerAndClient : public QObject
{
    Q_OBJECT

public:
    ServerAndClient(QEventLoop &ev) : success(false)
                                    , eventLoop(ev)
    {
        setObjectName("serverAndClient");
        tcpServer = new QTcpServer(this);
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
        tcpServer->listen(QHostAddress::LocalHost, 1088);
        httpClient = new QHttp(this);
        connect(httpClient, SIGNAL(requestFinished(int, bool)), SLOT(requestFinished(int, bool)));
    }

    bool success;
    QEventLoop &eventLoop;

public slots:
    void doIt()
    {
        QUrl url("http://127.0.0.1:1088");
        httpClient->setHost( url.host(), 1088);
        QHttpRequestHeader req_head("POST", url.path());
        req_head.setValue("host", url.host());
        req_head.setValue("user-agent", "xml-test");
        req_head.setValue("keep-alive", "false");

        QByteArray xmlrpc("<methodCall>\r\n\
                <methodName>SFD.GetVersion</methodName>\r\n\
                <params/>\r\n\
                </methodCall>");
        req_head.setContentLength(xmlrpc.size());
        req_head.setContentType("text/xml");

        httpClient->request(req_head, xmlrpc);
    }

    void requestFinished(int, bool isError)
    {
        QVERIFY(!isError);
    }

private slots:
    void newConnection()
    {
        QTcpSocket *const s = tcpServer->nextPendingConnection();

        if(s)
            connect(s, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }

    void readyRead()
    {
        QTcpSocket *const s = static_cast<QTcpSocket *>(sender());
        int bodyLength = -1;

        while(s->canReadLine())
        {
            const QString line(s->readLine());

            if(line.startsWith("content-length:"))
                bodyLength = line.mid(15).toInt();

            if(line == "\r\n")
            {
                if(bodyLength == -1)
                {
                    Q_ASSERT_X(false, Q_FUNC_INFO,
                               "No length was specified in the header.");
                    return;
                }

                QDomDocument domDoc;
                success = domDoc.setContent(s->read(bodyLength));
                eventLoop.exit();
            }
        }
    }

private:
    QTcpServer *tcpServer;
    QHttp* httpClient;
};

void tst_QXmlInputSource::waitForReadyIODevice() const
{
    QEventLoop el;
    ServerAndClient sv(el);
    QTimer::singleShot(1, &sv, SLOT(doIt()));

    el.exec();
    QVERIFY(sv.success);
}

QTEST_MAIN(tst_QXmlInputSource)
#include "tst_qxmlinputsource.moc"

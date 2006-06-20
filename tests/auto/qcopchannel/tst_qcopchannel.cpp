/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_WS_QWS

#include <QCopChannel>

class tst_QCopChannel : public QObject
{
    Q_OBJECT

public:
    tst_QCopChannel() {}
    virtual ~tst_QCopChannel() {}

private slots:
    void channel();
    void isRegistered();
    void sendreceive();
};

void tst_QCopChannel::channel()
{
    QCopChannel channel1("channel1");
    QCOMPARE(channel1.channel(), QString("channel1"));
}

void tst_QCopChannel::isRegistered()
{
    QVERIFY(!QCopChannel::isRegistered("foo"));

    const QString channelName("registered/channel");
    QCopChannel *channel = new QCopChannel(channelName);
    QVERIFY(QCopChannel::isRegistered(channelName));

    delete channel;
    QVERIFY(!QCopChannel::isRegistered(channelName));
}

void tst_QCopChannel::sendreceive()
{
    const QString channelName("tst_QcopChannel::send()");
    QCopChannel *channel = new QCopChannel(channelName);
    QSignalSpy spy(channel, SIGNAL(received(const QString&, const QByteArray&)));
    QCopChannel::send("foo", "msg");
    QApplication::processEvents();
    QCOMPARE(spy.count(), 0);
    QCopChannel::send(channelName, "msg", "data");
    QApplication::processEvents();
    QCOMPARE(spy.count(), 1);

    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("msg"));
    QCOMPARE(args.at(1).toByteArray(), QByteArray("data"));

    QCOMPARE(spy.count(), 0);

    delete channel;
    QCopChannel::send(channelName, "msg2");
    QApplication::processEvents();
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(tst_QCopChannel)

#include "tst_qcopchannel.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

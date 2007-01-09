/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qcoreapplication.h>

#include <qpointer.h>
#include <qtimer.h>
#include <qregexp.h>
#include <qmetaobject.h>
#include <qvariant.h>

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QProcess>

#include "qobject.h"

#include <math.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qobject.h corelib/kernel/qobject.cpp

class tst_QObject : public QObject
{
    Q_OBJECT

public:
    tst_QObject();
    virtual ~tst_QObject();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void disconnect();
    void connectByName();
    void connectSignalsToSignalsWithDefaultArguments();
    void qobject_castTemplate();
    void findChildren();
    void connectDisconnectNotify_data();
    void connectDisconnectNotify();
    void emitInDefinedOrder();
    void customTypes();
    void streamCustomTypes();
    void metamethod();
    void namespaces();
    void threadSignalEmissionCrash();
    void thread();
    void moveToThread();
    void sender();
    void declareInterface();
    void qpointerResetBeforeDestroyedSignal();
    void testUserData();
    void childDeletesItsSibling();
    void dynamicProperties();
    void floatProperty();
    void property();
    void recursiveSignalEmission();
    void blockingQueuedConnection();

protected:
};

tst_QObject::tst_QObject()
{

}

tst_QObject::~tst_QObject()
{

}

void tst_QObject::initTestCase()
{
}

void tst_QObject::cleanupTestCase()
{
}

void tst_QObject::init()
{
}

void tst_QObject::cleanup()
{
}

class SenderObject : public QObject
{
    Q_OBJECT

public:
    SenderObject() {}

    void emitSignal1() { emit signal1(); }
    void emitSignal2() { emit signal2(); }
    void emitSignal3() { emit signal3(); }
    void emitSignal4() { emit signal4(); }

signals:
    void signal1();
    void signal2();
    void signal3();
    void signal4();
    QT_MOC_COMPAT void signal5();

public slots:
    void aPublicSlot(){}

public:
    Q_INVOKABLE void invoke1(){}
    Q_SCRIPTABLE void sinvoke1(){}
protected:
    Q_INVOKABLE QT_MOC_COMPAT void invoke2(){}
    Q_INVOKABLE QT_MOC_COMPAT void invoke2(int){}
    Q_SCRIPTABLE QT_MOC_COMPAT void sinvoke2(){}
private:
    Q_INVOKABLE void invoke3(int hinz = 0, int kunz = 0){Q_UNUSED(hinz) Q_UNUSED(kunz)}
    Q_SCRIPTABLE void sinvoke3(){}

};

class ReceiverObject : public QObject
{
    Q_OBJECT

public:
    ReceiverObject() : sequence_slot1( 0 ),
		       sequence_slot2( 0 ),
		       sequence_slot3( 0 ),
		       sequence_slot4( 0 ) {}

    void reset() {
	sequence_slot4 = 0;
	sequence_slot3 = 0;
	sequence_slot2 = 0;
	sequence_slot1 = 0;
    }

    int sequence_slot1;
    int sequence_slot2;
    int sequence_slot3;
    int sequence_slot4;

    bool called(int slot) {
        switch (slot) {
        case 1: return sequence_slot1;
        case 2: return sequence_slot2;
        case 3: return sequence_slot3;
        case 4: return sequence_slot4;
        default: return false;
        }
    }

    static int sequence;

public slots:
    void slot1() { sequence_slot1 = ++sequence; }
    void slot2() { sequence_slot2 = ++sequence; }
    void slot3() { sequence_slot3 = ++sequence; }
    void slot4() { sequence_slot4 = ++sequence; }

};

int ReceiverObject::sequence = 0;

void tst_QObject::disconnect()
{
    SenderObject *s = new SenderObject;
    ReceiverObject *r1 = new ReceiverObject;
    ReceiverObject *r2 = new ReceiverObject;

    connect( s, SIGNAL( signal1() ), r1, SLOT( slot1() ) );

    connect( s, SIGNAL( signal2() ), r1, SLOT( slot2() ) );
    connect( s, SIGNAL( signal3() ), r1, SLOT( slot3() ) );
    connect( s, SIGNAL( signal4() ), r1, SLOT( slot4() ) );

    s->emitSignal1();
    s->emitSignal2();
    s->emitSignal3();
    s->emitSignal4();

    QCOMPARE( r1->called(1), TRUE );
    QCOMPARE( r1->called(2), TRUE );
    QCOMPARE( r1->called(3), TRUE );
    QCOMPARE( r1->called(4), TRUE );
    r1->reset();

    // usual disconnect with all parameters given
    bool ret = QObject::disconnect( s, SIGNAL( signal1() ), r1, SLOT( slot1() ) );

    s->emitSignal1();

    QCOMPARE( r1->called(1), FALSE );
    r1->reset();

#if QT_VERSION >= 0x030100
    QCOMPARE( ret, TRUE );
#endif
    ret = QObject::disconnect( s, SIGNAL( signal1() ), r1, SLOT( slot1() ) );
#if QT_VERSION >= 0x030100
    QCOMPARE( ret, FALSE  );
#endif

    // disconnect all signals from s from all slots from r1
    QObject::disconnect( s, 0, r1, 0 );

    s->emitSignal2();
    s->emitSignal3();
    s->emitSignal4();

    QCOMPARE( r1->called(2), FALSE );
    QCOMPARE( r1->called(3), FALSE );
    QCOMPARE( r1->called(4), FALSE );
    r1->reset();

    connect( s, SIGNAL( signal1() ), r1, SLOT( slot1() ) );
    connect( s, SIGNAL( signal1() ), r1, SLOT( slot2() ) );
    connect( s, SIGNAL( signal1() ), r1, SLOT( slot3() ) );
    connect( s, SIGNAL( signal2() ), r1, SLOT( slot4() ) );

    // disconnect s's signal1() from all slots of r1
    QObject::disconnect( s, SIGNAL( signal1() ), r1, 0 );

    s->emitSignal1();
    s->emitSignal2();

    QCOMPARE( r1->called(1), FALSE );
    QCOMPARE( r1->called(2), FALSE );
    QCOMPARE( r1->called(3), FALSE );
    QCOMPARE( r1->called(4), TRUE );
    r1->reset();
    // make sure all is disconnected again
    QObject::disconnect( s, 0, r1, 0 );

    connect( s, SIGNAL( signal1() ), r1, SLOT( slot1() ) );
    connect( s, SIGNAL( signal1() ), r2, SLOT( slot1() ) );
    connect( s, SIGNAL( signal2() ), r1, SLOT( slot2() ) );
    connect( s, SIGNAL( signal2() ), r2, SLOT( slot2() ) );
    connect( s, SIGNAL( signal3() ), r1, SLOT( slot3() ) );
    connect( s, SIGNAL( signal3() ), r2, SLOT( slot3() ) );

    // disconnect signal1() from all receivers
    QObject::disconnect( s, SIGNAL( signal1() ), 0, 0 );
    s->emitSignal1();
    s->emitSignal2();
    s->emitSignal3();

    QCOMPARE( r1->called(1), FALSE );
    QCOMPARE( r2->called(1), FALSE );
    QCOMPARE( r1->called(2), TRUE );
    QCOMPARE( r2->called(2), TRUE );
    QCOMPARE( r1->called(2), TRUE );
    QCOMPARE( r2->called(2), TRUE );

    r1->reset();
    r2->reset();

    // disconnect all signals of s from all receivers
    QObject::disconnect( s, 0, 0, 0 );

    QCOMPARE( r1->called(2), FALSE );
    QCOMPARE( r2->called(2), FALSE );
    QCOMPARE( r1->called(2), FALSE );
    QCOMPARE( r2->called(2), FALSE );

    delete r2;
    delete r1;
    delete s;
}

class AutoConnectSender : public QObject
{
    Q_OBJECT

public:
    AutoConnectSender(QObject *parent)
        : QObject(parent)
    {}

    void emitSignalNoParams() { emit signalNoParams(); }
    void emitSignalWithParams(int i) { emit signalWithParams(i); }
    void emitSignalWithParams(int i, QString string) { emit signalWithParams(i, string); }
    void emitSignalManyParams(int i1, int i2, int i3, QString string, bool onoff) { emit signalManyParams(i1, i2, i3, string, onoff); }
    void emitSignalManyParams2(int i1, int i2, int i3, QString string, bool onoff) { emit signalManyParams2(i1, i2, i3, string, onoff); }
    void emitSignalLoopBack() { emit signalLoopBack(); }

signals:
    void signalNoParams();
    void signalWithParams(int i);
    void signalWithParams(int i, QString string);
    void signalManyParams(int i1, int i2, int i3, QString string, bool onoff);
    void signalManyParams(int i1, int i2, int i3, QString string, bool onoff, bool);
    void signalManyParams2(int i1, int i2, int i3, QString string, bool onoff);
    void signalLoopBack();
};

class AutoConnectReceiver : public QObject
{
    Q_OBJECT

public:
    AutoConnectReceiver()
    {
        reset();

        connect(this, SIGNAL(on_Sender_signalLoopBack()), this, SLOT(slotLoopBack()));
    }

    void reset() {
        called_slot8 = 0;
        called_slot7 = 0;
        called_slot6 = 0;
        called_slot5 = 0;
        called_slot4 = 0;
        called_slot3 = 0;
        called_slot2 = 0;
        called_slot1 = 0;
    }

    int called_slot1;
    int called_slot2;
    int called_slot3;
    int called_slot4;
    int called_slot5;
    int called_slot6;
    int called_slot7;
    int called_slot8;

    bool called(int slot) {
        switch (slot) {
        case 1: return called_slot1;
        case 2: return called_slot2;
        case 3: return called_slot3;
        case 4: return called_slot4;
        case 5: return called_slot5;
        case 6: return called_slot6;
        case 7: return called_slot7;
        case 8: return called_slot8;
        default: return false;
        }
    }

public slots:
    void on_Sender_signalNoParams() { ++called_slot1; }
    void on_Sender_signalWithParams(int i = 0) { ++called_slot2; }
    void on_Sender_signalWithParams(int i, QString string) { ++called_slot3; }
    void on_Sender_signalManyParams() { ++called_slot4; }
    void on_Sender_signalManyParams(int i1, int i2, int i3, QString string, bool onoff) { ++called_slot5; }
    void on_Sender_signalManyParams(int i1, int i2, int i3, QString string, bool onoff, bool dummy) { ++called_slot6; }
    void on_Sender_signalManyParams2(int i1, int i2, int i3, QString string, bool onoff) { ++called_slot7; }
    void slotLoopBack() { ++called_slot8; }

protected slots:
    void o() { Q_ASSERT(0); }
    void on() { Q_ASSERT(0); }

signals:
    void on_Sender_signalLoopBack();
};

void tst_QObject::connectByName()
{
    AutoConnectReceiver receiver;
    AutoConnectSender sender(&receiver);
    sender.setObjectName("Sender");

    QMetaObject::connectSlotsByName(&receiver);

    sender.emitSignalNoParams();
    QCOMPARE(receiver.called(1), true);
    QCOMPARE(receiver.called(2), false);
    QCOMPARE(receiver.called(3), false);
    QCOMPARE(receiver.called(4), false);
    QCOMPARE(receiver.called(5), false);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), false);
    QCOMPARE(receiver.called(8), false);
    receiver.reset();

    sender.emitSignalWithParams(0);
    QCOMPARE(receiver.called(1), false);
    QCOMPARE(receiver.called(2), true);
    QCOMPARE(receiver.called(3), false);
    QCOMPARE(receiver.called(4), false);
    QCOMPARE(receiver.called(5), false);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), false);
    QCOMPARE(receiver.called(8), false);
    receiver.reset();

    sender.emitSignalWithParams(0, "string");
    QCOMPARE(receiver.called(1), false);
    QCOMPARE(receiver.called(2), false);
    QCOMPARE(receiver.called(3), true);
    QCOMPARE(receiver.called(4), false);
    QCOMPARE(receiver.called(5), false);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), false);
    QCOMPARE(receiver.called(8), false);
    receiver.reset();

    sender.emitSignalManyParams(1, 2, 3, "string", true);
    QCOMPARE(receiver.called(1), false);
    QCOMPARE(receiver.called(2), false);
    QCOMPARE(receiver.called(3), false);
    QCOMPARE(receiver.called(4), true);
    QCOMPARE(receiver.called(5), true);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), false);
    QCOMPARE(receiver.called(8), false);
    receiver.reset();

    sender.emitSignalManyParams2(1, 2, 3, "string", true);
    QCOMPARE(receiver.called(1), false);
    QCOMPARE(receiver.called(2), false);
    QCOMPARE(receiver.called(3), false);
    QCOMPARE(receiver.called(4), false);
    QCOMPARE(receiver.called(5), false);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), true);
    QCOMPARE(receiver.called(8), false);
    receiver.reset();

    sender.emitSignalLoopBack();
    QCOMPARE(receiver.called(1), false);
    QCOMPARE(receiver.called(2), false);
    QCOMPARE(receiver.called(3), false);
    QCOMPARE(receiver.called(4), false);
    QCOMPARE(receiver.called(5), false);
    QCOMPARE(receiver.called(6), false);
    QCOMPARE(receiver.called(7), false);
    QCOMPARE(receiver.called(8), true);
    receiver.reset();
}

void tst_QObject::qobject_castTemplate()
{
    QObject *o = 0;
    QVERIFY( !::qobject_cast<QObject*>(o) );

    o = new SenderObject;
    QVERIFY( ::qobject_cast<SenderObject*>(o) );
    QVERIFY( ::qobject_cast<QObject*>(o) );
    QVERIFY( !::qobject_cast<ReceiverObject*>(o) );
    delete o;
}

void tst_QObject::findChildren()
{
    QObject o;
    QObject o1(&o);
    QObject o2(&o);
    QObject o11(&o1);
    QObject o12(&o1);
    QObject o111(&o11);
    QObject unnamed(&o);
    QTimer t1(&o);
    QTimer t121(&o12);
    QTimer emptyname(&o);

    o.setObjectName("o");
    o1.setObjectName("o1");
    o2.setObjectName("o2");
    o11.setObjectName("o11");
    o12.setObjectName("o12");
    o111.setObjectName("o111");
    t1.setObjectName("t1");
    t121.setObjectName("t121");
    emptyname.setObjectName("");

    QObject *op = 0;

    op = qFindChild<QObject*>(&o, "o1");
    QCOMPARE(op, &o1);
    op = qFindChild<QObject*>(&o, "o2");
    QCOMPARE(op, &o2);
    op = qFindChild<QObject*>(&o, "o11");
    QCOMPARE(op, &o11);
    op = qFindChild<QObject*>(&o, "o12");
    QCOMPARE(op, &o12);
    op = qFindChild<QObject*>(&o, "o111");
    QCOMPARE(op, &o111);
    op = qFindChild<QObject*>(&o, "t1");
    QCOMPARE(op, static_cast<QObject *>(&t1));
    op = qFindChild<QObject*>(&o, "t121");
    QCOMPARE(op, static_cast<QObject *>(&t121));
    op = qFindChild<QTimer*>(&o, "t1");
    QCOMPARE(op, static_cast<QObject *>(&t1));
    op = qFindChild<QTimer*>(&o, "t121");
    QCOMPARE(op, static_cast<QObject *>(&t121));
    op = qFindChild<QTimer*>(&o, "o12");
    QCOMPARE(op, static_cast<QObject *>(0));
    op = qFindChild<QObject*>(&o, "o");
    QCOMPARE(op, static_cast<QObject *>(0));
    op = qFindChild<QObject*>(&o, "harry");
    QCOMPARE(op, static_cast<QObject *>(0));
    op = qFindChild<QObject*>(&o, "o1");
    QCOMPARE(op, &o1);

    QList<QObject*> l;
    QList<QTimer*> tl;

    l = qFindChildren<QObject*>(&o, "o1");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o1);
    l = qFindChildren<QObject*>(&o, "o2");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o2);
    l = qFindChildren<QObject*>(&o, "o11");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o11);
    l = qFindChildren<QObject*>(&o, "o12");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o12);
    l = qFindChildren<QObject*>(&o, "o111");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o111);
    l = qFindChildren<QObject*>(&o, "t1");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), static_cast<QObject *>(&t1));
    l = qFindChildren<QObject*>(&o, "t121");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), static_cast<QObject *>(&t121));
    tl = qFindChildren<QTimer*>(&o, "t1");
    QCOMPARE(tl.size(), 1);
    QCOMPARE(tl.at(0), &t1);
    tl = qFindChildren<QTimer*>(&o, "t121");
    QCOMPARE(tl.size(), 1);
    QCOMPARE(tl.at(0), &t121);
    l = qFindChildren<QObject*>(&o, "o");
    QCOMPARE(l.size(), 0);
    l = qFindChildren<QObject*>(&o, "harry");
    QCOMPARE(l.size(), 0);
    tl = qFindChildren<QTimer*>(&o, "o12");
    QCOMPARE(tl.size(), 0);
    l = qFindChildren<QObject*>(&o, "o1");
    QCOMPARE(l.size(), 1);
    QCOMPARE(l.at(0), &o1);

    l = qFindChildren<QObject*>(&o, QRegExp("o.*"));
    QCOMPARE(l.size(), 5);
    QVERIFY(l.contains(&o1));
    QVERIFY(l.contains(&o2));
    QVERIFY(l.contains(&o11));
    QVERIFY(l.contains(&o12));
    QVERIFY(l.contains(&o111));
    l = qFindChildren<QObject*>(&o, QRegExp("t.*"));
    QCOMPARE(l.size(), 2);
    QVERIFY(l.contains(&t1));
    QVERIFY(l.contains(&t121));
    tl = qFindChildren<QTimer*>(&o, QRegExp(".*"));
    QCOMPARE(tl.size(), 3);
    QVERIFY(tl.contains(&t1));
    QVERIFY(tl.contains(&t121));
    tl = qFindChildren<QTimer*>(&o, QRegExp("o.*"));
    QCOMPARE(tl.size(), 0);
    l = qFindChildren<QObject*>(&o, QRegExp("harry"));
    QCOMPARE(l.size(), 0);

    // empty and null string check
    op = qFindChild<QObject*>(&o);
    QCOMPARE(op, &o1);
    op = qFindChild<QObject*>(&o, "");
    QCOMPARE(op, &unnamed);
    op = qFindChild<QObject*>(&o, "unnamed");
    QCOMPARE(op, static_cast<QObject *>(0));

    l = qFindChildren<QObject*>(&o);
    QCOMPARE(l.size(), 9);
    l = qFindChildren<QObject*>(&o, "");
    QCOMPARE(l.size(), 2);
    l = qFindChildren<QObject*>(&o, "unnamed");
    QCOMPARE(l.size(), 0);

#ifndef QT_NO_MEMBER_TEMPLATES
    tl = o.findChildren<QTimer *>("t1");
    QCOMPARE(tl.size(), 1);
    QCOMPARE(tl.at(0), &t1);
#endif
}


class NotifyObject : public SenderObject, public ReceiverObject
{
public:
    NotifyObject() : SenderObject(), ReceiverObject()
    {}

    QString org_signal;
    QString nw_signal;

protected:
    void connectNotify( const char *signal )
    {
    	org_signal = signal;
    	nw_signal = QMetaObject::normalizedSignature(signal);
    };
    void disconnectNotify( const char *signal )
    {
    	org_signal = signal;
    	nw_signal = QMetaObject::normalizedSignature(signal);
    };
};

void tst_QObject::connectDisconnectNotify_data()
{
    QTest::addColumn<QString>("a_signal");
    QTest::addColumn<QString>("a_slot");

    QTest::newRow("combo1") << SIGNAL( signal1() )        << SLOT( slot1() );
    QTest::newRow("combo2") << SIGNAL( signal2(void) )    << SLOT( slot2(  ) );
    QTest::newRow("combo3") << SIGNAL( signal3(  ) )      << SLOT( slot3(void) );
    QTest::newRow("combo4") << SIGNAL(  signal4( void )  )<< SLOT(  slot4( void )  );
}

void tst_QObject::connectDisconnectNotify()
{
    NotifyObject *s = new NotifyObject;
    NotifyObject *r = new NotifyObject;

    QFETCH(QString, a_signal);
    QFETCH(QString, a_slot);

    // Test connectNotify
    connect( (SenderObject*)s, a_signal.toLatin1(), (ReceiverObject*)r, a_slot.toLatin1() );
    QCOMPARE( s->org_signal, s->nw_signal );

    // Test disconnectNotify
    QObject::disconnect( (SenderObject*)s, a_signal.toLatin1(), (ReceiverObject*)r, a_slot.toLatin1() );
    QCOMPARE( s->org_signal, s->nw_signal );

#if QT_VERSION > 0x040101
    // Reconnect
    connect( (SenderObject*)s, a_signal.toLatin1(), (ReceiverObject*)r, a_slot.toLatin1() );
    // Test disconnectNotify for a complete disconnect
    ((SenderObject*)s)->disconnect((ReceiverObject*)r);
#endif

    delete s;
    delete r;
}

class SequenceObject : public ReceiverObject
{
    Q_OBJECT

public:
    QObject *next;
    SequenceObject() : next(0) { }

public slots:
    void slot1_disconnectThis()
    {
        slot1();
        disconnect(sender(), SIGNAL(signal1()), this, SLOT(slot1_disconnectThis()));
    }

    void slot2_reconnectThis()
    {
        slot2();

        const QObject *s = sender();
        disconnect(s, SIGNAL(signal1()), this, SLOT(slot2_reconnectThis()));
        connect(s, SIGNAL(signal1()), this, SLOT(slot2_reconnectThis()));
    }

    void slot1_disconnectNext()
    {
        slot1();
        disconnect(sender(), SIGNAL(signal1()), next, SLOT(slot1()));
    }

    void slot2_reconnectNext()
    {
        slot2();

        // modify the connection list in 'this'
        disconnect(sender(), SIGNAL(signal1()), next, SLOT(slot2()));
        connect(sender(), SIGNAL(signal1()), next, SLOT(slot2()));

        // modify the sender list in 'this'
        connect(next, SIGNAL(destroyed()), this, SLOT(deleteLater()));
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
        disconnect(next, SIGNAL(destroyed()), this, SLOT(deleteLater()));
        disconnect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
    }

    void slot1_deleteNext()
    {
        slot1();
        delete next;
    }

    void slot2_deleteSender()
    {
        slot2();
        delete sender();
    }
};

void tst_QObject::emitInDefinedOrder()
{
    SenderObject sender;
    ReceiverObject receiver1, receiver2, receiver3, receiver4;

    connect(&sender, SIGNAL(signal1()), &receiver1, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver2, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver3, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver4, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver1, SLOT(slot2()));
    connect(&sender, SIGNAL(signal1()), &receiver2, SLOT(slot2()));
    connect(&sender, SIGNAL(signal1()), &receiver3, SLOT(slot2()));
    connect(&sender, SIGNAL(signal1()), &receiver4, SLOT(slot2()));

    int sequence;
    ReceiverObject::sequence = sequence = 0;
    sender.emitSignal1();
    QCOMPARE(receiver1.sequence_slot1, ++sequence);
    QCOMPARE(receiver2.sequence_slot1, ++sequence);
    QCOMPARE(receiver3.sequence_slot1, ++sequence);
    QCOMPARE(receiver4.sequence_slot1, ++sequence);
    QCOMPARE(receiver1.sequence_slot2, ++sequence);
    QCOMPARE(receiver2.sequence_slot2, ++sequence);
    QCOMPARE(receiver3.sequence_slot2, ++sequence);
    QCOMPARE(receiver4.sequence_slot2, ++sequence);

    QObject::disconnect(&sender, SIGNAL(signal1()), &receiver2, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver2, SLOT(slot1()));

    ReceiverObject::sequence = sequence =  0;
    sender.emitSignal1();
    QCOMPARE(receiver1.sequence_slot1, ++sequence);
    QCOMPARE(receiver3.sequence_slot1, ++sequence);
    QCOMPARE(receiver4.sequence_slot1, ++sequence);
    QCOMPARE(receiver1.sequence_slot2, ++sequence);
    QCOMPARE(receiver2.sequence_slot2, ++sequence);
    QCOMPARE(receiver3.sequence_slot2, ++sequence);
    QCOMPARE(receiver4.sequence_slot2, ++sequence);
    QCOMPARE(receiver2.sequence_slot1, ++sequence);

    QObject::disconnect(&sender, SIGNAL(signal1()), &receiver1, SLOT(slot1()));
    connect(&sender, SIGNAL(signal1()), &receiver1, SLOT(slot1()));

    ReceiverObject::sequence = sequence =  0;
    sender.emitSignal1();
    QCOMPARE(receiver3.sequence_slot1, ++sequence);
    QCOMPARE(receiver4.sequence_slot1, ++sequence);
    QCOMPARE(receiver1.sequence_slot2, ++sequence);
    QCOMPARE(receiver2.sequence_slot2, ++sequence);
    QCOMPARE(receiver3.sequence_slot2, ++sequence);
    QCOMPARE(receiver4.sequence_slot2, ++sequence);
    QCOMPARE(receiver2.sequence_slot1, ++sequence);
    QCOMPARE(receiver1.sequence_slot1, ++sequence);

    // ensure emission order even if the connections change during emission
    SenderObject *sender2 = new SenderObject;
    SequenceObject seq1, seq2, *seq3 = new SequenceObject, seq4;
    seq1.next = &seq2;
    seq2.next = seq3;
    seq3->next = &seq4;

    // try 1
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot1_disconnectThis()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot1_disconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot2_reconnectThis()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot2_reconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot2()));

    SequenceObject::sequence = sequence = 0;
    sender2->emitSignal1();
    QCOMPARE(seq1.called(1), TRUE);
    QCOMPARE(seq2.called(1), TRUE);
    QCOMPARE(seq3->called(1), FALSE);
    QCOMPARE(seq4.called(1), TRUE);
    QCOMPARE(seq1.called(2), TRUE);
    QCOMPARE(seq2.called(2), TRUE);
    QCOMPARE(seq3->called(2), FALSE);
    QCOMPARE(seq4.called(2), TRUE);
    QCOMPARE(seq1.sequence_slot1, ++sequence);
    QCOMPARE(seq2.sequence_slot1, ++sequence);
    QCOMPARE(seq4.sequence_slot1, ++sequence);
    QCOMPARE(seq1.sequence_slot2, ++sequence);
    QCOMPARE(seq2.sequence_slot2, ++sequence);
    QCOMPARE(seq4.sequence_slot2, ++sequence);

    QObject::disconnect(sender2, 0, &seq1, 0);
    QObject::disconnect(sender2, 0, &seq2, 0);
    QObject::disconnect(sender2, 0, seq3, 0);
    QObject::disconnect(sender2, 0, &seq4, 0);
    seq1.reset();
    seq2.reset();
    seq3->reset();
    seq4.reset();

    // try 2
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot2_reconnectThis()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot2_reconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot1_disconnectThis()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot1_disconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot1()));

    SequenceObject::sequence = sequence = 0;
    sender2->emitSignal1();
    QCOMPARE(seq1.called(2), TRUE);
    QCOMPARE(seq2.called(2), TRUE);
    QCOMPARE(seq3->called(2), FALSE);
    QCOMPARE(seq4.called(2), TRUE);
    QCOMPARE(seq1.called(1), TRUE);
    QCOMPARE(seq2.called(1), TRUE);
    QCOMPARE(seq3->called(1), FALSE);
    QCOMPARE(seq4.called(1), TRUE);
    QCOMPARE(seq1.sequence_slot2, ++sequence);
    QCOMPARE(seq2.sequence_slot2, ++sequence);
    QCOMPARE(seq4.sequence_slot2, ++sequence);
    QCOMPARE(seq1.sequence_slot1, ++sequence);
    QCOMPARE(seq2.sequence_slot1, ++sequence);
    QCOMPARE(seq4.sequence_slot1, ++sequence);

    QObject::disconnect(sender2, 0, &seq1, 0);
    QObject::disconnect(sender2, 0, &seq2, 0);
    QObject::disconnect(sender2, 0, seq3, 0);
    QObject::disconnect(sender2, 0, &seq4, 0);
    seq1.reset();
    seq2.reset();
    seq3->reset();
    seq4.reset();

    // try 3
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot1_disconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot2_reconnectNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot2()));

    SequenceObject::sequence = sequence = 0;
    sender2->emitSignal1();
    QCOMPARE(seq1.called(1), TRUE);
    QCOMPARE(seq2.called(1), TRUE);
    QCOMPARE(seq3->called(1), FALSE);
    QCOMPARE(seq4.called(1), TRUE);
    QCOMPARE(seq1.called(2), TRUE);
    QCOMPARE(seq2.called(2), TRUE);
    QCOMPARE(seq3->called(2), FALSE);
    QCOMPARE(seq4.called(2), TRUE);
    QCOMPARE(seq1.sequence_slot1, ++sequence);
    QCOMPARE(seq2.sequence_slot1, ++sequence);
    QCOMPARE(seq4.sequence_slot1, ++sequence);
    QCOMPARE(seq1.sequence_slot2, ++sequence);
    QCOMPARE(seq2.sequence_slot2, ++sequence);
    QCOMPARE(seq4.sequence_slot2, ++sequence);

    // ensure emission order even if objects are destroyed during emission
    QObject::disconnect(sender2, 0, &seq1, 0);
    QObject::disconnect(sender2, 0, &seq2, 0);
    QObject::disconnect(sender2, 0, seq3, 0);
    QObject::disconnect(sender2, 0, &seq4, 0);
    seq1.reset();
    seq2.reset();
    seq3->reset();
    seq4.reset();

    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot1_deleteNext()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot1()));
    connect(sender2, SIGNAL(signal1()), &seq1, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq2, SLOT(slot2_deleteSender()));
    connect(sender2, SIGNAL(signal1()), seq3, SLOT(slot2()));
    connect(sender2, SIGNAL(signal1()), &seq4, SLOT(slot2()));

    QPointer<SenderObject> psender = sender2;
    QPointer<SequenceObject> pseq3 = seq3;

    SequenceObject::sequence = sequence = 0;
    sender2->emitSignal1();
    QCOMPARE(static_cast<QObject *>(psender), static_cast<QObject *>(0));
    QCOMPARE(static_cast<QObject *>(pseq3), static_cast<QObject *>(0));
    QCOMPARE(seq1.called(1), TRUE);
    QCOMPARE(seq2.called(1), TRUE);
    QCOMPARE(seq4.called(1), TRUE);
    QCOMPARE(seq1.called(2), TRUE);
    QCOMPARE(seq2.called(2), TRUE);
    QCOMPARE(seq4.called(2), FALSE);
    QCOMPARE(seq1.sequence_slot1, ++sequence);
    QCOMPARE(seq2.sequence_slot1, ++sequence);
    QCOMPARE(seq4.sequence_slot1, ++sequence);
    QCOMPARE(seq1.sequence_slot2, ++sequence);
    QCOMPARE(seq2.sequence_slot2, ++sequence);
}

static int instanceCount = 0;

struct CustomType
{
    CustomType(int l1 = 0, int l2 = 0, int l3 = 0): i1(l1), i2(l2), i3(l3)
    { ++instanceCount; }
    CustomType(const CustomType &other): i1(other.i1), i2(other.i2), i3(other.i3)
    { ++instanceCount; }
    ~CustomType() { --instanceCount; }

    int i1, i2, i3;
    int value() { return i1 + i2 + i3; }
};

Q_DECLARE_METATYPE(CustomType*)

class QCustomTypeChecker: public QObject
{
    Q_OBJECT

public:
    QCustomTypeChecker(QObject *parent = 0): QObject(parent) {}
    void doEmit(CustomType ct)
    { emit signal1(ct); }

public slots:
    void slot1(CustomType ct);

signals:
    void signal1(CustomType ct);

public:
    CustomType received;
};

void QCustomTypeChecker::slot1(CustomType ct)
{ received = ct; }


void tst_QObject::customTypes()
{
    CustomType t0;
    CustomType t1(1, 2, 3);
    CustomType t2(2, 3, 4);

    {
        QCustomTypeChecker checker;
        QCOMPARE(instanceCount, 4);

        connect(&checker, SIGNAL(signal1(CustomType)), &checker, SLOT(slot1(CustomType)),
                Qt::DirectConnection);
        QCOMPARE(checker.received.value(), 0);
        checker.doEmit(t1);
        QCOMPARE(checker.received.value(), t1.value());
        checker.received = t0;

        int idx = qRegisterMetaType<CustomType>("CustomType");
        QCOMPARE(QMetaType::type("CustomType"), idx);

        checker.disconnect();
        connect(&checker, SIGNAL(signal1(CustomType)), &checker, SLOT(slot1(CustomType)),
                Qt::QueuedConnection);
        QCOMPARE(instanceCount, 4);
        checker.doEmit(t2);
        QCOMPARE(instanceCount, 5);
        QCOMPARE(checker.received.value(), t0.value());

        QCoreApplication::processEvents();
        QCOMPARE(checker.received.value(), t2.value());
        QCOMPARE(instanceCount, 4);

        QVERIFY(QMetaType::isRegistered(idx));
        QCOMPARE(qRegisterMetaType<CustomType>("CustomType"), idx);
        QCOMPARE(QMetaType::type("CustomType"), idx);
        QVERIFY(QMetaType::isRegistered(idx));
    }
    QCOMPARE(instanceCount, 3);
}

QDataStream &operator<<(QDataStream &stream, const CustomType &ct)
{
    stream << ct.i1 << ct.i2 << ct.i3;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, CustomType &ct)
{
    stream >> ct.i1;
    stream >> ct.i2;
    stream >> ct.i3;
    return stream;
}

void tst_QObject::streamCustomTypes()
{
    QByteArray ba;

    int idx = qRegisterMetaType<CustomType>("CustomType");
    qRegisterMetaTypeStreamOperators<CustomType>("CustomType");

    {
        CustomType t1(1, 2, 3);
        QCOMPARE(instanceCount, 1);
        QDataStream stream(&ba, QIODevice::WriteOnly);
        QMetaType::save(stream, idx, &t1);
    }

    QCOMPARE(instanceCount, 0);

    {
        CustomType t2;
        QCOMPARE(instanceCount, 1);
        QDataStream stream(&ba, QIODevice::ReadOnly);
        QMetaType::load(stream, idx, &t2);
        QCOMPARE(instanceCount, 1);
        QCOMPARE(t2.i1, 1);
        QCOMPARE(t2.i2, 2);
        QCOMPARE(t2.i3, 3);
    }
    QCOMPARE(instanceCount, 0);
}

class PropertyObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Alpha)

    Q_PROPERTY(Alpha alpha READ alpha WRITE setAlpha)
    Q_PROPERTY(int number READ number WRITE setNumber)
    Q_PROPERTY(QString string READ string WRITE setString)
    Q_PROPERTY(QVariant variant READ variant WRITE setVariant)
    Q_PROPERTY(CustomType* custom READ custom WRITE setCustom)
    Q_PROPERTY(float myFloat READ myFloat WRITE setMyFloat)

public:
    enum Alpha {
        Alpha0,
        Alpha1,
        Alpha2
    };

    PropertyObject()
        : m_alpha(Alpha0), m_number(0), m_custom(0), m_float(42)
    {}

    Alpha alpha() const { return m_alpha; }
    void setAlpha(Alpha alpha) { m_alpha = alpha; }

    int number() const { return m_number; }
    void setNumber(int number) { m_number = number; }

    QString string() const { return m_string; }
    void setString(const QString &string) { m_string = string; }

    QVariant variant() const { return m_variant; }
    void setVariant(const QVariant &variant) { m_variant = variant; }

    CustomType *custom() const { return m_custom; }
    void setCustom(CustomType *custom) { m_custom = custom; }

    void setMyFloat(float value) { m_float = value; }
    inline float myFloat() const { return m_float; }

private:
    Alpha m_alpha;
    int m_number;
    QString m_string;
    QVariant m_variant;
    CustomType *m_custom;
    float m_float;
};

void tst_QObject::threadSignalEmissionCrash()
{
    for (int i = 0; i < 1000; ++i) {
        QTcpSocket socket;
        socket.connectToHost("localhost", 80);
    }
}

class TestThread : public QThread
{
    Q_OBJECT
public:
    inline void run()
    {
        *object = new QObject;
        *child = new QObject(*object);
        mutex.lock();
        cond.wakeOne();
        cond.wait(&mutex);
        mutex.unlock();
    }

    QObject **object, **child;
    QMutex mutex;
    QWaitCondition cond;
};

void tst_QObject::thread()
{
    QThread *currentThread = QThread::currentThread();
    // the current thread is the same as the QApplication
    // thread... see tst_QApplication::thread()

    {
        QObject object;
        // thread affinity for objects with no parent should be the
        // current thread
        QVERIFY(object.thread() != 0);
        QCOMPARE(object.thread(), currentThread);
        // children inherit their parent's thread
        QObject child(&object);
        QCOMPARE(child.thread(), object.thread());
    }

    QObject *object = 0;
    QObject *child = 0;

    {
        TestThread thr;
        QVERIFY(thr.thread() != 0);
        QCOMPARE(thr.thread(), currentThread);

        thr.object = &object;
        thr.child = &child;

        thr.mutex.lock();
        thr.start();
        thr.cond.wait(&thr.mutex);

        // thread affinity for an object with no parent should be the
        // thread in which the object was created
        QCOMPARE(object->thread(), (QThread *)&thr);
        // children inherit their parent's thread
        QCOMPARE(child->thread(), object->thread());

        thr.cond.wakeOne();
        thr.mutex.unlock();
        thr.wait();

        // even though the thread is no longer running, the affinity
        // should not change
        QCOMPARE(object->thread(), (QThread *)&thr);
        QCOMPARE(child->thread(), object->thread());
    }

    // the thread has been destroyed, thread affinity should
    // automatically reset to no thread
    QCOMPARE(object->thread(), (QThread *)0);
    QCOMPARE(child->thread(), object->thread());

    delete object;
}

class MoveToThreadObject : public QObject
{
    Q_OBJECT
public:
    QThread *timerEventThread;
    QThread *customEventThread;
    QThread *slotThread;

    MoveToThreadObject(QObject *parent = 0)
        : QObject(parent), timerEventThread(0), customEventThread(0), slotThread(0)
    { }

    void customEvent(QEvent *)
    {
        Q_ASSERT(customEventThread == 0);
        customEventThread = QThread::currentThread();
        emit theSignal();
    }

    void timerEvent(QTimerEvent *)
    {
        Q_ASSERT(timerEventThread == 0);
        timerEventThread = QThread::currentThread();
        emit theSignal();
    }

public slots:
    void theSlot()
    {
        Q_ASSERT(slotThread == 0);
        slotThread = QThread::currentThread();
        emit theSignal();
    }

signals:
    void theSignal();
};

class MoveToThreadThread : public QThread
{
public:
    ~MoveToThreadThread()
    {
        if (isRunning()) {
            terminate();
            wait();
        }
    }
    void start()
    {
        QEventLoop eventLoop;
        connect(this, SIGNAL(started()), &eventLoop, SLOT(quit()), Qt::QueuedConnection);
        QThread::start();
        // wait for thread to start
        (void) eventLoop.exec();
    }
    void run()
    { (void) exec(); }
};

void tst_QObject::moveToThread()
{
    QThread *currentThread = QThread::currentThread();

    {
        QObject *object = new QObject;
        QObject *child = new QObject(object);
        QCOMPARE(object->thread(), currentThread);
        QCOMPARE(child->thread(), currentThread);
        object->moveToThread(0);
        QCOMPARE(object->thread(), (QThread *)0);
        QCOMPARE(child->thread(), (QThread *)0);
        object->moveToThread(currentThread);
        QCOMPARE(object->thread(), currentThread);
        QCOMPARE(child->thread(), currentThread);
        object->moveToThread(0);
        QCOMPARE(object->thread(), (QThread *)0);
        QCOMPARE(child->thread(), (QThread *)0);
        // can delete an object with no thread anywhere
        delete object;
    }

    {
        MoveToThreadThread thread;
        thread.start();

        QObject *object = new QObject;
        QObject *child = new QObject(object);
        QPointer<QObject> opointer = object;
        QPointer<QObject> cpointer = object;

        QCOMPARE(object->thread(), currentThread);
        QCOMPARE(child->thread(), currentThread);
        object->moveToThread(&thread);
        QCOMPARE(object->thread(), (QThread *)&thread);
        QCOMPARE(child->thread(), (QThread *)&thread);

        connect(object, SIGNAL(destroyed()), &thread, SLOT(quit()), Qt::DirectConnection);
        object->deleteLater();
        thread.wait();

        QVERIFY(opointer == 0);
        QVERIFY(cpointer == 0);
    }

    {
        // make sure posted events are moved with the object
        MoveToThreadThread thread;
        thread.start();

        MoveToThreadObject *object = new MoveToThreadObject;
        MoveToThreadObject *child = new MoveToThreadObject(object);

        connect(object, SIGNAL(theSignal()), &thread, SLOT(quit()), Qt::DirectConnection);
        QCoreApplication::postEvent(child, new QEvent(QEvent::User));
        QCoreApplication::postEvent(object, new QEvent(QEvent::User));

        QCOMPARE(object->thread(), currentThread);
        QCOMPARE(child->thread(), currentThread);
        object->moveToThread(&thread);
        QCOMPARE(object->thread(), (QThread *)&thread);
        QCOMPARE(child->thread(), (QThread *)&thread);

        thread.wait();

        QCOMPARE(object->customEventThread, (QThread *)&thread);
        QCOMPARE(child->customEventThread, (QThread *)&thread);

        thread.start();
        connect(object, SIGNAL(destroyed()), &thread, SLOT(quit()), Qt::DirectConnection);
        object->deleteLater();
        thread.wait();
    }

    {
        // make sure timers are moved with the object
        MoveToThreadThread thread;
        thread.start();

        MoveToThreadObject *object = new MoveToThreadObject;
        MoveToThreadObject *child = new MoveToThreadObject(object);

        connect(object, SIGNAL(theSignal()), &thread, SLOT(quit()), Qt::DirectConnection);
        child->startTimer(90);
        object->startTimer(100);

        QCOMPARE(object->thread(), currentThread);
        QCOMPARE(child->thread(), currentThread);
        object->moveToThread(&thread);
        QCOMPARE(object->thread(), (QThread *)&thread);
        QCOMPARE(child->thread(), (QThread *)&thread);

        thread.wait();

        QCOMPARE(object->timerEventThread, (QThread *)&thread);
        QCOMPARE(child->timerEventThread, (QThread *)&thread);

        thread.start();
        connect(object, SIGNAL(destroyed()), &thread, SLOT(quit()), Qt::DirectConnection);
        object->deleteLater();
        thread.wait();
    }

    {
        // make sure socket notifiers are moved with the object
        MoveToThreadThread thread;
        thread.start();

        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));
        QTcpSocket *socket = new QTcpSocket;
        MoveToThreadObject *child = new MoveToThreadObject(socket);
        connect(socket, SIGNAL(disconnected()), child, SLOT(theSlot()), Qt::DirectConnection);
        connect(child, SIGNAL(theSignal()), &thread, SLOT(quit()), Qt::DirectConnection);

        socket->connectToHost(server.serverAddress(), server.serverPort());

        QVERIFY(server.waitForNewConnection(1000));
        QTcpSocket *serverSocket = server.nextPendingConnection();
        QVERIFY(serverSocket);

        socket->waitForConnected();

        QCOMPARE(socket->thread(), currentThread);
        socket->moveToThread(&thread);
        QCOMPARE(socket->thread(), (QThread *)&thread);

        serverSocket->close();

        QVERIFY(thread.wait(10000));

        QCOMPARE(child->slotThread, (QThread *)&thread);

        thread.start();
        connect(socket, SIGNAL(destroyed()), &thread, SLOT(quit()), Qt::DirectConnection);
        socket->deleteLater();
        thread.wait();
    }
}


void tst_QObject::property()
{
    PropertyObject object;
    const QMetaObject *mo = object.metaObject();
    QMetaProperty property;
    QVERIFY(mo);

    QVERIFY(mo->indexOfProperty("alpha") != -1);
    property = mo->property(mo->indexOfProperty("alpha"));
    QVERIFY(property.isEnumType());
    QCOMPARE(property.typeName(), "Alpha");
    QCOMPARE(property.type(), QVariant::Int);

    QVariant var = object.property("alpha");
    QVERIFY(!var.isNull());
    QCOMPARE(var.toInt(), int(PropertyObject::Alpha0));
    object.setAlpha(PropertyObject::Alpha1);
    QCOMPARE(object.property("alpha").toInt(), int(PropertyObject::Alpha1));
    QVERIFY(object.setProperty("alpha", PropertyObject::Alpha2));
    QCOMPARE(object.property("alpha").toInt(), int(PropertyObject::Alpha2));
    QVERIFY(object.setProperty("alpha", "Alpha1"));
    QCOMPARE(object.property("alpha").toInt(), int(PropertyObject::Alpha1));
    QVERIFY(!object.setProperty("alpha", QVariant()));

    QVERIFY(mo->indexOfProperty("number") != -1);
    QCOMPARE(object.property("number").toInt(), 0);
    object.setNumber(24);
    QCOMPARE(object.property("number"), QVariant(24));
    QVERIFY(object.setProperty("number", 12));
    QCOMPARE(object.property("number"), QVariant(12));
    QVERIFY(object.setProperty("number", "42"));
    QCOMPARE(object.property("number"), QVariant(42));

    QVERIFY(mo->indexOfProperty("string") != -1);
    QCOMPARE(object.property("string").toString(), QString());
    object.setString("String1");
    QCOMPARE(object.property("string"), QVariant("String1"));
    QVERIFY(object.setProperty("string", "String2"));
    QCOMPARE(object.property("string"), QVariant("String2"));
    QVERIFY(!object.setProperty("string", QVariant()));

    const int idx = mo->indexOfProperty("variant");
    QVERIFY(idx != -1);
    QVERIFY(mo->property(idx).type() == QVariant::LastType);
    QCOMPARE(object.property("variant"), QVariant());
    QVariant variant1(42);
    QVariant variant2("string");
    object.setVariant(variant1);
    QCOMPARE(object.property("variant"), variant1);
    QVERIFY(object.setProperty("variant", variant2));
    QCOMPARE(object.variant(), QVariant(variant2));
    QCOMPARE(object.property("variant"), variant2);
    QVERIFY(object.setProperty("variant", QVariant()));
    QCOMPARE(object.property("variant"), QVariant());

    QVERIFY(mo->indexOfProperty("custom") != -1);
    property = mo->property(mo->indexOfProperty("custom"));
    QVERIFY(property.isValid());
    QVERIFY(property.isWritable());
    QVERIFY(!property.isEnumType());
    QCOMPARE(property.typeName(), "CustomType*");
    QCOMPARE(property.type(), QVariant::UserType);

    CustomType *customPointer = 0;
    QVariant customVariant = object.property("custom");
    customPointer = qVariantValue<CustomType *>(customVariant);
    QCOMPARE(customPointer, object.custom());

    CustomType custom;
    customPointer = &custom;
    qVariantSetValue(customVariant, customPointer);

    property = mo->property(mo->indexOfProperty("custom"));
    QVERIFY(property.isWritable());
    QCOMPARE(property.typeName(), "CustomType*");
    QCOMPARE(property.type(), QVariant::UserType);

    QVERIFY(object.setProperty("custom", customVariant));
    QCOMPARE(object.custom(), customPointer);

    customVariant = object.property("custom");
    customPointer = qVariantValue<CustomType *>(customVariant);
    QCOMPARE(object.custom(), customPointer);
}

void tst_QObject::metamethod()
{
    SenderObject obj;
    const QMetaObject *mobj = obj.metaObject();
    QMetaMethod m;

    m = mobj->method(mobj->indexOfMethod("invoke1()"));
    QVERIFY(QByteArray(m.signature()) == "invoke1()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Public);
    QVERIFY(!(m.attributes() & QMetaMethod::Scriptable));
    QVERIFY(!(m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("sinvoke1()"));
    QVERIFY(QByteArray(m.signature()) == "sinvoke1()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Public);
    QVERIFY((m.attributes() & QMetaMethod::Scriptable));
    QVERIFY(!(m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("invoke2()"));
    QVERIFY(QByteArray(m.signature()) == "invoke2()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Protected);
    QVERIFY(!(m.attributes() & QMetaMethod::Scriptable));
    QVERIFY((m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("sinvoke2()"));
    QVERIFY(QByteArray(m.signature()) == "sinvoke2()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Protected);
    QVERIFY((m.attributes() & QMetaMethod::Scriptable));
    QVERIFY((m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("invoke3()"));
    QVERIFY(QByteArray(m.signature()) == "invoke3()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Private);
    QVERIFY(!(m.attributes() & QMetaMethod::Scriptable));
    QVERIFY(!(m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("sinvoke3()"));
    QVERIFY(QByteArray(m.signature()) == "sinvoke3()");
    QVERIFY(m.methodType() == QMetaMethod::Method);
    QVERIFY(m.access() == QMetaMethod::Private);
    QVERIFY((m.attributes() & QMetaMethod::Scriptable));
    QVERIFY(!(m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("signal5()"));
    QVERIFY(QByteArray(m.signature()) == "signal5()");
    QVERIFY(m.methodType() == QMetaMethod::Signal);
    QVERIFY(m.access() == QMetaMethod::Protected);
    QVERIFY(!(m.attributes() & QMetaMethod::Scriptable));
    QVERIFY((m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("aPublicSlot()"));
    QVERIFY(QByteArray(m.signature()) == "aPublicSlot()");
    QVERIFY(m.methodType() == QMetaMethod::Slot);
    QVERIFY(m.access() == QMetaMethod::Public);
    QVERIFY(!(m.attributes() & QMetaMethod::Scriptable));
    QVERIFY(!(m.attributes() & QMetaMethod::Compatibility));

    m = mobj->method(mobj->indexOfMethod("invoke1()"));
    QCOMPARE(m.parameterNames().count(), 0);
    QCOMPARE(m.parameterTypes().count(), 0);

    m = mobj->method(mobj->indexOfMethod("invoke2(int)"));
    QCOMPARE(m.parameterNames().count(), 1);
    QCOMPARE(m.parameterTypes().count(), 1);
    QCOMPARE(m.parameterTypes().at(0), QByteArray("int"));
    QVERIFY(m.parameterNames().at(0).isEmpty());

    m = mobj->method(mobj->indexOfMethod("invoke3(int,int)"));
    QCOMPARE(m.parameterNames().count(), 2);
    QCOMPARE(m.parameterTypes().count(), 2);
    QCOMPARE(m.parameterTypes().at(0), QByteArray("int"));
    QCOMPARE(m.parameterNames().at(0), QByteArray("hinz"));
    QCOMPARE(m.parameterTypes().at(1), QByteArray("int"));
    QCOMPARE(m.parameterNames().at(1), QByteArray("kunz"));

}

namespace QObjectTest
{
    class TestObject: public QObject
    {
    Q_OBJECT
    public:
        TestObject(): QObject(), i(0) {}
        void doEmit() { emit aSignal(); }
        int i;
    public slots:
        void aSlot() { ++i; }
    signals:
        void aSignal();
    };
}

void tst_QObject::namespaces()
{
    QObjectTest::TestObject obj;

    QVERIFY(connect(&obj, SIGNAL(aSignal()), &obj, SLOT(aSlot())));
    obj.doEmit();
    QCOMPARE(obj.i, 1);
}

class SuperObject : public QObject
{
    Q_OBJECT
public:
    QObject *theSender;
    SuperObject()
    {
        theSender = 0;
    }

    friend class tst_QObject;

    using QObject::sender;

public slots:
    void rememberSender()
    {
        theSender = sender();
    }

    void deleteAndRememberSender()
    {
        delete theSender;
        theSender = sender();
    }
signals:
    void theSignal();

};

void tst_QObject::sender()
{
    {
        SuperObject sender;
        SuperObject receiver;
        connect(&sender, SIGNAL(theSignal()),
                &receiver, SLOT(rememberSender()));
        QCOMPARE(receiver.sender(), (QObject *)0);
        emit sender.theSignal();
        QCOMPARE(receiver.theSender, (QObject *)&sender);
        QCOMPARE(receiver.sender(), (QObject *)0);
    }

    {
        SuperObject *sender = new SuperObject;
        SuperObject receiver;
        connect(sender, SIGNAL(theSignal()),
                &receiver, SLOT(deleteAndRememberSender()));
        QCOMPARE(receiver.sender(), (QObject *)0);
        receiver.theSender = sender;
        emit sender->theSignal();
        QCOMPARE(receiver.theSender, (QObject *)0);
        QCOMPARE(receiver.sender(), (QObject *)0);
    }

}

namespace Foo
{
    struct Bar
    {
        virtual ~Bar() {}
        virtual int rtti() const = 0;
    };

    struct Bleh
    {
        virtual ~Bleh() {}
        virtual int rtti() const = 0;
    };
}
Q_DECLARE_INTERFACE(Foo::Bar, "com.qtest.foobar")

#define Bleh_iid "com.qtest.bleh"
Q_DECLARE_INTERFACE(Foo::Bleh, Bleh_iid)

class FooObject: public QObject, public Foo::Bar
{
    Q_OBJECT
    Q_INTERFACES(Foo::Bar)
public:
    int rtti() const { return 42; }
};

class BlehObject : public QObject, public Foo::Bleh
{
    Q_OBJECT
    Q_INTERFACES(Foo::Bleh)
public:
    int rtti() const { return 43; }
};

void tst_QObject::declareInterface()
{
    FooObject obj;

    Foo::Bar *bar = qobject_cast<Foo::Bar *>(&obj);
    QVERIFY(bar);
    QCOMPARE(bar->rtti(), 42);
    QCOMPARE(static_cast<Foo::Bar *>(&obj), bar);

    BlehObject bleh;

    bar = qobject_cast<Foo::Bar *>(&bleh);
    QVERIFY(!bar);
    Foo::Bleh *b = qobject_cast<Foo::Bleh *>(&bleh);
    QCOMPARE(b->rtti(), 43);
    QCOMPARE(static_cast<Foo::Bleh *>(&bleh), b);

}

class CustomData : public QObjectUserData
{
public:
    int id;
};

void tst_QObject::testUserData()
{
    const int USER_DATA_COUNT = 100;
    int user_data_ids[USER_DATA_COUNT];

    // Register a few
    for (int i=0; i<USER_DATA_COUNT; ++i) {
        user_data_ids[i] = QObject::registerUserData();
    }

    // Randomize the table a bit
    for (int i=0; i<100; ++i) {
        int p1 = rand() % USER_DATA_COUNT;
        int p2 = rand() % USER_DATA_COUNT;

        int tmp = user_data_ids[p1];
        user_data_ids[p1] = user_data_ids[p2];
        user_data_ids[p2] = tmp;
    }

    // insert the user data into an object
    QObject my_test_object;
    for (int i=0; i<USER_DATA_COUNT; ++i) {
        CustomData *data = new CustomData;
        data->id = user_data_ids[i];
        my_test_object.setUserData(data->id, data);
    }

    // verify that all ids and positions are matching
    for (int i=0; i<USER_DATA_COUNT; ++i) {
        int id = user_data_ids[i];
        CustomData *data = static_cast<CustomData *>(my_test_object.userData(id));
        QVERIFY(data != 0);
        QVERIFY(data->id == id);
    }
}

class DestroyedListener : public QObject
{
    Q_OBJECT
public:
    inline DestroyedListener() : pointerWasZero(false) {}

    QPointer<QObject> pointer;
    bool pointerWasZero;

private slots:
    inline void otherObjectDestroyed()
    { pointerWasZero = pointer.isNull(); }
};

void tst_QObject::qpointerResetBeforeDestroyedSignal()
{
    QObject *obj = new QObject;
    DestroyedListener listener;
    listener.pointer = obj;
    listener.pointerWasZero = false;
    connect(obj, SIGNAL(destroyed()), &listener, SLOT(otherObjectDestroyed()));
    delete obj;
    QVERIFY(listener.pointerWasZero);
    QVERIFY(listener.pointer.isNull());
}

class DefaultArguments : public QObject
{
    Q_OBJECT

public slots:

    void theSlot(const QString &s) { result = s; }

signals:
    void theOriginalSignal();
    void theSecondSignal(const QString &s = QString("secondDefault"));

public:

    void emitTheOriginalSignal() { emit theOriginalSignal(); }
    void emitTheSecondSignal() { emit theSecondSignal(); }
    QString result;
};

void tst_QObject::connectSignalsToSignalsWithDefaultArguments()
{
    DefaultArguments o;
    connect(&o, SIGNAL(theOriginalSignal()), &o, SIGNAL(theSecondSignal()));
    connect(&o, SIGNAL(theSecondSignal(QString)), &o, SLOT(theSlot(QString)));
    QVERIFY( o.result.isEmpty() );
    o.emitTheSecondSignal();
    QCOMPARE(o.result, QString("secondDefault"));
    o.result = "Not called";
    o.emitTheOriginalSignal();
    QCOMPARE(o.result, QString("secondDefault"));

}

class SiblingDeleter : public QObject
{
public:
    inline SiblingDeleter(QObject *sibling, QObject *parent)
        : QObject(parent), sibling(sibling) {}
    inline virtual ~SiblingDeleter() { delete sibling; }

private:
    QPointer<QObject> sibling;
};


void tst_QObject::childDeletesItsSibling()
{
    QObject *commonParent = new QObject(0);
    QPointer<QObject> child = new QObject(0);
    QPointer<QObject> siblingDeleter = new SiblingDeleter(child, commonParent);
    child->setParent(commonParent);
    delete commonParent; // don't crash
    QVERIFY(!child);
    QVERIFY(!siblingDeleter);
}

void tst_QObject::floatProperty()
{
    PropertyObject obj;
    const int idx = obj.metaObject()->indexOfProperty("myFloat");
    QVERIFY(idx > 0);
    QMetaProperty prop = obj.metaObject()->property(idx);
    QVERIFY(prop.isValid());
    QVERIFY(prop.type() == uint(QMetaType::type("float")));
    QVERIFY(!prop.write(&obj, QVariant("Hello")));
    QVERIFY(prop.write(&obj, qVariantFromValue(128.0f)));
    QVariant v = prop.read(&obj);
    QVERIFY(int(v.userType()) == QMetaType::Float);
    QVERIFY(qVariantValue<float>(v) == 128.0f);
}

class DynamicPropertyObject : public PropertyObject
{
public:
    inline DynamicPropertyObject() {}

    inline virtual bool event(QEvent *e) {
        if (e->type() == QEvent::DynamicPropertyChange) {
            changedDynamicProperties.append(static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName());
        }
        return QObject::event(e);
    }

    QList<QByteArray> changedDynamicProperties;
};

void tst_QObject::dynamicProperties()
{
    DynamicPropertyObject obj;

    QVERIFY(obj.dynamicPropertyNames().isEmpty());

    QVERIFY(obj.setProperty("number", 42));
    QVERIFY(obj.changedDynamicProperties.isEmpty());
    QCOMPARE(obj.property("number").toInt(), 42);

    QVERIFY(!obj.setProperty("number", "invalid string"));
    QVERIFY(obj.changedDynamicProperties.isEmpty());

    QVERIFY(!obj.setProperty("myuserproperty", "Hello"));
    QCOMPARE(obj.changedDynamicProperties.count(), 1);
    QCOMPARE(obj.changedDynamicProperties.first(), QByteArray("myuserproperty"));
    obj.changedDynamicProperties.clear();

    QCOMPARE(obj.property("myuserproperty").toString(), QString("Hello"));

    QCOMPARE(obj.dynamicPropertyNames().count(), 1);
    QCOMPARE(obj.dynamicPropertyNames().first(), QByteArray("myuserproperty"));

    QVERIFY(!obj.setProperty("myuserproperty", QVariant()));

    QCOMPARE(obj.changedDynamicProperties.count(), 1);
    QCOMPARE(obj.changedDynamicProperties.first(), QByteArray("myuserproperty"));
    obj.changedDynamicProperties.clear();

    QVERIFY(obj.property("myuserproperty").isNull());

    QVERIFY(obj.dynamicPropertyNames().isEmpty());
}

void tst_QObject::recursiveSignalEmission()
{
    QProcess proc;
    proc.start("./signalbug");
    QVERIFY(proc.waitForFinished());
    QVERIFY(proc.exitStatus() == QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 0);
}

void tst_QObject::blockingQueuedConnection()
{
    SenderObject sender;

    MoveToThreadThread thread;
    ReceiverObject receiver;
    receiver.moveToThread(&thread);
    thread.start();

    receiver.connect(&sender, SIGNAL(signal1()), SLOT(slot1()), Qt::BlockingQueuedConnection);
    sender.emitSignal1();
    QVERIFY(receiver.called(1));

    receiver.reset();
    QVERIFY(QMetaObject::invokeMethod(&receiver, "slot1", Qt::BlockingQueuedConnection));
    QVERIFY(receiver.called(1));

    thread.quit();
    QVERIFY(thread.wait());
}

QTEST_MAIN(tst_QObject)
#include "tst_qobject.moc"

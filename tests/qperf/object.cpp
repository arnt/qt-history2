#include "qperf.h"
#include <qobject.h>


class MyObject : public QObject
{
    Q_OBJECT
public:
    MyObject( QObject *parent=0, const char *name=0 );
   ~MyObject();
signals:
    void signalA();
    void signalB( int );
public slots:
    void slotA()	{ asum++; }
    void slotB( int n )	{ bsum += n; }
public:
    void emitA()        { emit signalA(); }
    void emitB( int n ) { emit signalB(n); }
private:
    int  asum, bsum;
};

MyObject::MyObject( QObject *parent, const char *name )
    : QObject(parent,name), asum(0), bsum(0)
{
}

MyObject::~MyObject()
{
}

#include "object.moc"


static void object_init()
{
}


static int object_create()
{
    int i;
    for ( i=0; i<10000; i++ ) {
	MyObject my;
    }
    return i;
}

static int object_connect()
{
    MyObject a;
    MyObject b;
    int i;
    for ( i=0; i<10000; i++ ) {
	QObject::connect( &a, SIGNAL(signalA()), &b, SLOT(slotA()) );
	QObject::disconnect( &a, SIGNAL(signalA()), &b, SLOT(slotA()) );
    }
    return i;
}

static int object_emit_void()
{
    MyObject a;
    int i;
    for ( i=0; i<10000; i++ ) {
	a.emitA();
    }
    return i;
}

static int object_emit_one()
{
    MyObject a;
    MyObject b;
    QObject::connect( &a, SIGNAL(signalA()), &b, SLOT(slotA()) );
    int i;
    for ( i=0; i<10000; i++ ) {
	a.emitA();
    }
    return i;
}

static int object_emit_two()
{
    MyObject a;
    MyObject b;
    MyObject c;
    QObject::connect( &a, SIGNAL(signalA()), &b, SLOT(slotA()) );
    QObject::connect( &a, SIGNAL(signalA()), &c, SLOT(slotA()) );
    int i;
    for ( i=0; i<10000; i++ ) {
	a.emitA();
    }
    return i;
}


QPERF_BEGIN(object,"QObject operations, signals/slots, properties etc.")
    QPERF(object_create,"Create and destroy an object")
    QPERF(object_connect,"Connect and disconnect a signal")
    QPERF(object_emit_void,"Emit signal when nothing is connected")
    QPERF(object_emit_one,"Emit signal with one connection")
    QPERF(object_emit_two,"Emit signal with two connections")
QPERF_END(object)

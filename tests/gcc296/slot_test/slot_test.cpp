#include <qobject.h>

const int SIZE = 3;


// -------------------------

class V {
public:
    V();
    virtual ~V();
    virtual void vfunc();
protected:
    int v[SIZE];
};

V::V()
{
    for ( int i = 0; i < SIZE; ++i )
	v[i] = i;
}

V::~V()
{
}

void V::vfunc()
{
    qDebug( "V::vfunc()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tv[%d] = %d", j, v[j] );
}


// -------------------------

class X : public QObject, virtual public V {
    Q_OBJECT
public:
    X();
    virtual ~X();
    virtual void debug();
public slots:
    virtual void xslot1();
    virtual void xslot2( int );
    virtual void xslot3();
    void xslot4();
    void xslot5( int );
    void xslot6();
signals:
    void xsignal1();
    void xsignal2( int );
protected:
    int x[SIZE];
};

X::X() : QObject( 0, 0 ), V()
{
    for ( int i = 0; i < SIZE; ++i )
	x[i] = i;
    connect( this, SIGNAL(xsignal1()), SLOT(xslot1()) );
    connect( this, SIGNAL(xsignal2(int)), SLOT(xslot2(int)) );
    connect( this, SIGNAL(xsignal1()), SLOT(xslot3()) );
    connect( this, SIGNAL(xsignal1()), SLOT(xslot4()) );
    connect( this, SIGNAL(xsignal2(int)), SLOT(xslot5(int)) );
    connect( this, SIGNAL(xsignal1()), SLOT(xslot6()) );
}

X::~X()
{
}

void X::debug()
{
    qDebug( "X::debug()" );
    emit xsignal1();
    emit xsignal2( 12345 );
}


void X::xslot1()
{
    qDebug( "X::xslot1()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tx[%d] = %d", j, x[j] );
}

void X::xslot2( int )
{
    qDebug( "X::xslot2()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tx[%d] = %d", j, x[j] );
}

void X::xslot3()
{
    qDebug( "X::xslot3()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tv[%d] = %d", j, v[j] );
    vfunc();
}

void X::xslot4()
{
    qDebug( "X::xslot4()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tx[%d] = %d", j, x[j] );
}

void X::xslot5( int )
{
    qDebug( "X::xslot5()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tx[%d] = %d", j, x[j] );
}

void X::xslot6()
{
    qDebug( "X::xslot6()" );
    for ( int j = 0; j < SIZE; ++j )
	qDebug( "\tv[%d] = %d", j, v[j] );
    vfunc();
}


// -------------------------

#include "slot_test.moc"


// -------------------------

int main()
{
    X x;
    x.debug();
    return 0;
}

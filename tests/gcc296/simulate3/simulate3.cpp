#include <stdio.h>

typedef unsigned int uint;

const int SIZE = 3;


// -------------------------

class Q {
public:
    Q();
    virtual ~Q();
protected:
    int q[SIZE];
};

typedef void (Q::*QMember)();

Q::Q()
{
    for ( int i = 0; i < SIZE; ++i )
	q[i] = i;
}

Q::~Q()
{
}


// -------------------------

class V {
public:
    V();
    virtual ~V();
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


// -------------------------

class X : public Q, virtual public V {
public:
    X();
    virtual ~X();
    virtual void debug();
protected:
    int x[SIZE];
};

typedef void (X::*XMember)();

X::X() : Q(), V()
{
    for ( int i = 0; i < SIZE; ++i )
	x[i] = i;
}

X::~X()
{
}

void X::debug()
{
    for ( int i = 0; i < SIZE; ++i )
	printf( "\t\tq[%d] = %d\tv[%d] = %d\tx[%d] = %d\n",
	    i, q[i], i, v[i], i, x[i] );
}


// -------------------------

int main() {
    X x;

    X* xptr = &x;
    Q* qptr = xptr;
    
    XMember xmfp = &X::debug;
    QMember qmfp1 = *((QMember*)&xmfp);
    QMember qmfp2 = *reinterpret_cast<QMember*>(&xmfp);
    QMember qmfp3 = (QMember)xmfp;
    QMember qmfp4 = reinterpret_cast<QMember>(xmfp);

    uint i;

    printf( "X*       -> XMember\n" );
    printf( "%08x ->", (uint)xptr );
    for ( i = 0; i < sizeof(xmfp)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&xmfp)[i] );
    printf( "\n" );
    (xptr->*xmfp)();
    printf( "\n" );

    printf( "Q*       -> reinterpret_cast<QMember>(XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp4)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp4)[i] );
    printf( "\n" );
    (qptr->*qmfp4)();
    printf( "\n" );

    printf( "Q*       -> (QMember)XMember\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp3)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp3)[i] );
    printf( "\n" );
    (qptr->*qmfp3)();
    printf( "\n" );

    printf( "Q*       -> *reinterpret_cast<QMember*>(&XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp2)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp2)[i] );
    printf( "\n" );
    (qptr->*qmfp2)();
    printf( "\n" );

    printf( "Q*       -> *((QMember*)&XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp1)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp1)[i] );
    printf( "\n" );
    (qptr->*qmfp1)();
    printf( "\n" );

    return 0;
}

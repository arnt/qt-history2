#include <stdio.h>

typedef unsigned int uint;

const int SIZE = 3;


// -------------------------

class Q {
public:
    Q();
protected:
    int q[SIZE];
};

Q::Q()
{
    for ( int i = 0; i < SIZE; ++i )
	q[i] = i;
}

typedef void (Q::*QMember)();


// -------------------------

class V {
public:
    V();
protected:
    int v[SIZE];
};

V::V()
{
    for ( int i = 0; i < SIZE; ++i )
	v[i] = i;
}


// -------------------------

class X : public Q, virtual public V {
public:
    X();
    void debug();
protected:
    int x[SIZE];
};

typedef void (X::*XMember)();


X::X() : Q(), V()
{
    for ( int i = 0; i < SIZE; ++i )
	x[i] = i;
}

void X::debug()
{
    printf( "\t'this' address\n" );
    printf( "\t\t%p\n", this );

    printf( "\tmember variables\n" );
    for ( int i = 0; i < SIZE; ++i )
	printf( "\t\tq[%d] = %d\n", i, q[i] );
    for ( int i = 0; i < SIZE; ++i )
	printf( "\t\tv[%d] = %d\n", i, v[i] );
    for ( int i = 0; i < SIZE; ++i )
	printf( "\t\tx[%d] = %d\n", i, x[i] );
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

    printf( "X*       -> XMember\n" );
    printf( "%08x ->", (uint*)xptr );
    for ( int i = 0; i < sizeof(xmfp)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&xmfp)[i] );
    printf( "\n\n" );

    printf( "Q*       -> reinterpret_cast<QMember>(XMember)\n" );
    printf( "%08x ->", (uint*)qptr );
    for ( int i = 0; i < sizeof(qmfp4)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp4)[i] );
    printf( "\n\n" );

    printf( "Q*       -> (QMember)XMember\n" );
    printf( "%08x ->", (uint*)qptr );
    for ( int i = 0; i < sizeof(qmfp3)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp3)[i] );
    printf( "\n\n" );

    printf( "Q*       -> *reinterpret_cast<QMember*>(&XMember)\n" );
    printf( "%08x ->", (uint*)qptr );
    for ( int i = 0; i < sizeof(qmfp2)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp2)[i] );
    printf( "\n\n" );

    printf( "Q*       -> *((QMember*)&XMember)\n" );
    printf( "%08x ->", (uint*)qptr );
    for ( int i = 0; i < sizeof(qmfp1)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp1)[i] );
    printf( "\n\n" );
}

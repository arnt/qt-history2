#include <stdio.h>

typedef unsigned int uint;


// -------------------------

class Q {
public:
    virtual ~Q();
};

typedef void (Q::*QMember)();

Q::~Q()
{
}


// -------------------------

class V {
public:
    virtual ~V();
};

V::~V()
{
}


// -------------------------

class X : public Q, virtual public V {
public:
    virtual ~X();
    void foo();
};

typedef void (X::*XMember)();

X::~X()
{
}

void X::foo()
{
}


// -------------------------

int main() {
    X x;

    X* xptr = &x;
    Q* qptr = xptr;
    
    XMember xmfp = &X::foo;
    QMember qmfp1 = *((QMember*)&xmfp);
    QMember qmfp2 = *reinterpret_cast<QMember*>(&xmfp);
    QMember qmfp3 = (QMember)xmfp;
    QMember qmfp4 = reinterpret_cast<QMember>(xmfp);

    uint i;

    printf( "X*       -> XMember\n" );
    printf( "%08x ->", (uint)xptr );
    for ( i = 0; i < sizeof(xmfp)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&xmfp)[i] );
    printf( "\n\n" );

    printf( "Q*       -> reinterpret_cast<QMember>(XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp4)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp4)[i] );
    printf( "\n\n" );

    printf( "Q*       -> (QMember)XMember\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp3)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp3)[i] );
    printf( "\n\n" );

    printf( "Q*       -> *reinterpret_cast<QMember*>(&XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp2)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp2)[i] );
    printf( "\n\n" );

    printf( "Q*       -> *((QMember*)&XMember)\n" );
    printf( "%08x ->", (uint)qptr );
    for ( i = 0; i < sizeof(qmfp1)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp1)[i] );
    printf( "\n\n" );

    return 0;
}

#include <stdio.h>

typedef unsigned int uint;


// -------------------------

class Q {
};

typedef void (Q::*QMember)();


// -------------------------

class V {
};


// -------------------------

class X : public Q, virtual public V {
public:
    X();
    void foo();
private:
    int i;
};

typedef void (X::*XMember)();

X::X() : i( 12345 )
{
}

void X::foo()
{
    printf( "\ti       = %d\n", i );
}


// -------------------------

int main() {
    X x;

    X* xptr = &x;
    Q* qptr = xptr;
    
    XMember xmfp = &X::foo;
    QMember qmfp1 = (QMember)xmfp;
    QMember qmfp2 = reinterpret_cast<QMember>(xmfp);

    uint i;

    printf( "X* -> XMember\n" );
    printf( "\tX*      =" );
    for ( i = 0; i < sizeof(xptr)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&xptr)[i] );
    printf( "\n" );
    printf( "\tXMember =" );
    for ( i = 0; i < sizeof(xmfp)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&xmfp)[i] );
    printf( "\n" );
    (xptr->*xmfp)();
    printf( "\n" );

    printf( "Q* -> (QMember)XMember\n" );
    printf( "\tQ*      =" );
    for ( i = 0; i < sizeof(qptr)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qptr)[i] );
    printf( "\n" );
    printf( "\tQMember =" );
    for ( i = 0; i < sizeof(qmfp1)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp1)[i] );
    printf( "\n" );
    (qptr->*qmfp1)();
    printf( "\n" );

    printf( "Q* -> reinterpret_cast<QMember>(XMember)\n" );
    printf( "\tQ*      =" );
    for ( i = 0; i < sizeof(qptr)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qptr)[i] );
    printf( "\n" );
    printf( "\tQMember =" );
    for ( i = 0; i < sizeof(qmfp2)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp2)[i] );
    printf( "\n" );
    (qptr->*qmfp2)();
    printf( "\n" );

    return 0;
}

#include <stdio.h>

typedef unsigned int uint;


// -------------------------

class Q {
};

typedef void (Q::*QMember)();


// -------------------------

class C {
public:
    C( QMember );
    QMember *member() const;
private:
    QMember mbr;
};

C::C( QMember m ) : mbr(m) {
}

QMember* C::member() const {
    return (QMember*)&mbr;
}


// -------------------------

class X : public Q {
public:
    void foo( int );
};

typedef void (X::*XMember)(int);

void X::foo( int )
{
}


// -------------------------

int main() {
    C c( (QMember)&X::foo );

    typedef void (Q::*RT1)(int);

    uint i;

    printf( "reinterpret_cast<RT1>\n" );
    RT1 qmfp1 = reinterpret_cast<RT1>(*(c.member()));
    for ( i = 0; i < sizeof(qmfp1)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp1)[i] );
    printf( "\n" );

    printf( "(RT1)\n" );
    RT1 qmfp2 = (RT1)*(c.member());
    for ( i = 0; i < sizeof(qmfp2)/sizeof(uint); ++i )
	printf( " %08x", ((uint*)&qmfp2)[i] );
    printf( "\n" );

    return 0;
}

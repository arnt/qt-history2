#include "ucom.h"
#include "utypes.h"
#include "qutypes.h"
#include <qptrlist.h>

double foo( int i, double d )
{
    return i*d;
}

QString bar( const char *c, int i )
{
    return QString( "%1 -- %2" ).arg( c ).arg( i );
}

void getXY( int& x, int &y )
{
    x = 42;
    y = 43;

}
void getXYPtr( int* x, int* y )
{
    *x = 14;
    *y = 11;
}

void getName( QString& s )
{
    qDebug("getName got input: %s", s.latin1() );
    s = "Universal Component Model";
}


static const UParameter ParamFoo[] = {
    { 0, pUType_double, 0, 2 },
    { "i", pUType_int, 0, 1 },
    { "d", pUType_double, 0, 1 }
};

static const UParameter ParamBar[] = {
    { 0, pUType_QString, 0, 2 },
    { "c", pUType_charstar, 0, 1 },
    { "i", pUType_int, 0, 1 }
};

static const UParameter ParamGetXY[] = {
    { "x", pUType_int, 0, 3 },
    { "y", pUType_int, 0, 3 }
};

static const UParameter ParamGetXYPtr[] = {
    { "x", pUType_ptr, 0, 1 },
    { "y", pUType_ptr, 0, 1 }
};

static const UParameter ParamGetName[] = {
    { "s", pUType_QString, 0, 3 }
};

static const UMethod ExampleMethods[] = {
    {"foo", 3,  ParamFoo},
    {"bar", 3,  ParamBar},
    {"getXY", 2,  ParamGetXY},
    {"getXYPtr", 2,  ParamGetXYPtr},
    {"getName", 1,  ParamGetName},
};

static const UInterfaceDescription  ExampleDescription = {
    5, ExampleMethods,
    0, 0
};

URESULT invoke( int id, UObject *o )
{
    bool ok = true;

    switch ( id ) {
    case 0:
	pUType_double->set( o, foo( pUType_int->get( o+1, &ok ), pUType_double->get( o+2, &ok ) ) );
	break;
    case 1:
	pUType_QString->set( o, bar( pUType_charstar->get( o+1, &ok ), pUType_int->get( o+2, &ok ) ) );
	break;
    case 2:
	getXY( pUType_int->get( o, &ok ), pUType_int->get( o+1, &ok ) );
	break;
    case 3:
	getXYPtr( (int*)pUType_ptr->get( o, &ok ), (int*)pUType_ptr->get( o+1, &ok ) );
	break;
    case 4:
	getName( pUType_QString->get( o, &ok ) );
	break;
    default:
	return URESULT_INVALID_ID;
	break;
    }
    return ok ? URESULT_OK : URESULT_TYPE_MISMATCH;
}


int main( int /*argc*/, char** /*argv*/ )
{
    {
	UObject o[3];
	pUType_double->set( o+1, 7 );
	pUType_int->set( o+2, 3.14159268 );
	invoke( 0, o );

	double r = pUType_int->get( o );

	qDebug( "invoke(0) : %f", r );
    }
    {
	UObject o[3];
	pUType_QString->set( o+1, "Hallo" );
	pUType_int->set( o+2, 42 );
	invoke( 1, o );

	char* s = pUType_charstar->get( o );

	qDebug( "invoke(1) : %s", s );
    }

    {
	UObject o[2];
	invoke( 2, o );
	
	int x = pUType_int->get( o );
	int y = pUType_int->get( o+1 );
	
	qDebug( "invoke(2) : %d %d", x, y );
    }

    {
	UObject o[2];
	int x,y;
	pUType_ptr->set( o, &x );
	pUType_ptr->set( o+1, &y );
	invoke( 3, o );
	qDebug( "invoke(3) : %d %d", x, y );
    }

    {
	UObject o[1];
	
 	pUType_charstar->set( o, "Hello brother" );
 	invoke( 4, o );
 	const char* s = pUType_charstar->get( o );
	
	qDebug( "invoke(4) : %s", s );
    }


    return 0;
}

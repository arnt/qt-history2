#include "ucom.h"
#include "utypes.h"
#include "qutypes.h"


double foo( int i, double d )
{
    return i*d;
}

QString bar( const char *c, int i )
{
    return QString( "%1 -- %2" ).arg( c ).arg( i );
}

URESULT invoke( int id, UObject *o )
{
    bool ok = true;

    switch ( id ) {
    case 0:
	pUType_double->set( o, foo( pUType_int->get( o+1, &ok ), pUType_double->get( o+2, &ok ) ) );
	break;
    case 1:
	pUType_QString->set( o, bar( pUType_CharStar->get( o+1, &ok ), pUType_int->get( o+2, &ok ) ) );
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

	char* s = pUType_CharStar->get( o );

	qDebug( "invoke(1) : %s", s );
    }
    
    return 0;
}

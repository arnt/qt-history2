#include "ucom.h"

#define UResult int
#define URESULT_OK 0
#define URESULT_TYPE_MISMATCH -1
#define INVALID_ID 5

double foo( int i, double d )
{
    return i*d;
}

QString bar( const char *c, int i )
{
    return QString( "%1 -- %2" ).arg( c ).arg( i );
}

UResult invoke( int id, UObject *o )
{
    bool ok = true;

    switch ( id ) {
    case 0:
	pUType_Double->set( o, foo( pUType_Int->get( o+1, &ok ), pUType_Double->get( o+2, &ok ) ) );
	break;
    case 1:
	pUType_QString->set( o, bar( pUType_CharStar->get( o+1, &ok ), pUType_Int->get( o+2, &ok ) ) );
	break;
    default:
	return INVALID_ID;
	break;
    }
    return ok ? URESULT_OK : URESULT_TYPE_MISMATCH;
}

int main( int argc, char** argv )
{
    {
	UObject o[3];
	pUType_Double->set( o+1, 7 );
	pUType_Int->set( o+2, 3.14159268 );
	invoke( 0, o );

	double r = pUType_Int->get( o );

	qDebug( "invoke(0) : %f", r );
    }
    {
	UObject o[3];
	pUType_QString->set( o+1, "Hallo" );
	pUType_Int->set( o+2, 42 );
	invoke( 1, o );

	char* s = pUType_CharStar->get( o );

	qDebug( "invoke(1) : %s", s );
    }
}

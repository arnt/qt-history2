#include <stdio.h>
#include <qstring.h>


static long randomSeed = 13579;

static void setRandomSeed( int seed )
{
    if ( seed < 0 )
        seed = - seed;
    randomSeed = seed - (seed / 100000)*100000;
    if ( randomSeed == 0 )
        randomSeed = 15397;
}

static inline uchar randomValue()
{
    randomSeed = randomSeed*147;
    randomSeed = randomSeed - (randomSeed / 100000)*100000;
    if ( randomSeed == 0 )
        randomSeed = 15397;
    return randomSeed % 256;
}

static inline void scramble( uchar *data, int length )
{
    while ( length-- ) {
	*data ^= randomValue();
	data++;
    }
}

static QString unscrambleString( int seed, const uchar *data, int len )
{
    QString s( len+1 );
    memcpy( s.data(), data, len );
    setRandomSeed( seed );
    scramble( (uchar *)s.data(), len );
    return s;
}

static QByteArray scrambleString( int seed, const char *string )
{
    int len = strlen(string);
    QByteArray a(len);
    memcpy( a.data(), string, len );
    setRandomSeed( seed );
    scramble( (uchar *)a.data(), len );
    return a;
}


static void embedData( const char *name, const QByteArray &input )
{
    static char hexdigits[] = "0123456789abcdef";
    QString s( 100 );
    int nbytes = input.size();
    char *p = s.data();
    printf( "static uchar trial_data_%s[] = {", name );
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    strcpy( p, "\n    " );
	    printf( s );
	    p = s.data();
	}
	int v = (int)((uchar)input[i]);
	*p++ = '0';
	*p++ = 'x';
	*p++ = hexdigits[(v >> 4) & 15];
	*p++ = hexdigits[v & 15];
	if ( i < nbytes-1 )
	    *p++ = ',';
    }
    if ( p > s.data() ) {
	*p = '\0';
	printf( s );
    }
    printf( " };\nstatic const int trial_len_%s = %d;\n\n", name,
	    nbytes );
}


static void produceData( int seed, const char *name, const char *string )
{
    embedData( name, scrambleString(seed,string) );
}



int main( int argc, char **argv )
{
#if 0
    QString in = "hallo";
    int seed = 97579;
    QByteArray data = scrambleString(seed,in);
    QString out = unscrambleString(seed,(uchar *)data.data(),data.size());

    ASSERT( in == out );
    // return 0;
#endif

    produceData(37391,  "header",
			"Qt %s Evaluation License\n\nThis trial version may"
		        " only be used for evaluation purposes." );
    produceData(57113,	"regto",
			"Registered to:\n\n" );
    produceData(91573,	"expire",
			"The evaluation expires in %d days" );
    produceData(17517,  "copyright",
			"Contact sales@troll.no for pricing and purchasing information.\n"
		        "Qt is copyright (C) 1992-1997 by Troll Tech AS. "
		    	"All rights reserved." );
    return 0;
}

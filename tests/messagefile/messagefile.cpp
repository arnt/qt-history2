#include <qtranslator.h>
#include <stdlib.h>
#include <qfile.h>

struct {
    const char * i, * o, *s;
} messages[] = {
    { "essentials", "Essex", "establish" },
    { "heterogeneousness", "heterogenous", "heterosexual" },
    { "homosexual", "Honda", "Hondo" },
    { "impregnable", "impregnate", "impress" },
    { "middlemen", "middles", "Middlesex" },
    { "pregnancy", "pregnant", "prehistoric" },
    { "sewing", "sews", "sex" },
    { "sexed", "sexes", "sexist" },
    { "Sextans", "sextet", "sextillion" },
    { "sexton", "sextuple", "sextuplet" },
    { "sexual", "sexuality", "sexually" },
    { "sexy", "Seychelles", "Seymour" },
    { "Susquehanna", "Sussex", "sustain" },
    { 0, 0, 0 }
};


static int tries = 0;
static bool find( QTranslator & m, int i )
{
    if ( !m.find( messages[i].s, messages[i].i ).length() ) {
	warning(  "failed to find %s in %s (item %i, after %d lookups)",
		  messages[i].i, messages[i].s, i, tries );
	return FALSE;
    }
    tries++;
    return TRUE;
}


static bool ripple( QTranslator & m )
{
    int i = 0;
    while( messages[i].i ) {
	if ( !find( m, i ) )
	    return FALSE;
	i++;
    }
    int c = i;

    int j;
    for( j=0; j<1000; j++ )
	if ( !find( m, random()%c ) )
	    return FALSE;
    return TRUE;
}


static bool test( bool load )
{
    QTranslator m( 0 );
    int i;

    if ( load ) {
	m.load( "msg.tr" );
    } else {
	i = 0;
	while( messages[i].i ) {
	    m.insert( messages[i].s, messages[i].i, messages[i].o );
	    debug( "inserted %s -> %s", messages[i].i, messages[i].o );
	    i++;
	}
    }

    debug( "trying (%s)", load ? "just loaded" : "just inserted" );
    if ( !ripple( m ) )
	return FALSE;
    if ( !load ) {
	debug( "squeezing" );
	m.squeeze();
	if ( !ripple( m ) )
	    return FALSE;
    }
    debug( "unsqueezing" );
    m.unsqueeze();
    if ( !ripple( m ) )
	return FALSE;
    debug( "squeezing" );
    m.squeeze();
    if ( !ripple( m ) ) {
	m.unsqueeze();
	if ( ripple( m ) )
	    debug( "but then uns/ripple worked" );
	return FALSE;
    }

    if ( !load )
	m.save( "msg.tr" );
    return TRUE;
}



int main(int, char** )
{
    QFile f( "/dev/random" );
    char randomstuff[4];
    if ( f.open( IO_ReadOnly ) && 
	 f.readBlock( randomstuff, 4 ) == 4 )
	::srandom( randomstuff[0] + 
		   randomstuff[1] << 8 + 
		   randomstuff[2] << 16 + 
		   randomstuff[3] << 24 ); 
	
    if ( test( FALSE ) && test( TRUE ) )
	qDebug( "okay, it worked." );
}

#include <qmessagefile.h>

const char * s1 = "sex";
const char * s2 = "vold";
const char * s3 = "uff";
const char * s4 = "hei";

main(int, char** )
{
    QMessageFile m( 0 );

    m.insert( 1, s1 );
    m.insert( 123456, s2 );
    m.insert( 257, s3 );
    m.insert( 513, s4 );
    if ( !m.contains( 1 ) )
	debug( "insertion failed(1)" );
    if ( !m.contains( 123456 ) )
	debug( "insertion failed(123456)" );
    if ( !m.contains( 257 ) )
	debug( "insertion failed(257)" );
    if ( !m.contains( 513 ) )
	debug( "insertion failed(513)" );

    QString tmp;
    tmp = m.find( 1 );
    if ( tmp != s1 )
	debug( "look-up failed(1) (<%s>)", tmp.ascii() );
    tmp = m.find( 123456 );
    if ( tmp != s2 )
	debug( "look-up failed(123456) (<%s>)", tmp.ascii() );
    tmp = m.find( 257 );
    if ( tmp != s3 )
	debug( "look-up failed(257) (<%s>)", tmp.ascii() );
    tmp = m.find( 513 );
    if ( tmp != s4 )
	debug( "look-up failed(513) (<%s>)", tmp.ascii() );

    m.squeeze();
    tmp = m.find( 1 );
    if ( tmp != s1 )
	debug( "squeezed look-up failed(1) (<%s>)", tmp.ascii() );
    tmp = m.find( 123456 );
    if ( tmp != s2 )
	debug( "squeezed look-up failed(123456) (<%s>)", tmp.ascii() );
    tmp = m.find( 257 );
    if ( tmp != s3 )
	debug( "squeezed look-up failed(257) (<%s>)", tmp.ascii() );
    tmp = m.find( 513 );
    if ( tmp != s4 )
	debug( "squeezed look-up failed(513) (<%s>)", tmp.ascii() );

    m.unsqueeze();
    tmp = m.find( 1 );
    if ( tmp != s1 )
	debug( "unsqueezed look-up failed(1) (<%s>)", tmp.ascii() );
    tmp = m.find( 123456 );
    if ( tmp != s2 )
	debug( "unsqueezed look-up failed(123456) (<%s>)", tmp.ascii() );
    tmp = m.find( 257 );
    if ( tmp != s3 )
	debug( "unsqueezed look-up failed(257) (<%s>)", tmp.ascii() );
    tmp = m.find( 513 );
    if ( tmp != s4 )
	debug( "unsqueezed look-up failed(513) (<%s>)", tmp.ascii() );

    QMessageFileIterator it( m );
    if ( !it.toFirst() )
	debug( "it.toFirst failed" );
    QString *s;
    while( (s=it.current()) != 0 ) {
	debug( "saw %d: <%s> using iterator", it.currentKey(), s->ascii() );
	++it;
    }
}

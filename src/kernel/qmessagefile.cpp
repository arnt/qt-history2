/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmessagefile.cpp#1 $
**
** Localization database support.
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qmessagefile.h"

#if defined(UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#else
// appropriate stuff here
#endif

const int headertablesize = 256; // entries, must be power of two


static inline int readoffset( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 17) +
	    ((uchar)(c[o+1]) << 9) +
	    ((uchar)(c[o+2]) << 1));
}


static inline int readlength( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 16) +
	    ((uchar)(c[o+1]) << 8) +
	    ((uchar)(c[o+2])));
}


static inline int readhash( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 24) +
	    ((uchar)(c[o+1]) << 16) +
	    ((uchar)(c[o+2]) << 8) +
	    ((uchar)(c[o+3])));
}



/*!  Constructs an empty message file, not connected to any file.
*/

QMessageFile::QMessageFile( QObject * parent, const char * name )
    : QObject( parent, name )
{
    t = 0;
    l = 0;
}


/*! Destroys the object and frees any allocated resources.
*/

QMessageFile::~QMessageFile()
{
    // delete/free t
}


/*!  Open 

*/

void QMessageFile::open( const QString & filename, const QString & directory )
{
    QString realname( filename );
    if ( directory.length() ) {
	if ( realname[0] != '/' ) { // also check \ and c:\ on windows
	    if ( directory[qstrlen(directory)-1] != '/' )
		realname.prepend( "/" );
	    realname.prepend( directory );
	}
    }

#if defined(UNIX)
    // unix

    //const char * lang = getenv( "LANG" );

    int f;

    f = ::open( realname.ascii(), O_RDONLY );
    if ( f < 0 ) {
	debug( "can't open %s: %s", realname.ascii(), strerror( errno ) );
	return;
    }

    struct stat st;
    if ( fstat( f, &st ) ) {
	debug( "can't stat %s: %s", realname.ascii(), strerror( errno ) );
	return;
    }
    char * tmp;
    tmp = (char*)mmap( 0, st.st_size, // any address, whole file
		       PROT_READ, // read-only memory
		       MAP_FILE | MAP_PRIVATE, // swap-backed map from file
		       f, 0 ); // from offset 0 of f
    if ( !tmp ) {
	debug( "can't mmap %s: %s", filename, strerror( errno ) );
	return;
    }

    if ( t )
	munmap( (void *) t, l );
    
    ::close( f );

    t = (const char *) tmp;
    l = st.st_size;
#else
    // windows
#endif
}


/*!

*/

QString QMessageFile::find( const QString & key )
{
    if ( !t || !l )
	return key;

    int h = hash( key );
    int o = readoffset( t, 3*(h&(headertablesize-1)) );
    while( o && o+7 < l && readhash( t, o ) != h ) {
	if ( o + 7 >= l )
	    o = 0;
	else
	    o = readoffset( t, o+3 );
    }
    if ( !o )
	return key;
    
    // inefficiency below... we could use a "shallow copy, detach on
    // change, do not free this" constructor if the byte order
    // matches.

    int sl = readlength( t, o+7 );
    QString r( sl );
    int i, j;
    j = o+10;
    for( int i=0; i<sl; i++ )
	r[i] = (o[j++] << 8) + o[j++];
    return r;
}


/*!  Returns a

*/

uint QMessageFile::hash( const QString & key )
{
    uint h = 0;
    uint g;
    int i;
    for( i=i<key.length()-1; i >= 0; i-- ) {
	h = (h<<4) + key[i];
	if ( (g = h & 0xf0000000) )
	    h ^= g >> 16;
	h &= ~g;
    }
    return h;
}

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmessagefile.cpp#8 $
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

// for mmap
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

// for htonl
#include <netinet/in.h>

// for close
#include <unistd.h>

#else
// appropriate stuff here
#endif

// for qsort
#include <stdlib.h>

// other qt stuff necessary for the implementation
#include "qintdict.h"
#include "qstring.h"
#include "qapp.h"
#include "qfile.h"
#include "qdatastream.h"
#include "qdict.h"

/*
$ mcookie
3cb86418caef9c95cd211cbf60a1bddd
$
*/

const uchar magic[] = { // magic number for the file.
    0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
    0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};

const int headertablesize = 256; // entries, must be power of two


class QMessageFilePrivate {
public:
    QMessageFilePrivate() :
	messages( 0 ),
	t( 0 ), l( 0 ),
	unmapPointer( 0 ), unmapLength( 0 ),
	byteArray( 0 ) {}
    // note: QMessageFile must finalize this before deallocating it.

    struct SortableMessage {
	QString result;
	uint hash;
    };

    // for read-write message files
    QIntDict<QString> * messages;

    // for read-only files
    const char * t;
    uint l;

    // for mmap'ed files, this is what needs to be unmapped.
    void * unmapPointer;
    unsigned int unmapLength;

    // for squeezed but non-file data, this is what needs to be deleted
    QByteArray * byteArray;

    // the headers dict
    QDict<QString> headers;
};


/* format of the data file.

   headertablesize 3-byte offsets
   headertablezize per-hash structures:
       3-byte offset of first string
       for each message with this hash, sorted by rest-of-hash
           3-byte rest-of-hash
	   3-byte length-of-string
       for each message with this hash, sorted by rest-of-hash
           actual string (in utf16; should be in reuters format)
   eof

   this format permits very fast lookup.
*/

static inline uint readoffset( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 17) +
	    ((uchar)(c[o+1]) << 9) +
	    ((uchar)(c[o+2]) << 1));
}


static inline uint readlength( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 16) +
	    ((uchar)(c[o+1]) << 8) +
	    ((uchar)(c[o+2])));
}


static inline uint readhash( const char * c, int o, uint base ) {
    return (((uchar)(c[o  ]) << 24) +
	    ((uchar)(c[o+1]) << 16) +
	    ((uchar)(c[o+2]) << 8) +
	    base);
}



/*!  Constructs an empty message file, not connected to any file.
*/

QMessageFile::QMessageFile( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = new QMessageFilePrivate;
}


/*! Destroys the object and frees any allocated resources.
*/

QMessageFile::~QMessageFile()
{
    if ( qApp && parent() == qApp )
	qApp->removeMessageFile( this );
    clear();
    delete d;
}


/*!  Loads \a filename, which may be an absolute file name or relative
  to \a directory.
*/

void QMessageFile::load( const QString & filename, const QString & directory )
{
    clear();
    squeeze();

    QString realname( filename );
    if ( directory.length() ) {
	if ( realname[0] != '/' ) { // also check \ and c:\ on windows
	    if ( directory[int(qstrlen(directory)-1)] != '/' )
		realname.prepend( "/" );
	    realname.prepend( directory );
	}
    }

    const char * t;
    uint l;
#if defined(UNIX)
    // unix (if mmap supported)

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
	debug( "can't mmap %s: %s", filename.ascii(), strerror( errno ) );
	return;
    }

    ::close( f );

    d->unmapPointer = tmp;
    d->unmapLength = st.st_size;
    t = ((const char *) tmp)+16; // 16 being the length of the magic number
    l = d->unmapLength - 16;
#else
    // windows
    fatal("Not written yet -- contact agulbra@troll.no");
#endif

    // now that we've read it and all, check that it has the right
    // magic number, and forget all about it if it doesn't.
    if ( memcmp( (const void *)(d->unmapPointer), magic, 16 ) )
	clear();

    // then we go on to read in the headers... I'd prefer not, but...
}


/*!  Saves this message file to \a filename, overwriting the previous
  contents of \a filename.

  \sa load()
*/

void QMessageFile::save( const QString & filename )
{
    QFile f( filename );
    if ( f.open( IO_WriteOnly ) ) {
	QDataStream s( &f );

	// magic number
	if ( f.writeBlock( (const char *)magic, 16 ) < 16 )
	    return;

	// header strings
	s << d->headers.count();
	QDictIterator<QString> it( d->headers );
	QString * c;
	const char * k;
	while( (c=it.current()) != 0 ) {
	    k = it.currentKey();
	    ++it;
	    s << *k << *c;
	}

	// the rest
	squeeze();
	f.writeBlock( d->t, d->l );
    }
}


/*!
  \fn QString QMessageFile::find( uint h, const char* scope, const char* key ) const

  Returns the string matching hash code \a h, or QString::null in
  case there is no string for \a h.

  The \a scope and \a key arguments are not used in the default implementation,
  but are available for QMessageFile subclasses to use
  alternative translation techniques.
*/

QString QMessageFile::find( uint h, const char*, const char* ) const
{
    if ( d->messages ) {
	QString * r1 = d->messages->find( h );
	if ( r1 )
	    return *r1;
	return QString::null;
    }
	
    if ( !d->t || !d->l )
	return QString::null;

    // offset we care about at any instant
    uint o = readoffset( d->t, 3*(h % headertablesize) );
    // if that bucket is empty, return quickly
    if ( o+10 >= d->l || o < 3*headertablesize )
	return QString::null;
    // string pointer, first string pointer
    uint fsp, sp;
    fsp = sp = readoffset( d->t, o );
    uint base = h % headertablesize;
    o += 3;
    uint r;
    while ( o+5 < fsp && (r=readhash( d->t, o, base )) < h ) {
	// not yet found the one we want
	sp += 2*readlength( d->t, o+3 );
	o += 6;
    }
    if ( o+5 < fsp && r == h ) {
	// here it is.  let's return it.
	int sl = readlength( d->t, o+3 );
	QString result( sl );
	int i;
	for( i=0; i<sl; i++ ) {
	    uchar row =  d->t[sp++];
	    uchar cell =  d->t[sp++];
	    result[i] = QChar( cell, row );
	}
	return result;
    }
    return QString::null;
}


/*!  Returns a hash og \a scope and \a name.  Neither of the two may
  be null (though hash() does not crash if they are).  The result of
  the hash function is never 0.

  This function will not change; you may rely on its output to remain
  the same in future versions of Qt.
*/

uint QMessageFile::hash( const char * scope, const char * name )
{
    const char *k;
    uint h = 0;
    uint g;

    // scope
    if ( scope ) {
	k = scope;
	while ( *k ) {
	    h = (h<<4) + *k++;
	    if ( (g = h & 0xf0000000) )
		h ^= g >> 24;
	    h &= ~g;
	}
    }

    // null between the two
    h = h<<4;
    if ( (g = h & 0xf0000000) )
	h ^= g >> 24;
    h &= ~g;

    // scope
    if ( name ) {
	k = name;
	while ( *k ) {
	    h = (h<<4) + *k++;
	    if ( (g = h & 0xf0000000) )
		h ^= g >> 24;
	    h &= ~g;
	}
    }

    if ( !h )
	h = 1;

    return h;
}


/*!  Empties this message file of all contents.
*/

void QMessageFile::clear()
{
    if ( d->t ) {
	if ( d->unmapPointer && d->unmapLength ) {
#if defined(UNIX)
	    munmap( d->unmapPointer, d->unmapLength );
	    d->unmapPointer = 0;
	    d->unmapLength = 0;
#else
	    // windows stuff here.
#endif
	}
	if ( d->byteArray ) {
	    delete d->byteArray;
	    d->byteArray = 0;
	}
	d->t = 0;
	d->l = 0;
    } else {
	delete d->messages;
	d->messages = 0;
    }
}


static inline void writethreebytes( QByteArray & b, uint o, uint d )
{
    b[(int)o  ] = (d&0xff0000) >> 16;
    b[(int)o+1] = (d&0x00ff00) >>  8;
    b[(int)o+2] = (d&0x0000ff);
}


static inline void writeoffset( QByteArray & b, uint o, uint d ) {
    if ( d & 1 )
	fatal( "oops! wanted to write offset %6x, which is odd", d );
    writethreebytes( b, o, d/2 );
}


// note: we want reverse sorting, hence the strange return values
static int cmp( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    QMessageFilePrivate::SortableMessage * m1
	= (QMessageFilePrivate::SortableMessage *)n1;
    QMessageFilePrivate::SortableMessage * m2
	= (QMessageFilePrivate::SortableMessage *)n2;

    if ( (m1->hash % headertablesize) < (m2->hash % headertablesize) )
	return 1;
    else if ( (m1->hash % headertablesize) > (m2->hash % headertablesize) )
	return -1;
    else if ( m1->hash < m2->hash )
	return 1;
    else if ( m1->hash > m2->hash )
	return -1;
    else
	return 0;
}


/*!  Converts this message file to the compact format used to store
  message files on disk.  This causes all string keys to be forgotten
  (the compact format keeps only the hash code and result strings).
*/

void QMessageFile::squeeze()
{
    if ( !d->messages )
	return;

    uint size = headertablesize * 3;

    QMessageFilePrivate::SortableMessage * items
	= new QMessageFilePrivate::SortableMessage[ d->messages->count() ];

    QIntDictIterator<QString> it( *(d->messages) );
    QString * s;
    int i = 0;
    while( (s=it.current()) != 0 ) {
	items[i].result = *s;
	items[i].hash = it.currentKey();
	++it;
	++i;
	size = size + 10 + 2*s->length();
    }

    ::qsort( items, d->messages->count(),
	     sizeof( QMessageFilePrivate::SortableMessage ), cmp );

    QByteArray b( size );
    b.fill( '\0' );
    uint fp = 3*headertablesize;
    // this is where the header should be written

    i = d->messages->count()-1;
    while( i >= 0 ) {
	uint bucket = items[i].hash % headertablesize;
	fp = ((fp-1) | 3)+1;
	writeoffset( b, 3*bucket, fp );
	int j = 0;
	while( j <= i && bucket == items[i-j].hash % headertablesize )
	    j++;
	if ( j ) {
	    uint sp = fp + 4 + (6*j);
	    writeoffset( b, fp, sp );
	    fp += 3;
	    QString res;
	    while( j ) {
		writethreebytes( b, fp, items[i].hash / headertablesize );
		writethreebytes( b, fp+3, items[i].result.length() );
		fp += 6;
		res = items[i].result;
		uint k;
		for( k=0; k<res.length(); k++ ) {
		    b[(int)sp++] = res[(int)k].row;
		    b[(int)sp++] = res[(int)k].cell;
		}
		i--;
		j--;
	    }
	    fp = sp;
	}
    }
    if ( fp < size )
	b.resize( fp );
    clear();
    d->byteArray = new QByteArray( b );
    d->t = b.data();
    d->l = b.size();
}


/*!  Converts this message file into an easily modifiable data
  structure, less compact than the format used in the files.

  You should never need to call this function; it is called by
  insert() etc. as necessary.

  \sa squeeze()
*/

void QMessageFile::unsqueeze()
{
    if ( d->messages )
	return;
    QIntDict<QString> * messages = new QIntDict<QString>( 4271 );
    messages->setAutoDelete( TRUE );

    if ( d->t && d->l ) {
	int i;
	for( i=0; i<headertablesize; i++ ) {
	    uint fp = readoffset( d->t, i*3 );
	    if ( fp+10 <= d->l && fp >= 3*headertablesize ) {
		uint sp = readoffset( d->t, fp );
		fp += 3;
		uint fsp = sp;
		while( fp+5 < fsp ) {
		    uint h = readhash( d->t, fp, i );
		    int sl = readlength( d->t, fp+3 );
		    QString result( sl );
		    int k;
		    for( k=0; k<sl; k++ ) {
			uchar row = d->t[sp++];
			uchar cell = d->t[sp++];
			result[k] = QChar( cell, row );
		    }
		    messages->insert( (long)h, new QString( result ) );
		    fp += 6;
		}
	    }
	}
	clear();
    }
    d->messages = messages;
}


/*!  Returns TRUE if this message file contains a message with hash
  value \a h, and FALSE if it does not.

  (This is is a one-liner than calls find().)
*/

bool QMessageFile::contains( uint h ) const
{
    return find( h ) != QString::null;
	
}


/*!  Inserts \a s with hash value \a h into this message file,
  replacing any current string for \a h.
*/

void QMessageFile::insert( uint h, const QString & s )
{
    unsqueeze();
    d->messages->replace( (long)h, new QString( s ) );
}


/*!  Removes the string for \a h from this message file.  If there is
  no string for h, this function does nothing.

*/

void QMessageFile::remove( uint h )
{
    unsqueeze();
    d->messages->remove( h );
}


/*! \class QMessageFileIterator qmessagefile.h

  \brief The QMessageFileIterator class provides the ability to list QMessageFile contents etc.

  Normally not needed, and still not documented.
*/
  
/*!  Constructs a QMessageFileIterator that operates on \a m */

QMessageFileIterator::QMessageFileIterator( QMessageFile & m )
{
    m.unsqueeze();
    it = new QIntDictIterator<QString>( *(m.d->messages) );
}


/*! Destroys the iterator and frees any allocated resources. */

QMessageFileIterator::~QMessageFileIterator()
{
    delete it;
    it = 0;
}


/*! Returns the number of items in the message file this iterator
  operates on.

  \sa isEmpty()
*/

uint QMessageFileIterator::count() const
{
    return it->count();
}


/*!  Returns TRUE if the message file on which this iterator operates
  is empty, and FALSE if it contains at least one message.

*/

bool QMessageFileIterator::isEmpty() const
{
    return it->isEmpty();
}


/*! Sets the current iterator item to point to the first item in the
  message file and returns a pointer to the item.  If the message file
  is empty it sets the current item to null and returns a null string.
*/

QString * QMessageFileIterator::toFirst()
{
    return it->toFirst();
}


/*! Returns a pointer to the current iterator item. */

QString * QMessageFileIterator::current() const
{
    return it->current();
}


/* Returns the key for the current iterator item. */

uint QMessageFileIterator::currentKey() const
{
    return it->currentKey();
}


/*! Cast operator. Returns a pointer to the current iterator item.
  Same as current(). */

QMessageFileIterator::operator QString *() const
{
    return it->current();
}




/*!  Prefix ++ makes the succeeding item current and returns the new current
  item.

  If the current iterator item was the last item in the dictionary or if it
  was null, null is returned.
*/

QString * QMessageFileIterator::operator++()
{
    return ++*it;
}


/*!  Sets the current item to the item \e jump positions after the
  current item, and returns a pointer to that item.

  If that item is beyond the last item or if the dictionary is  empty,
  it sets the current item to null and returns null.
*/

QString * QMessageFileIterator::operator+=( uint jump )
{
    return *it += jump;
}

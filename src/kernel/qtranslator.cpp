/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.cpp#21 $
**
** Localization database support.
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qtranslator.h"
#include "qfileinfo.h"

#if defined(UNIX)
#define QT_USE_MMAP
#endif

#if defined(QT_USE_MMAP)

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
#include "qapplication.h"
#include "qfile.h"
#include "qdatastream.h"
#include "qdict.h"
#include "qmap.h"

/*
$ mcookie
3cb86418caef9c95cd211cbf60a1bddd
$
*/

static const int magic_length = 16;
static const uchar magic[magic_length] = { // magic number for the file.
    0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
    0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};

const int headertablesize = 256; // entries, must be power of two

struct QTranslationDomain {
    QTranslationDomain() { }
    QTranslationDomain(const char* s, const char* m) :
	scope(s), message(m), hash(QTranslator::hash(s,m)) { }
    QCString scope;
    QCString message;
    uint hash;
    bool operator <(const QTranslationDomain& o) const
    {
	int r = hash - o.hash;
	if ( r ) return r < 0;
	r = strcmp(scope,o.scope);
	return r < 0;
    }
};

typedef QMap<QTranslationDomain,QString>::Iterator QTranslatorIteratorPrivate;

class QTranslatorPrivate {
public:
    QTranslatorPrivate() :
	messages( 0 ),
	t( 0 ), s( 0 ), l( 0 ),
	unmapPointer( 0 ), unmapLength( 0 ),
	byteArray( 0 ) {}
    // note: QTranslator must finalize this before deallocating it.

    struct SortableMessage {
	QCString scope;
	QString translation;
	uint hash;
    };

    // for read-write message files
    QMap<QTranslationDomain,QString> * messages;

    // for read-only files
    const char * t; // hash table
    const char * s; // scope table
    uint l;

    // for mmap'ed files, this is what needs to be unmapped.
    char * unmapPointer;
    unsigned int unmapLength;

    // for squeezed but non-file data, this is what needs to be deleted
    QByteArray * byteArray;

    // the headers dict
    QDict<QString> headers;
};


/* format of the data file.

   file                = MAGIC [1] scope-table [2] hash-table .
   scope-table         = scope-table-size { { CHAR8 } NUL } .
   scope-table-size    = INT24 .
   hash-table          = { bucket-offset }*256 { bucket } .
   bucket-offset       = INT24 .
   bucket              = first-string-offset offset-list { target } .
   offset-list         = { rest-of-hash string-length } .
   first-string-offset = INT24 .
   rest-of-hash        = INT24 .
   string-length       = INT24 .
   target              = scope { CHAR16 } .
   scope               = INT24 .

   The scope-table contains all the scopes, as a list of simple
   C strings.  The target.scope is the offset into this list, measured
   from [1] (ie. the first scope is at offset WORDSIZE).

   The bucket-offset offsets in hash-table are offsets into the
   hash-table.bucket data, measured from [2] (ie. the first scope is at
   offset WORDSIZE*256).

   Each string-length is the count of CHAR16 in the corresponding
   target - by adding up these lengths, plus WORDSIZE bytes for the scope
   of each target, the offset into the bucket.target data (measured
   from [2]) can be calculated.

   d->s = [1]
   d->t = [2]
   d->l = length(hash-table)

   This format permits very fast lookup.
*/

static const int WORDSIZE = 3;

// Dependendent on WORDSIZE
static inline uint readoffset( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 17) +
	    ((uchar)(c[o+1]) << 9) +
	    ((uchar)(c[o+2]) << 1));
}


// Dependendent on WORDSIZE
static inline uint readlength( const char * c, int o ) {
    return (((uchar)(c[o  ]) << 16) +
	    ((uchar)(c[o+1]) << 8) +
	    ((uchar)(c[o+2])));
}


// Dependendent on WORDSIZE
static inline uint readhash( const char * c, int o, uint base ) {
    return (((uchar)(c[o  ]) << 24) +
	    ((uchar)(c[o+1]) << 16) +
	    ((uchar)(c[o+2]) << 8) +
	    base);
}



/*! \class QTranslator qtranslator.h

  \brief The QTranslator class provides internationalization support for text output.

  The class is conceptually very simple: An object of this class
  contains a set of translations from the reference language to a
  target language, and provides functions to add, look up and remove
  such translations as well as the ability to load and save the object
  to a file.

  The most common use of QTranslator is expected to be loading one
  from a file, installing it using QApplication::installTranslator(),
  and using it via QObject::tr().

  Slightly more advanced usage of QTranslator includes direct lookup
  using find() (with input almost invariably provided by hash()),
  adding new translations using insert() and removing existing ones
  using remove() or even clear(), and testing whether the QTranslator
  contains a translation using contains().

  The hash() function mentioned is a variant on the standard ELF hash,
  modified to work well with Unicode strings in UCS-2 format.  Its
  algorithm is not specified beyond the fact that it will remain
  unchanged in future versions of Qt.

  To examine the contents of a QTranslator, use QTranslatorIterator.

  \sa QTranslatorIterator QApplication::installTranslator QApplication::removeTranslator() QObject::tr() QApplication::translate()
*/


/*!  Constructs an empty message file, not connected to any file.
*/

QTranslator::QTranslator( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = new QTranslatorPrivate;
}


/*! Destroys the object and frees any allocated resources.
*/

QTranslator::~QTranslator()
{
    if ( qApp && parent() == qApp )
	qApp->removeTranslator( this );
    clear();
    delete d;
}


/*!  Loads \a filename, which may be an absolute file name or relative
  to \a directory.  If the full filename does not exist, other filenames
  are tried in the following order:
  <ol>
   <li>Filename with \a suffix appended (".qm" if suffix is QString::null)
   <li>Filename with text after a character in \a search_delimiters stripped
	("_." is the default for \a search_delimiters if it is QString::null)
   <li>Filename stripped and \a suffix appended.
   <li>Filename stripped further, etc.
  </ol>
  For example, load("foo_bar.baz", "/opt/foolib") will search for:
  <ol>
   <li>/opt/foolib/foo_bar.baz
   <li>/opt/foolib/foo_bar.baz.qm
   <li>/opt/foolib/foo_bar
   <li>/opt/foolib/foo_bar.qm
   <li>/opt/foolib/foo
   <li>/opt/foolib/foo.qm
  </ol>
*/

bool QTranslator::load( const QString & filename, const QString & directory,
		        const QString & search_delimiters,
		        const QString & suffix )
{
    clear();
    squeeze();

    QString prefix;

    if ( filename[0] == '/'
#ifdef _WS_WIN_
	|| filename[0] && filename[1] == ':'
	|| filename[0] == '\\'
#endif
    )
	prefix = QString::fromLatin1("");
    else
	prefix = directory;

    if ( prefix.length() ) {
	if ( prefix[int(prefix.length()-1)] != '/' )
	    prefix += QChar('/');
    }

    QString fname = filename;
    QString realname;
    QString delims;
    delims = search_delimiters.isNull() ?
	QString::fromLatin1("_.") : search_delimiters;

    // COMPLICATED LOOP
    try_with_and_without_suffix:
    {
	QFileInfo fi;

	realname = prefix + fname;
	fi.setFile(realname);
	if ( fi.isReadable() )
	    goto found_file; // EXIT LOOP

	realname += suffix.isNull() ? QString::fromLatin1(".qm") : suffix;
	fi.setFile(realname);
	if ( fi.isReadable() )
	    goto found_file; // EXIT LOOP

	for ( int i=0; i<(int)delims.length(); i++) {
	    int dlm;
	    if ( (dlm=fname.find(delims[i])) >= 0 ) {
		// found a truncation
		fname = fname.left(dlm);
		goto try_with_and_without_suffix;
	    }
	}

	// No truncations - fail
	return FALSE;
    }
    found_file: ; // END OF COMPLICATED LOOP


    // realname is now the fully qualified name of a readable file.


#if defined(QT_USE_MMAP)
    // unix (if mmap supported)

// ###### Arnt: seems some platforms don't define these
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

    //const char * lang = getenv( "LANG" );

    int f;

    f = ::open( QFile::encodeName(realname), O_RDONLY );
    if ( f < 0 ) {
	// debug( "can't open %s: %s", realname.ascii(), strerror( errno ) );
	return FALSE;
    }

    struct stat st;
    if ( fstat( f, &st ) ) {
	// debug( "can't stat %s: %s", realname.ascii(), strerror( errno ) );
	return FALSE;
    }
    char * tmp;
    tmp = (char*)mmap( 0, st.st_size, // any address, whole file
		       PROT_READ, // read-only memory
		       MAP_FILE | MAP_PRIVATE, // swap-backed map from file
		       f, 0 ); // from offset 0 of f
    if ( !tmp || tmp == (char*)MAP_FAILED ) {
	// debug( "can't mmap %s: %s", filename.ascii(), strerror( errno ) );
	// #### could revert to file io?
	return FALSE;
    }

    ::close( f );

    d->unmapPointer = tmp;
    d->unmapLength = st.st_size;
#else
    // windows, or unix without mmap
    QFile f(realname);
    d->unmapLength = f.size();
    d->unmapPointer = new char[d->unmapLength];
    bool ok = FALSE;
    if ( f.open(IO_ReadOnly) ) {
	ok = d->unmapLength ==
		    (uint)f.readBlock( d->unmapPointer, d->unmapLength );
	f.close();
    }
    if ( !ok ) {
	delete [] d->unmapPointer;
	d->unmapPointer = 0;
	return FALSE;
    }
#endif

    d->s = ((const char *) d->unmapPointer)+magic_length;
    int scope_table_size = readlength( d->s, 0 );
    d->t = d->s + WORDSIZE + scope_table_size;
    d->l = d->unmapLength - magic_length - WORDSIZE - scope_table_size;

    // now that we've read it and all, check that it has the right
    // magic number, and forget all about it if it doesn't.
    if ( memcmp( (const void *)(d->unmapPointer), magic, magic_length ) ) {
	clear();
	return FALSE;
    }

    // then we go on to read in the headers... I'd prefer not, but...

    return TRUE;
}


/*!  Saves this message file to \a filename, overwriting the previous
  contents of \a filename.

  \sa load()
*/

bool QTranslator::save( const QString & filename )
{
    QFile f( filename );
    if ( f.open( IO_WriteOnly ) ) {
	// magic number
	if ( f.writeBlock( (const char *)magic, magic_length ) < magic_length )
	    return FALSE;

	// the rest
	squeeze();
	f.writeBlock( d->s, (d->t - d->s) + d->l );
	return TRUE;
    }
    return FALSE;
}


/*!
  \fn QString QTranslator::find( uint h, const char* scope, const char* key ) const

  Returns the string matching hash code \a h, or QString::null in
  case there is no string for \a h.

  The \a scope and \a key arguments are not used in the default
  implementation, but are available for QTranslator subclasses to use
  in alternative translation techniques.
*/

QString QTranslator::find( uint h, const char* scope, const char* message ) const
{
    if ( d->messages ) {
	QTranslationDomain k(scope,message);
	QMap<QTranslationDomain,QString>::Iterator it = d->messages->find(k);
	if ( it !=  d->messages->end() )
	    return *it;
	return QString::null;
    }
	
    if ( !d->t || !d->l )
	return QString::null;

    const char* s = d->s;
    const char* t = d->t;

    // offset we care about at any instant
    uint o = readoffset( t, WORDSIZE*(h % headertablesize) );
    // if that bucket is empty, return quickly
    if ( o+10 >= d->l || o < WORDSIZE*headertablesize )
	return QString::null;
    // string pointer, first string pointer
    uint fsp, sp;
    fsp = sp = readoffset( s, o );
    uint base = h % headertablesize;
    o += WORDSIZE;
    uint r;
    while ( o+5 < fsp && (r=readhash( s, o, base )) < h ) {
	// not yet found the one we want
	sp += 2*readlength( s, o+WORDSIZE )+WORDSIZE;
	o += 2*WORDSIZE;
    }
    if ( o+5 < fsp && r == h ) {
	// match found - check the scope
	int sl = readlength( s, o+WORDSIZE )-WORDSIZE;
	int sc = readoffset( s, sp );
	if ( strcmp(scope, d->s + WORDSIZE + sc)!=0 ) {
	    // bad match.
	    return QString::null;
	}
	// matched - return it
	sp += WORDSIZE;
	QString result;
	// ### could use QConstString if byte order is ok, but need somewhere
	// ###  to keep the QConstString in existence and to reuse it from.
	int i;
	for( i=0; i<sl; i++ ) {
	    uchar row =  s[sp++];
	    uchar cell =  s[sp++];
	    result += QChar( cell, row );
	}
	return result;
    }
    return QString::null;
}


/*!  Returns a hash of \a scope and \a name.  Neither of the two may
  be null (in which case the return value is unspecified).  The result of
  the hash function is never 0.

  This function will not change; you may rely on its output to remain
  the same in future versions of Qt.
*/

uint QTranslator::hash( const char * scope, const char * name )
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

    // name
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

void QTranslator::clear()
{
    if ( d->t ) {
	if ( d->unmapPointer && d->unmapLength ) {
#if defined(QT_USE_MMAP)
	    munmap( d->unmapPointer, d->unmapLength );
#else
	    delete [] d->unmapPointer;
#endif
	    d->unmapPointer = 0;
	    d->unmapLength = 0;
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


// Dependent on WORDSIZE
static inline void writethreebytes( QByteArray & b, uint o, uint d )
{
    b[(int)o  ] = (d&0xff0000) >> 16;
    b[(int)o+1] = (d&0x00ff00) >>  8;
    b[(int)o+2] = (d&0x0000ff);
}


static inline void writeoffset( QByteArray & b, uint o, uint d ) {
    if ( d & 1 )
	qFatal( "oops! wanted to write offset %6x, which is odd", d );
    writethreebytes( b, o, d/2 );
}


// note: we want reverse sorting, hence the strange return values
static int cmp( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    QTranslatorPrivate::SortableMessage * m1
	= (QTranslatorPrivate::SortableMessage *)n1;
    QTranslatorPrivate::SortableMessage * m2
	= (QTranslatorPrivate::SortableMessage *)n2;

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
  message files on disk.  You should never need to call this directly;
  save() and other functions call it as necessary.
*/

void QTranslator::squeeze()
{
    if ( !d->messages )
	return;

    QMap<QCString,int> scope_offsets;

    uint size = headertablesize * 2*WORDSIZE;

    QTranslatorPrivate::SortableMessage * items
	= new QTranslatorPrivate::SortableMessage[ d->messages->count() ];

    int i = 0;
    int scope_table_size = 0;
    {
	QMap<QTranslationDomain,QString>::Iterator it = d->messages->begin();
	while( it != d->messages->end() ) {
	    items[i].scope = it.key().scope;
	    items[i].translation = *it;
	    items[i].hash = it.key().hash;
	    size += 10 + WORDSIZE + 2 * (*it).length();
	    if ( !scope_offsets.contains(it.key().scope) ) {
		scope_offsets.insert(it.key().scope, scope_table_size);
		scope_table_size += it.key().scope.length()+1;
	    }
	    ++it;
	    ++i;
	}
    }
    size += scope_table_size + WORDSIZE;

    ::qsort( items, d->messages->count(),
	     sizeof( QTranslatorPrivate::SortableMessage ), cmp );

    QByteArray b( size );
    b.fill( '\0' );

    writethreebytes( b, 0, scope_table_size );
    uint sc = WORDSIZE + scope_table_size;
    uint fp = sc + WORDSIZE*headertablesize;

    {
	QMap<QCString,int>::Iterator it = scope_offsets.begin();
	while ( it != scope_offsets.end() ) {
	    memcpy( b.data()+WORDSIZE+*it, it.key().data(), it.key().length()+1 );
	    ++it;
	}
    }

    i = d->messages->count()-1;
    while( i >= 0 ) {
	uint bucket = items[i].hash % headertablesize;
	fp = ((fp-1) | 3)+1;
	writeoffset( b, sc+WORDSIZE*bucket, fp );
	int j = 0;
	while( j <= i && bucket == items[i-j].hash % headertablesize )
	    j++;
	if ( j ) {
	    uint sp = fp + 4 + (2*WORDSIZE*j);
	    writeoffset( b, fp, sp );
	    fp += WORDSIZE;
	    QString str;
	    while( j ) {
		str = items[i].translation;
		writethreebytes( b, fp, items[i].hash / headertablesize );
		writethreebytes( b, fp+WORDSIZE, str.length()+WORDSIZE );
		fp += 2*WORDSIZE;
		uint k;
		writethreebytes( b, sp, scope_offsets[items[i].scope] );
		sp += WORDSIZE;
		for( k=0; k<str.length(); k++ ) {
		    b[(int)sp++] = str[(int)k].row();
		    b[(int)sp++] = str[(int)k].cell();
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
    d->s = b.data();
    d->t = d->s+WORDSIZE+scope_table_size;
    d->l = b.size();
}


/*!  Converts this message file into an easily modifiable data
  structure, less compact than the format used in the files.

  You should never need to call this function; it is called by
  insert() etc. as necessary.

  \sa squeeze()
*/

void QTranslator::unsqueeze()
{
    if ( d->messages )
	return;
    QMap<QTranslationDomain,QString> * messages =
	new QMap<QTranslationDomain,QString>;

    if ( d->t && d->l ) {
	int i;
	for( i=0; i<headertablesize; i++ ) {
	    uint fp = readoffset( d->t, i*WORDSIZE );
	    if ( fp+10 <= d->l && fp >= WORDSIZE*headertablesize ) {
		uint sp = readoffset( d->t, fp );
		fp += WORDSIZE;
		uint fsp = sp;
		while( fp+5 < fsp ) {
		    uint h = readhash( d->t, fp, i );
		    int sl = readlength( d->t, fp+WORDSIZE );
		    QString result;
		    int k;
		    int sc = readlength( d->t, sp );
		    sp += WORDSIZE;
		    for( k=0; k<sl; k++ ) {
			uchar row = d->t[sp++];
			uchar cell = d->t[sp++];
			result[k] = QChar( cell, row );
		    }
		    QTranslationDomain key;
		    key.scope = d->s + WORDSIZE + sc;
		    key.hash = h;
		    messages->insert( key, result );
		    fp += 2*WORDSIZE;
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

bool QTranslator::contains( const char* scope, const char* key ) const
{
    uint h = hash(scope,key);
    return find( h, scope, key ) != QString::null;
}


/*! Inserts \a translation of \a message
  into this message file.
*/

void QTranslator::insert( const char* scope,
			  const char* message,
			  const QString& translation )
{
    unsqueeze();
    QTranslationDomain k( scope, message );
    d->messages->replace( k, translation );
}


/*!  Removes the string for \a h from this message file.  If there is
  no string for h, this function does nothing.

*/

void QTranslator::remove( const char *scope, const char *message )
{
    unsqueeze();
    QTranslationDomain k( scope, message );
    d->messages->remove( k );
}



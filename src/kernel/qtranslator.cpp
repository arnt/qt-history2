/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.cpp#36 $
**
** Localization database support.
**
** Created : 980906
**
** Copyright (C) 1998-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/


#include "qtranslator.h"

#ifndef QT_NO_TRANSLATION

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
#include "qbuffer.h"
#include "qdatastream.h"
#include "qmap.h"
#include "qtl.h"
#include "qtextcodec.h"

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

static bool match( const char* found, const char* target )
{
    return found[0] == '\0' || qstrcmp(found, target) == 0;
}

static int cmp_uint32_little( const void* target, const void* candidate )
{
    const uchar* t = (const uchar*) target;
    const uchar* c = (const uchar*) candidate;
    return t[3] != c[0] ? (int) t[3] - (int) c[0]
	   : t[2] != c[1] ? (int) t[2] - (int) c[1]
	   : t[1] != c[2] ? (int) t[1] - (int) c[2]
	   : (int) t[0] - (int) c[3];
}

static int cmp_uint32_big( const void* target, const void* candidate )
{
    const uchar* t = (const uchar*) target;
    const uchar* c = (const uchar*) candidate;
    return t[0] != c[0] ? (int) t[0] - (int) c[0]
	   : t[1] != c[1] ? (int) t[1] - (int) c[1]
	   : t[2] != c[2] ? (int) t[2] - (int) c[2]
	   : (int) t[3] - (int) c[3];
}

static int systemWordSize = 0;
static bool systemBigEndian;

static uint hash( const char * name )
{
    const uchar *k;
    uint h = 0;
    uint g;

    if ( name ) {
	k = (const uchar*)name;
	while( *k ) {
	    h = (h << 4) + *k++;
	    if ( (g = (h & 0xf0000000)) != 0 )
		h ^= g >> 24;
	    h &= ~g;
	}
    }

    if ( !h )
	h = 1;

    return h;
}


class QTranslatorPrivate {
public:
    struct Offset {
	Offset() { h=0; o=0; }
	Offset( const QTranslatorMessage& m, int offset )
	{ h = m.hash(); o = offset; } // ### qChecksum

	bool operator<( const Offset&k ) const { return ( h != k.h )
							     ? h < k.h
							     : o < k.o; }

	uint h;
	uint o;
    };

    enum { Codec = 0x1d, Contexts = 0x2f, Hashes = 0x42, Messages = 0x69 } Tag;

    QTranslatorPrivate() :
	unmapPointer( 0 ), unmapLength( 0 ),
	messageArray( 0 ), offsetArray( 0 ), contextArray( 0 ),
	mib( (Q_UINT32) 0 ),
	messages( 0 ) {}
    // note: QTranslator must finalize this before deallocating it.

    // for mmap'ed files, this is what needs to be unmapped.
    char * unmapPointer;
    unsigned int unmapLength;


    // for squeezed but non-file data, this is what needs to be deleted
    QByteArray * messageArray;
    QByteArray * offsetArray;
    QByteArray * contextArray;
    Q_UINT32 mib;

    QMap<QTranslatorMessage, void *> * messages;

};


/*! \class QTranslator qtranslator.h

  \brief The QTranslator class provides internationalization support for text
  output.

  \ingroup environment

  An object of this class contains a set of QTranslatorMessage
  objects, each of which specifies a translation from a source
  language to a target language.  QTranslator provides functions to
  look up such translations, add new ones, remove, load and save them,
  etc.

  The most common use of QTranslator is expected to be loading a
  translator file, installing it using
  QApplication::installTranslator(), and using it via QObject::tr(),
  like this:

  \code
  int main( int argc, char ** argv )
  {
    QApplication app( argc, argv );

    QTranslator translator( 0 );
    translator.load( "trfile.qm", "." );
    app.installTranslator( &translator );

    MyWidget m;
    app.setMainWidget( &m );
    m.show();

    return app.exec();
  }
  \endcode

  Most applications will never need to do anything else with this
  class.  However, applications that work on translator files need the
  other functions in this class.

  It is possible to do lookup using find() (as tr() and
  QApplication::translate() do) and contains(), insert a new
  translation message using insert() and removing them using remove().

  Since end-user programs and translation tools have rather different
  requirements, QTranslator can use stripped translator files in a way
  that uses a minimum of memory and provides very little functionality
  other than find().

  Thus, load() may not load enough information to make anything more
  than find() work.  save() has an argument indicating whether to save
  just this minimum of information, or everything.

  Everything means that for each translation item the following information
  is kept: <ul>
  <li> The \e translated \e text - the return value from tr().
  <li> The input key: <ul>
    <li> The \e source \e text - the argument to tr(), normally.
    <li> The \e context - usually the class name for the tr() caller.
    <li> The \e comment - a comment which helps disambiguate different uses
    of the same text in the same context.
  </ul>
  <li> The status of this item - "fully translated", "needs more work" or "not
  currently in use".
  </ul>

  The minimum is, for each item, just the information that is
  necessary for find() to return the right text.  This may include the
  source, context and comment, but usually is just a hash value and
  the translated text.

  For example, the "Cancel" in a dialog might have "Anuluj" when the
  program runs in Polish, in which case the source text would be
  "Cancel", the context would (normally) be the dialog's class name,
  there would normally be no comment, and the translated text would be
  "Anuluj".

  But it's not always so simple: The Spanish version of a printer
  dialog with settings for two-sided printing and binding would
  probably require both "Activado" and "Activada" as translations for
  "Enabled".  In this case, the source text would be "Enabled" in both
  cases and the context would be the dialog's class name, but the two
  items would have disambiguating comments such as "two-sided
  printing" for one and "binding" for the other.  The comment enables
  the translator to choose the appropriate gender for the Spanish
  version, and Qt to distinguish between translations.

  Note that when QTranslator loads a stripped file, most functions do
  not work.  The functions that do work with stripped files are
  explicitly documented as such.

  \sa QTranslatorMessage QApplication::installTranslator()
  QApplication::removeTranslator() QObject::tr() QApplication::translate()
*/

/*! \enum QTranslator::SaveMode
  This enum type defines how QTranslator can write translation files.
  There are two modes:

  <ul>
  <li> \c Everything - files are saved with all contents
  <li> \c Stripped - files are saved with just what's needed for end-users
  </ul>

  Note that when QTranslator loads a stripped file, most functions do
  not work.  The functions that do work with stripped files are
  explicitly documented as such.
*/

/*!  Constructs an empty message file, not connected to any file.
*/

QTranslator::QTranslator( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = new QTranslatorPrivate;
}


/*!  Destructs the object and frees any allocated resources.
*/

QTranslator::~QTranslator()
{
    if ( qApp )
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

  This function works with stripped translator files.

  \sa save()
*/

bool QTranslator::load( const QString & filename, const QString & directory,
			const QString & search_delimiters,
			const QString & suffix )
{
    clear();

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

    while( TRUE ) {
	bool done = FALSE;
	QFileInfo fi;

	realname = prefix + fname;
	fi.setFile(realname);
	if ( fi.isReadable() )
	    break;

	realname += suffix.isNull() ? QString::fromLatin1(".qm") : suffix;
	fi.setFile(realname);
	if ( fi.isReadable() )
	    break;

	int i = 0;
	while( !done && i<(int)delims.length() ) {
	    int dlm;
	    if ( (dlm=fname.find(delims[i])) >= 0 ) {
		// found a truncation
		fname = fname.left(dlm);
		done = TRUE;
	    }
	    i++;
	}
	if ( !done )
	    return FALSE; // No truncations - fail
    }

    // realname is now the fully qualified name of a readable file.


#if defined(QT_USE_MMAP)
    // unix (if mmap supported)

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
    if ( !f.exists() )
	return FALSE;
    d->unmapLength = f.size();
    d->unmapPointer = new char[d->unmapLength]; // ### really not
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

    // now that we've read it and all, check that it has the right
    // magic number, and forget all about it if it doesn't.
    if ( memcmp( (const void *)(d->unmapPointer), magic, magic_length ) ) {
	clear();
	return FALSE;
    }

    // prepare to read.
    QByteArray tmpArray;
    tmpArray.setRawData( d->unmapPointer, d->unmapLength );
    QDataStream s( tmpArray, IO_ReadOnly );
    s.device()->at( magic_length );

    // read.
    Q_UINT8 tag = 0;
    Q_UINT32 length = 0;
    s >> tag >> length;
    while( tag && length ) {
	if ( tag == QTranslatorPrivate::Codec ) {
	    s >> d->mib;
	} else if ( tag == QTranslatorPrivate::Contexts && !d->contextArray ) {
	    d->contextArray = new QByteArray;
	    d->contextArray->setRawData( tmpArray.data()+s.device()->at(),
					 length );
	} else if ( tag == QTranslatorPrivate::Hashes && !d->offsetArray ) {
	    d->offsetArray = new QByteArray;
	    d->offsetArray->setRawData( tmpArray.data()+s.device()->at(),
					length );
	} else if ( tag == QTranslatorPrivate::Messages && !d->messageArray ) {
	    d->messageArray = new QByteArray;
	    d->messageArray->setRawData( tmpArray.data()+s.device()->at(),
					 length );
	}
	s.device()->at( s.device()->at() + length );
	tag = 0;
	length = 0;
	if ( !s.atEnd() )
	    s >> tag >> length;
    }

    tmpArray.resetRawData( d->unmapPointer, d->unmapLength );
    return TRUE;
}


/*!  Saves this message file to \a filename, overwriting the previous
  contents of \a filename.  If \a mode is \c Everything (this is the
  default), all the information is preserved.  If \a mode is \c Stripped,
  all information that is not necessary for find() is stripped away.

  \sa load()
*/

bool QTranslator::save( const QString & filename, SaveMode mode )
{
    QFile f( filename );
    if ( f.open( IO_WriteOnly ) ) {
	squeeze( mode );

	QDataStream s( &f );
	s.writeRawBytes( (const char *)magic, magic_length );
	Q_UINT8 tag;

	if ( d->mib != 0 ) {
	    tag = (Q_UINT8) QTranslatorPrivate::Codec;
	    s << tag << (Q_UINT32) 4 << d->mib;
	}
	if ( d->offsetArray != 0 ) {
	    tag = (Q_UINT8) QTranslatorPrivate::Hashes;
	    Q_UINT32 oas = (Q_UINT32) d->offsetArray->size();
	    s << tag << oas;
	    s.writeRawBytes( d->offsetArray->data(), oas );
	}
	if ( d->messageArray != 0 ) {
	    tag = (Q_UINT8) QTranslatorPrivate::Messages;
	    Q_UINT32 mas = (Q_UINT32) d->messageArray->size();
	    s << tag << mas;
	    s.writeRawBytes( d->messageArray->data(), mas );
	}
	if ( d->contextArray != 0 ) {
	    tag = (Q_UINT8) QTranslatorPrivate::Contexts;
	    Q_UINT32 cas = (Q_UINT32) d->contextArray->size();
	    s << tag << cas;
	    s.writeRawBytes( d->contextArray->data(), cas );
	}
	return TRUE;
    }
    return FALSE;
}


/*!  Empties this translator of all contents.

  This function works with stripped translator files.
*/

void QTranslator::clear()
{
    if ( d->unmapPointer && d->unmapLength ) {
#if defined(QT_USE_MMAP)
	munmap( d->unmapPointer, d->unmapLength );
#else
	delete [] d->unmapPointer;
#endif
	d->unmapPointer = 0;
	d->unmapLength = 0;
	if ( d->messageArray )
	    d->messageArray->resetRawData( d->messageArray->data(),
					   d->messageArray->size() );
	if ( d->offsetArray )
	    d->offsetArray->resetRawData( d->offsetArray->data(),
					  d->offsetArray->size() );
	if ( d->contextArray )
	    d->contextArray->resetRawData( d->contextArray->data(),
					   d->contextArray->size() );
    }
    delete d->messageArray;
    d->messageArray = 0;
    delete d->offsetArray;
    d->offsetArray = 0;
    delete d->contextArray;
    d->contextArray = 0;
    d->mib = (Q_UINT32) 0;
    delete d->messages;
    d->messages = 0;
}


/*! Converts this message file to the compact format used to store
  message files on disk.

  You should never need to call this directly; save() and other functions call
  it as necessary.

  \sa save() unsqueeze()
*/

void QTranslator::squeeze( SaveMode mode )
{
    if ( !d->messages ) {
	if ( mode == Stripped )
	    unsqueeze();
	else
	    return;
    }

    QMap<QTranslatorMessage, void *> * messages = d->messages;

    d->messages = 0;
    clear();

    d->messageArray = new QByteArray;
    d->offsetArray = new QByteArray;

    QMap<QTranslatorPrivate::Offset, void *> offsets;

    QDataStream ms( *d->messageArray, IO_WriteOnly );
    QMap<QTranslatorMessage, void *>::Iterator it = messages->begin(), next;
    int cpPrev = 0, cpNext = 0;
    for ( it = messages->begin(); it != messages->end(); ++it ) {
	/*
	  These really should be stripped in a first pass.  By doing it here,
	  the result is correct but sub-optimal.
	 */
	if ( mode == Stripped &&
	     it.key().type() != QTranslatorMessage::Finished )
	    continue;

	cpPrev = cpNext;
	next = it;
	++next;
	if ( next == messages->end() )
	    cpNext = 0;
	else
	    cpNext = (int) it.key().commonPrefix( next.key() );
	offsets.replace( QTranslatorPrivate::Offset(it.key(),
			 ms.device()->at()), (void*)0 );
	it.key().write( ms, mode == Stripped,
			(QTranslatorMessage::Prefix) QMAX(cpPrev, cpNext + 1) );
    }

    d->offsetArray->resize( 0 );
    QMap<QTranslatorPrivate::Offset, void *>::Iterator offset;
    offset = offsets.begin();
    QDataStream ds( *d->offsetArray, IO_WriteOnly );
    while( offset != offsets.end() ) {
	QTranslatorPrivate::Offset k = offset.key();
	++offset;
	ds << (Q_UINT32)k.h << (Q_UINT32)k.o;
    }

    if ( mode == Stripped ) {
	QAsciiDict<int> contextSet( 1511 );
	int strindberg;

	for ( it = messages->begin(); it != messages->end(); ++it )
	    contextSet.replace( it.key().context(), &strindberg );

	Q_UINT16 hTableSize;
	if ( contextSet.count() < 200 )
	    hTableSize = ( contextSet.count() < 60 ) ? 151 : 503;
	else if ( contextSet.count() < 2500 )
	    hTableSize = ( contextSet.count() < 750 ) ? 1511 : 5003;
	else
	    hTableSize = 15013;

	QIntDict<char> hDict( hTableSize );
	QAsciiDictIterator<int> c = contextSet;
	while ( c.current() != 0 ) {
	    hDict.insert( (long) (::hash(c.currentKey()) % hTableSize),
			  c.currentKey() );
	    ++c;
	}

	/*
	  The contexts found in this translator are stored in a hash table to
	  provide fast look-up.  The context array has the following format:

	      Q_UINT16 hTableSize;
	      Q_UINT16 hTable[hTableSize];
	      Q_UINT8  contextPool[...];

	  The context pool stores the contexts as Pascal strings (au da!):

	      Q_UINT8  len;
	      Q_UINT8  data[len];

	  Let's consider the look-up of context "FunnyDialog".  A hash value
	  between 0 and hTableSize - 1 is computed, say h.  If hTable[h] is 0,
	  "FunnyDialog" is not covered by this translator.  Else, we check in
	  the contextPool at offset 2 * hTable[h] to see if "FunnyDialog" is one
	  of the contexts stored there, until we find it or we meet the empty
	  string.
	*/
	d->contextArray = new QByteArray;
	d->contextArray->resize( 2 + (hTableSize << 1) );
	QDataStream t( *d->contextArray, IO_WriteOnly );
	Q_UINT16 *hTable = new Q_UINT16[hTableSize];
	memset( hTable, 0, hTableSize * sizeof(Q_UINT16) );

	t << hTableSize;
	t.device()->at( 2 + (hTableSize << 1) );
	t << (Q_UINT16) 0; // the entry at offset 0 cannot be used (c.f. Pascal)
	uint upto = 2;

	for ( int i = 0; i < hTableSize; i++ ) {
	    const char *con = hDict.find( i );
	    if ( con == 0 ) {
		hTable[i] = 0;
	    } else {
		hTable[i] = (Q_UINT16) ( upto >> 1 );
		do {
		    uint len = (uint) qstrlen( con );
		    len = QMIN( len, 255 );
		    t << (Q_UINT8) len;
		    t.writeRawBytes( con, len );
		    upto += 1 + len;
		    hDict.remove( i );
		} while ( (con = hDict.find(i)) != 0 );
		do {
		    t << (Q_UINT8) 0; // empty string (at least one)
		    upto++;
		} while ( (upto & 0x1) != 0 ); // offsets have to be even
	    }
	}
	t.device()->at( 2 );
	for ( int j = 0; j < hTableSize; j++ )
	    t << hTable[j];
	delete hTable;

	if ( upto > 131072 ) {
	    qWarning( "QTranslator::squeeze: Too many contexts" );
	    delete d->contextArray;
	    d->contextArray = 0;
	}
    }
    delete messages;
}


/*! \overload

  This function calls squeeze( Everything ).  It is provided for compatibility;
  in Qt 3.0 it will be replaced by a default argument.
*/

void QTranslator::squeeze()
{
    squeeze( Everything );
}


/*!  Converts this message file into an easily modifiable data structure, less
  compact than the format used in the files.

  You should never need to call this function; it is called by insert() and
  friends as necessary.

  \sa squeeze()
*/

void QTranslator::unsqueeze()
{
    if ( d->messages )
	return;

    d->messages = new QMap<QTranslatorMessage, void *>;
    if ( !d->messageArray )
	return;

    QDataStream s( *d->messageArray, IO_ReadOnly );
    while( TRUE ) {
	QTranslatorMessage m( s );
	if ( m.hash() == 0 )
	    break;
	d->messages->replace( m, (void *) 0 );
    }
}


/*!  Returns TRUE if this message file contains a message with the key
  ( \a context, \a sourceText, \a comment ), and FALSE if it does not.

  This function works with stripped translator files.

  (This is is a one-liner that calls find().)
*/

bool QTranslator::contains( const char* context, const char* sourceText,
			    const char* comment ) const
{
    return find( context, sourceText, comment ) != QString::null;
}


/*! \overload
  \obsolete

  This version of the function assumes that the comment is "".
*/

bool QTranslator::contains( const char* context, const char* sourceText ) const
{
    return contains( context, sourceText, "" );
}

/*!  Inserts \a message into this message file.

  This function does \e not work with stripped translator files.  It
  may seem to, but that is not dependable.

  \sa remove()
*/

void QTranslator::insert( const QTranslatorMessage& message )
{
    unsqueeze();
    d->messages->replace( message, (void *) 0 );
}


/*! \overload
  \obsolete
*/

void QTranslator::insert( const char * context, const char * sourceText,
			  const QString & translation )
{
    insert( QTranslatorMessage(context, sourceText, "", translation,
			       QTranslatorMessage::Finished) );
}


/*!  Removes \a message from this translator.

  This function works with stripped translator files.

  \sa insert()
*/

void QTranslator::remove( const QTranslatorMessage& message )
{
    unsqueeze();
    d->messages->remove( message );
}


/*! \overload
  \obsolete

  Removes the translation associated to the key ( \a context, \a sourceText,
  "" ) from this translator.
*/

void QTranslator::remove( const char *context, const char *sourceText )
{
    remove( QTranslatorMessage(context, sourceText, "") );
}


/*!  Returns the translation for the key ( \a context, \a sourceText,
  \a comment ), or QString::null if there is none in this translator.

  This function works with stripped translator files.

  \sa findMessage
*/

QString QTranslator::find( const char* context, const char* sourceText,
			   const char* comment ) const
{
    if ( comment == 0 || comment[0] == '\0' )
	return find( context, sourceText );
    else
	return findMessage( context, sourceText, comment ).translation();
}


/*! \overload
  \obsolete

  Returns the translation for the key ( \a context, \a sourceText, "" ), or
  QString::null if there is none in this translator.
*/

QString QTranslator::find( const char* context, const char* sourceText ) const
{
    return findMessage( context, sourceText, "" ).translation();
}


/*!  Returns the QTranslatorMessage for the key
  ( \a context, \a sourceText, \a comment ).
*/

QTranslatorMessage QTranslator::findMessage( const char* context,
					     const char* sourceText,
					     const char* comment ) const
{
    if ( d->messages ) {
	QMap<QTranslatorMessage, void *>::ConstIterator it
	    = d->messages->find( QTranslatorMessage(context, sourceText,
				 comment) );
	if ( it == d->messages->end() )
	    return QTranslatorMessage();
	return it.key();
    }

    if ( !d->offsetArray )
	return QTranslatorMessage();

    if ( d->contextArray ) {
	Q_UINT16 hTableSize;
	QDataStream t( *d->contextArray, IO_ReadOnly );
	t >> hTableSize;
	uint g = ::hash( context ) % hTableSize;
	t.device()->at( 2 + (g << 1) );
	Q_UINT16 off;
	t >> off;
	if ( off == 0 )
	    return QTranslatorMessage();
	t.device()->at( 2 + (hTableSize << 1) + (off << 1) );

	Q_UINT8 len;
	char con[256];
	while(TRUE) {
	    t >> len;
	    if ( len == 0 )
		return QTranslatorMessage();
	    t.readRawBytes( con, len );
	    con[len] = '\0';
	    if ( qstrcmp(con, context) == 0 )
		break;
	}
    }

    uint h = ::hash( QCString(sourceText) + comment );

    Q_UINT32 rh;
    Q_UINT32 ro;

    size_t numItems = d->offsetArray->size() / ( 2 * sizeof(Q_UINT32) );
    if ( !numItems )
	return QTranslatorMessage();

    if ( systemWordSize == 0 )
	qSysInfo( &systemWordSize, &systemBigEndian );
    char *r = (char *) bsearch( &h, d->offsetArray->data(), numItems,
				2 * sizeof(Q_UINT32),
				systemBigEndian ? cmp_uint32_big
				: cmp_uint32_little );
    if ( r == 0 )
	return QTranslatorMessage();

    while( r != d->offsetArray->data() && cmp_uint32_big( r - 8, r ) == 0 )
	r -= 8;

    QDataStream s( *d->offsetArray, IO_ReadOnly );
    s.device()->at( r - d->offsetArray->data() );

    s >> rh >> ro;

    QDataStream ms( *d->messageArray, IO_ReadOnly );
    while( rh == h ) {
	ms.device()->at( ro );
	QTranslatorMessage m( ms );
	if ( match(m.context(), context)
		&& match(m.sourceText(), sourceText)
		&& match(m.comment(), comment) )
	    return m;
	if ( s.atEnd() )
	    return QTranslatorMessage();
	s >> rh >> ro;
    }

    return QTranslatorMessage();
}


/*!  Returns a list of the messages in the translator.  This function is
  somewhat slow; since it's seldom called it's optimized for simplicity and
  small size, not speed.
*/

QValueList<QTranslatorMessage> QTranslator::messages() const
{
    ((QTranslator *) this)->unsqueeze();
    QValueList<QTranslatorMessage> result;
    QMap<QTranslatorMessage, void *>::ConstIterator it;
    for ( it = d->messages->begin(); it != d->messages->end(); ++it )
	result.append( it.key() );
    return result;
}


/*!  Decodes the 8-bit string \a str using the default codec for the
  machine on which the translator file was created, if possible.

  \sa QTranslatorMessage::sourceText() QTranslatorMessage::comment()
*/

QString QTranslator::toUnicode( const char * str ) const
{
    QTextCodec * codec;
    if ( d->mib && (codec = QTextCodec::codecForMib((int) d->mib)) )
	return codec->toUnicode( str );
    return QString::fromLatin1( str );
}


/* NOT DOCUMENTED \class QTranslatorMessage qtranslator.h

  \brief The QTranslatorMessage class contains a translator message and its properties.

  \ingroup environment

  This class is of no interest to most applications, just for
  translation tools.

  For a QTranslator object, a lookup \e key is a triple ( \e context,
  \e source \e text, \e comment ) that uniquely identifies a message.  An
  \e extended \e key is a quadruple ( \e hash, \e context, \e source \e text,
  \e comment ), where \a hash is computed from the source text and the comment.
  Unless you plan to read and write messages yourself, you need not worry about
  the hash value.

  An object of this class also contains the translation, in Unicode, of the
  source text, with a type.

  \sa QTranslator
*/

/*! \enum QTranslatorMessage::Type

  This enum defines the types of messages in a translator object.

  <ul>
  <li> \c Unfinished - the translation is absent or unusable
  <li> \c Finished - the translation is present and useful
  <li> \c Obsolete - the translation is present but useless for the user (it
       may still be useful for the human translator, or may be made useful
       again)
  </ul>

  \sa setType() type()
*/

/*!  Constructs an \c Unfinished translator message with extended key
  ( 0, "", "", "" ) and QString::null as translation.
*/

QTranslatorMessage::QTranslatorMessage()
    : h( 0 ), cx( "" ), st( "" ), cm( "" ), ty( Unfinished )
{
}


/*!  Constructs an translator message of a certain \a type (\c Unfinished by
  default) with extended key ( \e h, \a context, \a sourceText, \a comment ),
  where \e h is computed from \a sourceText and \a comment, and possibly with a
  \a translation.
*/

QTranslatorMessage::QTranslatorMessage( const char * context,
					const char * sourceText,
					const char * comment,
					const QString& translation,
					Type type )
    : cx( context ), st( sourceText ), cm( comment ), tn( translation ),
      ty( type )
{
    h = ::hash( st + cm );
}


/*!  Constructs a translator message read from a \a stream.  The resulting
  message may have any combination of content.

  \sa save()
 */

QTranslatorMessage::QTranslatorMessage( QDataStream & stream )
    : cx( "" ), st( "" ), cm( "" )
{
    QString str16;
    ty = Unfinished;
    char tag;

    while( TRUE ) {
	tag = 0;
	if ( !stream.atEnd() )
	    stream.readRawBytes( &tag, 1 );
	switch( (Tag)tag ) {
	case Tag_End:
	    h = ::hash( st + cm );
	    return;
	case Tag_SourceText16:
	    stream >> str16;
	    st = str16.latin1();
	    break;
	case Tag_Translation:
	    stream >> tn;
	    break;
	case Tag_Context16:
	    stream >> str16;
	    cx = str16.latin1();
	    break;
	case Tag_Hash:
	    stream >> h;
	    break;
	case Tag_SourceText:
	    stream >> st;
	    break;
	case Tag_Context:
	    stream >> cx;
	    break;
	case Tag_Comment:
	    stream >> cm;
	    break;
	case Tag_Type:
	    {
		Q_UINT8 tmp;
		stream >> tmp;
		ty = (QTranslatorMessage::Type)tmp;
		if ( ty != Unfinished && ty != Finished && ty != Obsolete )
		    ty = Unfinished;
	    }
	    break;
	default:
	    st = cx = cm = "";
	    tn = QString::null;
	    h = 0;
	    ty = Unfinished;
	    return;
	}
    }
}


/*! \fn uint QTranslatorMessage::hash() const

  Returns the hash value used internally to represent the lookup key.  This
  value is zero only if this translator message was constructed from a stream
  containing invalid data.

  The hashing function is unspecified, but it will remain unchanged in future
  versions of Qt.
*/

/*! \fn const char *QTranslatorMessage::context() const

  Returns the context for this message (e.g., "FunnyDialog").
*/

/*! \fn const char *QTranslatorMessage::sourceText() const

  Returns the source text of this message (e.g., "&Save").  The source text is
  used as is for lookup; it should be converted using
  QTranslator::toUnicode() before it shows up properly.
*/

/*! \fn const char *QTranslatorMessage::comment() const

  Returns the comment for this message (e.g., "File > Save").  The comment is
  used as is for lookup; it should be converted using
  QTranslator::toUnicode() before it shows up properly.
*/

/*! \fn void QTranslatorMessage::setTranslation( const QString & translation )

  Sets the translation of the source text.

  \sa translation()
*/

/*! \fn QString QTranslatorMessage::translation() const

  Returns the translation of the source text (e.g., "&Sauvegarder").

  \sa setTranslation()
*/

/*! \fn void QTranslatorMessage::setType( Type type )

  Sets the type of this message.

  \sa type()
*/

/*! \fn Type QTranslatorMessage::type() const

  Returns the type of this message.

  The default value is \c Unfinished.

  \sa setType()
*/

/*! \enum QTranslatorMessage::Prefix

  Let ( \e h, \e c, \e s, \e m ) be the extended key.  The possible prefixes are

  <ul>
  <li> \c NoPrefix - no prefix
  <li> \c Hash - only ( \e h )
  <li> \c HashContext - only ( \e h, \e c )
  <li> \c HashContextSourceText - only ( \e h, \e c, \e s )
  <li> \c HashContextSourceTextComment - the whole extended key, ( \e h, \e c,
       \e s, \e m )
  </ul>

  \sa write() commonPrefix()
*/

/*!  Writes this translator message to a \a stream.  If \a strip is FALSE (the
  default), all the information in the message is written.  If \a strip is TRUE,
  only the part of the extended key specified by \a prefix is written with the
  translation (\c HashContextSourceTextComment by default).

  \sa commonPrefix()
*/

void QTranslatorMessage::write( QDataStream & stream, bool strip,
				Prefix prefix ) const
{
    char tag;

    tag = (char)Tag_Translation;
    stream.writeRawBytes( &tag, 1 );
    stream << tn;

    bool mustWriteHash = TRUE;
    if ( !strip )
	prefix = HashContextSourceTextComment;

    switch ( prefix ) {
    case HashContextSourceTextComment:
	tag = (char)Tag_Comment;
	stream.writeRawBytes( &tag, 1 );
	stream << cm;
	// fall through
    case HashContextSourceText:
	tag = (char)Tag_SourceText;
	stream.writeRawBytes( &tag, 1 );
	stream << st;
	// fall through
    case HashContext:
	tag = (char)Tag_Context;
	stream.writeRawBytes( &tag, 1 );
	stream << cx;
	// fall through
    default:
	if ( mustWriteHash ) {
	    tag = (char)Tag_Hash;
	    stream.writeRawBytes( &tag, 1 );
	    stream << h;
	}
    }

    if ( !strip ) {
	tag = (char)Tag_Type;
	stream.writeRawBytes( &tag, 1 );
	stream << (Q_UINT8)ty;
    }

    tag = (char)Tag_End;
    stream.writeRawBytes( &tag, 1 );
}


/*!  Returns the widest lookup prefix that is common to this translator message
  and message \a m.

  For example, if the extended key is for this message is ( 42, "FunnyDialog",
  "Yes", "Funny?" ) and that for \a m is ( 42, "FunnyDialog", "No", "Funny?" ),
  returns \c HashContext.

  \sa write()
*/

QTranslatorMessage::Prefix QTranslatorMessage::commonPrefix(
	const QTranslatorMessage& m ) const
{
    if ( h != m.h )
	return NoPrefix;
    if ( cx != m.cx )
	return Hash;
    if ( st != m.st )
	return HashContext;
    if ( cm != m.cm )
	return HashContextSourceText;
    return HashContextSourceTextComment;
}


/*!  Returns TRUE if the extended key of this object is equal to that of \a m,
  otherwise FALSE.
*/

bool QTranslatorMessage::operator==( const QTranslatorMessage& m ) const
{
    return h == m.h && cx == m.cx && st == m.st && cm == m.cm;
}


/*! \fn bool QTranslatorMessage::operator!=( const QTranslatorMessage& m ) const

  Returns TRUE if the extended key of this object is different from that of
  \a m, otherwise FALSE.
*/


/*!  Returns TRUE if the extended key of this object is lexicographically before
  than that of \a m, otherwise FALSE.
*/

bool QTranslatorMessage::operator<( const QTranslatorMessage& m ) const
{
    return h != m.h ? h < m.h
	   : ( cx != m.cx ? cx < m.cx
	     : (st != m.st ? st < m.st : cm < m.cm) );
}


/*! \fn bool QTranslatorMessage::operator<=( const QTranslatorMessage& m ) const

  Returns TRUE if the extended key of this object is lexicographically before
  that of \a m or if they are equal, otherwise FALSE.
*/

/*! \fn bool QTranslatorMessage::operator>( const QTranslatorMessage& m ) const

  Returns TRUE if the extended key of this object is lexicographically after
  that of \a m, otherwise FALSE.
*/

#endif // QT_NO_TRANSLATION

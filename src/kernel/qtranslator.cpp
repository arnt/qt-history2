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
#include "qfileinfo.h"
#include "qtextcodec.h"
#include "qlist.h"

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


static uint hash( const char * name )
{
    const uchar *k;
    uint h = 0;
    uint g;

    if ( name ) {
	k = (const uchar*)name;
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


class QTranslatorPrivate {
public:
    // a single message, with any combination of contents
    class Message {
    public:
	Message(): t( QTranslator::Unfinished ), h(0), collision( FALSE ) {}
	Message( QDataStream & );

	void setInput( const char * ni )
	{
	    QTextCodec* codec = qApp ? qApp->defaultCodec() : 0;
	    if ( codec )
		i = codec->toUnicode( ni );
	    else
		i = QString::fromLatin1( ni );
	}
	QString input() const { return i; }

	void setOutput( const QString & newOutput ) { o = newOutput; }
	QString output() const { return o; }

	void setScope( const QString & newScope ) { s = newScope; }
	QString scope() const { return s; }

	uint hash()
	{
	    if ( !h ) {
		QTextCodec* codec = qApp ? qApp->defaultCodec() : 0;
		if ( codec )
		    h = ::hash( codec->fromUnicode(i) );
		else
		    h = ::hash( i.latin1() );
	    }
	    return h;
	}

	void write( QTranslator::SaveMode, QDataStream & );

	bool sane() { return i.length() > 0 && o.length() > 0 && hash() != 0; }

	void setComment( const QString & newComment) { c = newComment; }
	QString comment() const { return c; }

	void setTranslationType( QTranslator::TranslationType nt ) { t = nt; }
	QTranslator::TranslationType translationType() { return t; }

	void setHashCollision( bool asdf ) { collision = asdf; }
	bool hashCollision() const { return collision; }

    private:
	QString i;
	QString o;
	QString s;
	QString c;
	QTranslator::TranslationType t;
	uint h;
	bool collision;

	enum Tag { End = 1, Input, Output, Scope, Hash, Comment, Type };

    };

    struct Offset {
	Offset() { h=0; o=0; }
	Offset( Message * m, int offset )
	{ h = m->hash(); o = offset; } // ### qChecksum

	bool operator<(const Offset&k) const { return ( h != k.h )
							     ? h < k.h
							     : o < k.o; }

	uint h;
	uint o;
    };

    enum { Hashes = 0x42, Messages = 0x69 } Tag;

    QTranslatorPrivate() :
	unmapPointer( 0 ), unmapLength( 0 ),
	messageArray( 0 ), offsetArray( 0 ),
	messages( 0 ) {}
    // note: QTranslator must finalize this before deallocating it.

    // for mmap'ed files, this is what needs to be unmapped.
    char * unmapPointer;
    unsigned int unmapLength;


    // for squeezed but non-file data, this is what needs to be deleted
    QByteArray * messageArray;
    QByteArray * offsetArray;

    QList<Message> * messages;

};


QTranslatorPrivate::Message::Message( QDataStream & stream )
{
    h = 0;
    t = QTranslator::Unfinished;
    collision = FALSE;
    char tag;

    while( TRUE ) {
	tag = 0;
	if ( !stream.atEnd() )
	    stream.readRawBytes( &tag, 1 );
	switch( (Tag)tag ) {
	case End:
	    return;
	    //break;
	case Input:
	    stream >> i;
	    break;
	case Output:
	    stream >> o;
	    break;
	case Scope:
	    stream >> s;
	    break;
	case Hash:
	    stream >> h;
	    break;
	case Comment:
	    stream >> c;
	    break;
	case Type:
	    { // a scope for tmp, to silence the compiler
		Q_UINT8 tmp;
		stream >> tmp;
		t = (QTranslator::TranslationType)tmp;
		if ( t != QTranslator::Unfinished &&
		     t != QTranslator::Finished &&
		     t != QTranslator::Obsolete )
		    t = QTranslator::Unfinished;
	    }
	    break;
	default:
	    i = o = s = c = QString::null;
	    h = 0;
	    t = QTranslator::Unfinished;
	    return;
	}
    }
}


void QTranslatorPrivate::Message::write( QTranslator::SaveMode m,
					 QDataStream & stream )
{
    char tag;

    tag = (char)Output;
    stream.writeRawBytes( &tag, 1 );
    stream << o;

    bool mustWriteHash = TRUE;

    if ( ( m == QTranslator::Everything || collision ) &&
	 s && i ) {
	tag = (char)Scope;
	stream.writeRawBytes( &tag, 1 );
	stream << s;
	tag = (char)Input;
	stream.writeRawBytes( &tag, 1 );
	stream << i;
    }

    if ( mustWriteHash ) {
	tag = (char)Hash;
	stream.writeRawBytes( &tag, 1 );
	stream << h;
    }

    if ( m == QTranslator::Everything ) {
	if ( c ) {
	    tag = (char)Comment;
	    stream.writeRawBytes( &tag, 1 );
	    stream << c;
	}
	tag = (char)Type;
	stream.writeRawBytes( &tag, 1 );
	stream << (Q_UINT8)t;
    }

    tag = (char)End;
    stream.writeRawBytes( &tag, 1 );
}




// NOT REVISED
/*! \class QTranslator qtranslator.h

  \brief The QTranslator class provides internationalization support for text output.

  \ingroup environment

  The class is conceptually very simple: An object of this class
  contains a set of translations from the reference language to a
  target language, and provides functions to add, look up and remove
  such translations as well as the ability to load and save the object
  to a file.

  The most common use of QTranslator is expected to be loading one
  from a file, installing it using QApplication::installTranslator(),
  and using it via QObject::tr(). Like this:
  \code
  main( int argc, char** argv )
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


  Slightly more advanced usage of QTranslator includes direct lookup
  using find(), adding new translations using insert() and removing
  existing ones using remove() or even clear(), and testing whether
  the QTranslator contains a translation using contains().

  The hash() function mentioned is a variant on the standard ELF hash,
  modified to work well with Unicode strings in UCS-2 format.  Its
  algorithm is not specified beyond the fact that it will remain
  unchanged in future versions of Qt.
*/

/*! \enum QTranslator::SaveMode
  This enum type defines how QTranslator can write translation files.
  There are currently only two modes: <ul>

  <li> \c Everything - files are saved with all contennts

  <li> \c Stripped - files are saved with just what's needed for
  end-user use of the files.

  </ul>
*/

/*!  Constructs an empty message file, not connected to any file.
*/

QTranslator::QTranslator( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = new QTranslatorPrivate;
}


/*! Destructs the object and frees any allocated resources.
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

    while( 1 ) {
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
    while ( tag && length ) {
	if ( tag == QTranslatorPrivate::Hashes && !d->offsetArray ) {
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
  contents of \a filename.

  \sa load()
*/

bool QTranslator::save( const QString & filename, SaveMode )
{
    QFile f( filename );
    if ( f.open( IO_WriteOnly ) ) {
	squeeze();

	QDataStream s( &f );
	s.writeRawBytes( (const char *)magic, magic_length );
	Q_UINT8 tag;

	tag = (Q_UINT8) QTranslatorPrivate::Hashes;
	Q_UINT32 oas = d->offsetArray ? (Q_UINT32) d->offsetArray->size() : 0;
	s << tag << oas;
	s.writeRawBytes( oas ? d->offsetArray->data() : 0, oas );

	tag = (Q_UINT8) QTranslatorPrivate::Messages;
	Q_UINT32 mas = d->messageArray ? (Q_UINT32) d->messageArray->size() : 0;
	s << tag << mas;
	s.writeRawBytes( mas ? d->messageArray->data() : 0, mas );

	return TRUE;
    }
    return FALSE;
}


/*!  Empties this translator of all contents.
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
    }
    delete d->messageArray;
    d->messageArray = 0;
    delete d->offsetArray;
    d->offsetArray = 0;
    delete d->messages;
    d->messages = 0;
}


/*!  Converts this message file to the compact format used to store
  message files on disk.  You should never need to call this directly;
  save() and other functions call it as necessary.
*/

void QTranslator::squeeze()
{
    if ( !d->messages )
	return;

    QList<QTranslatorPrivate::Message> * messages = d->messages;

    d->messages = 0;
    clear();

    d->messageArray = new QByteArray;
    d->offsetArray = new QByteArray;

    QMap<QTranslatorPrivate::Offset,void*> offsets;

    QDataStream ms( *d->messageArray, IO_WriteOnly );
    QListIterator<QTranslatorPrivate::Message> it( *messages );
    QTranslatorPrivate::Message * m;
    while( (m = it.current()) != 0 ) {
	++it;
	offsets.replace( QTranslatorPrivate::Offset(m,ms.device()->at()),
			 (void*)0 );
	m->write( Everything, ms );
    }

    d->offsetArray->resize( 0 );
    QMap<QTranslatorPrivate::Offset,void*>::Iterator offset;
    offset = offsets.begin();
    QDataStream ds( *d->offsetArray, IO_WriteOnly );
    while( offset != offsets.end() ) {
	QTranslatorPrivate::Offset k = offset.key();
	++offset;
	ds << (Q_UINT32)k.h << (Q_UINT32)k.o;
    }
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

    QList<QTranslatorPrivate::Message> * messages
	= new QList<QTranslatorPrivate::Message>;
    if ( !d->messageArray ) {
	d->messages = messages;
	return;
    }

    QDataStream s( *d->messageArray, IO_ReadOnly );
    QTranslatorPrivate::Message * m;
    while( TRUE ) {
	m = new QTranslatorPrivate::Message( s );
	if ( m->sane() ) {
	    messages->append( m );
	} else {
	    delete m;
	    clear();
	    d->messages = messages;
	    return;
	}
    }
}


/*!  Returns TRUE if this message file contains a message with hash
  value \a h, and FALSE if it does not.

  (This is is a one-liner that calls find().)
*/

bool QTranslator::contains( const char* scope, const char* key ) const
{
    return find( scope, key ) != QString::null;
}


/*! Inserts \a translation of \a message
  into this message file.
*/

void QTranslator::insert( const char* scope,
			  const char* message,
			  const QString& translation )
{
    unsqueeze();
    d->messages->first();
    QTranslatorPrivate::Message * m;
    while( (m=d->messages->current()) != 0 &&
	   (m->input() != message || m->scope() != scope) )
	d->messages->next();

    if ( m )
	d->messages->take();
    else
	m = new QTranslatorPrivate::Message();

    m->setInput( message );
    m->setScope( scope );
    m->setOutput( translation );
    d->messages->append( m );
}


/*!  Removes the string for \a h from this message file.  If there is
  no string for h, this function does nothing.

*/

void QTranslator::remove( const char *scope, const char *message )
{
    unsqueeze();
    d->messages->first();
    QTranslatorPrivate::Message * m;
    while( (m=d->messages->current()) != 0 &&
	   (m->input() != message || m->scope() != scope) )
	d->messages->next();

    if ( m )
	d->messages->remove();
}


/*!
  Returns the translation for (\a scope, \a key), or  QString::null in
  case there is none in this translator.
*/

QString QTranslator::find( const char* scope, const char* message ) const
{
    if ( d->messages ) {
	d->messages->first();
	QTranslatorPrivate::Message * m;
	while( (m=d->messages->current()) != 0 &&
	       (m->input() != message || m->scope() != scope) )
	    d->messages->next();
	if ( m )
	    return m->output();
	return QString::null;
    }

    uint h = ::hash( message );
    if ( !d->offsetArray )
	return QString::null;

    QDataStream s( *d->offsetArray, IO_ReadOnly );
    s.device()->at( 0 );
    Q_UINT32 rh = 0; // h is >= 1
    Q_UINT32 ro;
    while( rh < h && !s.atEnd() )
	s >> rh >> ro; // ### a long, slow loop.  needs fixing.

    if ( rh > h )
	return QString::null;

    QDataStream ms( *d->messageArray, IO_ReadOnly );
    while( rh == h ) {
	ms.device()->at( ro );
	QTranslatorPrivate::Message m( ms );
	if ( m.input().isNull() ||
	     ( m.input() == message && m.scope() == scope ) )
	    return m.output();
	if ( s.atEnd() )
	    return QString::null;
	s >> rh >> ro;
    }

    return QString::null;
}




/*  Returns a list of the inputs in the translator.  This function is
somewhat slow; since it's seldom called it's optimized for simplicity
and small size, not speed.
*/
QValueList<QTranslatorInputItem> QTranslator::inputKeys() const
{
    QValueList<QTranslatorInputItem> result;

    ((QTranslator *)this)->unsqueeze();
    d->messages->first();
    QTranslatorPrivate::Message * m;
    while( (m=d->messages->current()) != 0 ) {
	QTranslatorInputItem i;
	i.scope = m->scope();
	i.key = m->input();
	result.append( i );
	d->messages->next();
    }
    return result;
}


/*!  Sets the comment for \a message in \a scope to \a comment.  The
comment is a string which may help the translator provide a better
translation, but which isn't seen by any end-users.  Most applications
will never need to call this.
*/

void QTranslator::setComment( const char * scope, const char * message,
			      const char * comment )
{
    unsqueeze();
    QTranslatorPrivate::Message * m;
    while( (m=d->messages->current()) != 0 &&
	   (m->input() != message || m->scope() != scope) )
	d->messages->next();
    if ( m )
	m->setComment( comment );
}


/*!  Returns the comment for \a message in \a scope. */

QString QTranslator::comment( const char * scope, const char * message )
{
    unsqueeze();
    QTranslatorPrivate::Message * m;
    while( (m=d->messages->current()) != 0 &&
	   (m->input() != message || m->scope() != scope) )
	d->messages->next();
    return m ? m->comment() : QString::null;
}

#include "qrtformat.h"

#include <qcolor.h>
#include <qfont.h>
#include <qshared.h>
#include <qdict.h>

#include <limits.h>
#include <stdlib.h>

#include <assert.h>

// singleton
class QRTFormatCollection
{
public:
    static QRTFormatPrivate * defaultFormat();
    static QRTFormatPrivate * requestFormat( const QFont & f, const QColor &c );
    static void takeFormat( QRTFormatPrivate *fmt );

    static QRTFormatPrivate * _defaultFormat;
    static QDict<QRTFormatPrivate> *formats;
};

QRTFormatPrivate * QRTFormatCollection::_defaultFormat = 0;
QDict<QRTFormatPrivate> *QRTFormatCollection::formats = 0;

QRTFormatPrivate * QRTFormatCollection::defaultFormat()
{
    if ( !_defaultFormat ) {
	_defaultFormat = new QRTFormatPrivate;
	_defaultFormat->ref();
    }
    return _defaultFormat;
}

QRTFormatPrivate * QRTFormatCollection::requestFormat( const QFont & f, const QColor &c )
{
    if ( !formats )
	formats = new QDict<QRTFormatPrivate>(101);
    QString key = f.key() + c.name();
    QRTFormatPrivate *fmt = formats->find( key );
    if ( !fmt ) {
	fmt = new QRTFormatPrivate(f, c);
	formats->insert( key, fmt );
    }
    return fmt;
}

void QRTFormatCollection::takeFormat( QRTFormatPrivate *fmt )
{
    if ( !formats )
	return;
    QString key = fmt->font.key() + fmt->color.name();
    formats->take( key );
}


void QRTFormatPrivate::deleteMe()
{
    QRTFormatCollection::takeFormat( this );
    delete this;
}


void QRTFormat::statistics()
{
    qDebug("------- QRTFormat statistics -------");
    qDebug("default format: %d", QRTFormatCollection::defaultFormat()->refCount );
    if ( QRTFormatCollection::formats ) {
	QDictIterator<QRTFormatPrivate> it( *QRTFormatCollection::formats );
	for( ; it.current(); ++it )
	    qDebug("   entry %p: ref=%d", it.current(), it.current()->refCount );
    }
    qDebug("------- end statistics -------------");
}


QRTFormat::QRTFormat()
{
    d = QRTFormatCollection::defaultFormat();
    d->ref();
}


QRTFormat::QRTFormat( const QRTFormat &other )
{
    d = other.d;
    d->ref();
}

QRTFormat & QRTFormat::operator = ( const QRTFormat &other )
{
    other.d->ref();
    d->deref();
    d = other.d;
    return *this;
}

QRTFormat::~QRTFormat()
{
    d->deref();
}

QRTFormat::QRTFormat( const QFont &f, const QColor &c )
{
    d = QRTFormatCollection::requestFormat( f, c );
    d->ref();
}

void QRTFormat::setFont( const QFont &f )
{
    QRTFormatPrivate *newFormat = QRTFormatCollection::requestFormat( f, d->color );
    newFormat->ref();
    d->deref();
    d = newFormat;
}

const QFont &QRTFormat::font() const
{
    return d->font;
}

void QRTFormat::setColor( const QColor &c )
{
   QRTFormatPrivate *newFormat = QRTFormatCollection::requestFormat( d->font, c );
    newFormat->ref();
    d->deref();
    d = newFormat;
}

const QColor &QRTFormat::color() const
{
    return d->color;
}


QRTFormatArray::QRTFormatArray()
    : formatHints(0), size( 1 ), alloc( 1 )
{
}

QRTFormatArray::QRTFormatArray( const QRTFormat &f, int len )
    : size( 1 ), alloc( 1 )
{
    qDebug("sizeof=%d",  sizeof(FormatHint ) );
    formatHints = (FormatHint *) malloc( sizeof(FormatHint) );
    formatHints->format = f.d;
    formatHints->format->ref();
    formatHints->length = len;
}

QRTFormatArray::QRTFormatArray( const QRTFormatArray &other )
{
    duplicateFormatHints( other.formatHints, other.size );
}

QRTFormatArray &QRTFormatArray::operator =( const QRTFormatArray &other )
{
    duplicateFormatHints( other.formatHints, other.size );
    return *this;
}

QRTFormatArray::~QRTFormatArray()
{
    derefFormatHints();
    free( formatHints );
}

void QRTFormatArray::insert( int pos, int len, const QRTFormat &f )
{
    if ( !len )
	return;

    int i = 0;
    int p = 0;
    while ( i < size && p < pos ) {
	p += formatHints[i].length;
	i++;
    }
    if ( i )
	i--;

    assert( !( i == size && p < pos ) );

    if ( p == pos ) {
	// special case, we only need to insert one hint.
	insertFormatHints( i, 1, f.d );
    } else {
	// need to split the current one and insert the new one
	insertFormatHints( i+1, 2 );
	formatHints[i+1].format = f.d;
	formatHints[i+1].format->ref();
	formatHints[i+1].length = len;

	formatHints[i+2].format = formatHints[i].format;
	formatHints[i+2].format->ref();
	int l = formatHints[i].length;
	assert( p-pos < l );
	formatHints[i].length = l - (p - pos);
	formatHints[i+2].length = p - pos;
    }
    debug( "insert" );
}

void QRTFormatArray::insert( int pos, const QRTFormatArray &formats )
{
    if ( !formats.size )
	return;

    int i = 0;
    int p = 0;
    while ( i < size && p < pos ) {
	p += formatHints[i].length;
	i++;
    }

    assert( !( i == size && p < pos ) );

    if ( p == pos ) {
	// special case, we only need to insert one hint.
	int osize = size;
	resizeFormatHints( size + formats.size );
	qDebug("insert: newsize=%d, osize=%d, alloc=%d i=%d", size, osize, alloc, i );
	memmove( formatHints + i + formats.size, formatHints + i, sizeof(FormatHint)*(osize-i) );
	memcpy( formatHints + i, formats.formatHints, sizeof(FormatHint)*formats.size );
	int num = formats.size;
	FormatHint *data = formatHints + i;
	while ( num-- ) {
	    data->format->ref();
	    ++data;
	}
    } else {
	// need to split the current one and insert the new one
	i--;
	int osize = size;
	resizeFormatHints( size + formats.size + 1 );
	memmove( formatHints + i + formats.size + 1, formatHints + i, sizeof(FormatHint)*(osize-i) );
	memcpy( formatHints + i + 1, formats.formatHints, sizeof(FormatHint)*formats.size );
	int num = formats.size;
	FormatHint *data = formatHints + i + 1;
	while ( num-- ) {
	    data->format->ref();
	    ++data;
	}
	formatHints[i+formats.size+1].format = formatHints[i].format;
	formatHints[i+formats.size+1].format->ref();
	int l = formatHints[i].length;
	assert( p-pos < l );
	formatHints[i].length = l - (p - pos);
	formatHints[i+formats.size+1].length = p - pos;
    }
    debug( "insert2" );
}

void QRTFormatArray::stringInsert( int pos, int length )
{
    if ( !size ) {
	assert( pos = 0 );
	formatHints = (FormatHint *)malloc( sizeof(FormatHint) );
	formatHints->format = QRTFormatCollection::defaultFormat();
	formatHints->format->ref();
	formatHints->length = length;
	return;
    }

    int i = 0;
    int p = 0;
    while ( i < size && p < pos ) {
	p += formatHints[i].length;
	i++;
    }
    if ( i )
	i--;

    assert( i <= size );

    formatHints[i].length += length;
    debug( "stringInsert" );
}

void QRTFormatArray::remove( int pos, int len )
{
    if ( !size || !len )
	return;

    int i = 0;
    int p = 0;
    int eaten = 0;
    while ( i < size && p < pos ) {
	p += formatHints[i].length;
	i++;
    }
    if ( i )
	i--;

    assert( !( i == size ) );
    if ( len < 0 ) {
	// remove until end
	assert( p - pos < (int)formatHints[i].length );
	formatHints[i].length = p-pos;
	if ( formatHints[i].length )
	    i++;
	removeFormatHints( i, size-i-1 );
	debug( "remove1" );
	return;
    }

    if ( len < (int)formatHints[i].length ) {
	// simple, just have to shorten this element
	formatHints[i].length -= len;
	debug( "remove2" );
	return;
    }

    if ( p != pos ) {
	assert( p - pos < (int)formatHints[i].length );
	eaten = formatHints[i].length - (p - pos);
	formatHints[i].length = p-pos;
	i++;
    }

    int j = i;
    while ( eaten < len && j < size  ) {
	eaten += formatHints[j].length;
	j++;
    }
    if ( eaten > len ) {
	assert( eaten - len < (int)formatHints[j-1].length );
	formatHints[j-1].length -= eaten - len;
	j--;
    }
    assert( j > i );
    removeFormatHints( i,  j-i );
    debug( "remove" );
}

void QRTFormatArray::set( int pos,  int len, const QRTFormat &f )
{
    remove( pos, len );
    insert( pos, len, f );
    debug( "set" );
}




void QRTFormatArray::resizeFormatHints( int s )
{
    int newAlloc = (s+3) >> 2 << 2;
    if ( newAlloc != alloc ) {
	formatHints = (FormatHint *)realloc( formatHints, newAlloc*sizeof(FormatHint) );
// 	qDebug("reallocating to size %d",  newAlloc );
    }
    size = s;
    alloc = newAlloc;
}

void QRTFormatArray::duplicateFormatHints( FormatHint *hints, int num )
{
    if ( formatHints )
	derefFormatHints();
    resizeFormatHints( num );

    memcpy( formatHints, hints, sizeof( FormatHint )*num );
    FormatHint *data = formatHints;
    while ( num-- ) {
	data->format->ref();
	data++;
    }
}

void QRTFormatArray::derefFormatHints()
{
    int i = 0;
    while ( i < size ) {
	formatHints[i].format->deref();
	i++;
    }
    size = 0;
}

void QRTFormatArray::removeFormatHints( int from, int num )
{
    if ( from > size )
	return;
    if ( num < 0 )
	num = INT_MAX;
    FormatHint *data = formatHints + from;
    int i = 0;
    while ( i < num && formatHints->format ) {
	data->format->deref();
	data++;
	i++;
    }
    memmove( formatHints + from,  formatHints + from + i, sizeof(FormatHint)*(size-from-i) );
    size -= i;
}

void QRTFormatArray::insertFormatHints( int at, int num, QRTFormatPrivate *fp )
{
    int oldSize = size;
    resizeFormatHints( size + num );

    memmove( formatHints + at + num, formatHints + at, sizeof(FormatHint)*(oldSize-at) );
    int i = 0;
    while ( i < num ) {
	formatHints[at+i].format = fp;
	if ( fp )
	    fp->ref();
	i++;
    }
}

QRTFormat QRTFormatArray::operator [] (int pos) const
{
    if ( !size )
	return QRTFormat();

    int i = 0;
    int p = 0;
    while ( i < size  ) {
	p += formatHints[i].length;
	if ( p > pos )
	    break;
	i++;
    }
    if ( i == size )
	i--;

    return formatHints[i].format;
}


void QRTFormatArray::debug( const char *prefix )
{
    qDebug("%s: format array: size=%d", prefix, size );
    for ( int i = 0; i < size; i++ ) {
	qDebug("      part %d: len = %d, format=%p", i, formatHints[i].length, formatHints[i].format );
    }
}

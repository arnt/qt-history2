/****************************************************************************
**
** Implementation of QByteArray class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbytearray.h"
#include "qtools_p.h"
#ifndef QT_NO_DATASTREAM
#include <qdatastream.h>
#endif

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#ifndef QT_NO_COMPRESS
#include "../3rdparty/zlib/zlib.h"
#endif
#include <ctype.h>
#include <limits.h>
#include <string.h>

/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

/*!
    \relates QCString

    This function is normally part of the C library. Qt implements
    memmove() for platforms that do not provide it.

    memmove() copies \a len bytes from \a src into \a dst. The data
    is copied correctly even if \a src and \a dst overlap.
*/

void *qmemmove( void *dst, const void *src, uint len )
{
    register char *d;
    register char *s;
    if ( dst > src ) {
	d = (char *)dst + len - 1;
	s = (char *)src + len - 1;
	while ( len-- )
	    *d-- = *s--;
    } else if ( dst < src ) {
	d = (char *)dst;
	s = (char *)src;
	while ( len-- )
	    *d++ = *s++;
    }
    return dst;
}


/*!
    \relates QCString

    Returns a duplicate string.

    Allocates space for a copy of \a src, copies it, and returns a
    pointer to the copy. If \a src is 0, it immediately returns 0.

    The returned string must be deleted using \c delete[].
*/

char *qstrdup( const char *src )
{
    if ( !src )
	return 0;
    char *dst = new char[strlen(src)+1];
    return strcpy( dst, src );
}

/*!
    \fn char *qstrcpy( char *dst, const char *src )

    \relates QCString

    A safe strcpy() function.

    Copies all characters up to and including the '\0' from \a src
    into \a dst and returns a pointer to \a dst.
*/

/*!
    \relates QCString

    A safe strncpy() function.

    Copies at most \a len bytes from \a src (stopping at \a len or the
    terminating '\0' whichever comes first) into \a dst and returns a
    pointer to \a dst. Guarantees that \a dst is '\0'-terminated. If
    \a src or \a dst is 0, returns 0 immediately.

    \sa qstrcpy()
*/

char *qstrncpy( char *dst, const char *src, uint len )
{
    if ( !src || !dst )
	return 0;
    strncpy( dst, src, len );
    if ( len > 0 )
	dst[len-1] = '\0';
    return dst;
}

/*!
    \fn uint qstrlen( const char *str );

    \relates QCString

    A safe strlen function.

    Returns the number of characters that precede the terminating '\0'.
    or 0 if \a str is 0.
*/

/*!
    \fn int qstrcmp( const char *str1, const char *str2 );

    \relates QCString

    A safe strcmp() function.

    Compares \a str1 and \a str2. Returns a negative value if \a str1
    is less than \a str2, 0 if \a str1 is equal to \a str2 or a
    positive value if \a str1 is greater than \a str2.

    Special case I: Returns 0 if \a str1 and \a str2 are both 0.

    Special case II: Returns a random nonzero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrncmp() qstricmp() qstrnicmp()
	\link #asciinotion Note on character comparisons \endlink
*/

/*!
    \fn int qstrncmp( const char *str1, const char *str2, uint len );

    \relates QCString

    A safe strncmp() function.

    Compares at most \a len bytes of \a str1 and \a str2.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case I: Returns 0 if \a str1 and \a str2 are both 0.

    Special case II: Returns a random nonzero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstricmp(), qstrnicmp()
	\link #asciinotion Note on character comparisons \endlink
*/

/*!
    \relates QCString

    A safe stricmp() function.

    Compares \a str1 and \a str2 ignoring the case.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case I: Returns 0 if \a str1 and \a str2 are both 0.

    Special case II: Returns a random nonzero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp(), qstrnicmp()
	\link #asciinotion Note on character comparisons \endlink
*/

int qstricmp( const char *str1, const char *str2 )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 ? 1 : ( s2 ? -1 : 0 );
    for ( ; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++ )
	if ( !c )				// strings are equal
	    break;
    return res;
}

/*!
    \relates QCString

    A safe strnicmp() function.

    Compares at most \a len bytes of \a str1 and \a str2 ignoring the case.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a str1
    is equal to \a str2 or a positive value if \a str1 is greater than \a
    str2.

    Special case I: Returns 0 if \a str1 and \a str2 are both 0.

    Special case II: Returns a random nonzero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp() qstricmp()
	\link #asciinotion Note on character comparisons \endlink
*/

int qstrnicmp( const char *str1, const char *str2, uint len )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 ? 1 : ( s2 ? -1 : 0 );
    for ( ; len--; s1++, s2++ ) {
	if ( (res = (c=tolower(*s1)) - tolower(*s2)) )
	    return res;
	if ( !c )				// strings are equal
	    break;
    }
    return 0;
}


static Q_UINT16 crc_tbl[16];
static bool   crc_tbl_init = FALSE;

static void createCRC16Table()			// build CRC16 lookup table
{
    register uint i;
    register uint j;
    uint v0, v1, v2, v3;
    for ( i = 0; i < 16; i++ ) {
	v0 = i & 1;
	v1 = ( i >> 1 ) & 1;
	v2 = ( i >> 2 ) & 1;
	v3 = ( i >> 3 ) & 1;
	j = 0;
#undef SET_BIT
#define SET_BIT(x, b, v) (x) |= (v) << (b)
	SET_BIT( j,  0, v0 );
	SET_BIT( j,  7, v0 );
	SET_BIT( j, 12, v0 );
	SET_BIT( j,  1, v1 );
	SET_BIT( j,  8, v1 );
	SET_BIT( j, 13, v1 );
	SET_BIT( j,  2, v2 );
	SET_BIT( j,  9, v2 );
	SET_BIT( j, 14, v2 );
	SET_BIT( j,  3, v3 );
	SET_BIT( j, 10, v3 );
	SET_BIT( j, 15, v3 );
	crc_tbl[i] = j;
    }
}

/*!
    \relates QByteArray

    Returns the CRC-16 checksum of \a len bytes starting at \a data.

    The checksum is independent of the byte order (endianness).
*/

Q_UINT16 qChecksum( const char *data, uint len )
{
    if ( !crc_tbl_init ) {			// create lookup table

#ifdef QT_THREAD_SUPPORT
	QMutexLocker locker( qt_global_mutexpool ?
			     qt_global_mutexpool->get( &crc_tbl_init ) : 0 );
#endif // QT_THREAD_SUPPORT

	if ( !crc_tbl_init ) {
	    createCRC16Table();
	    crc_tbl_init = TRUE;
	}
    }
    register Q_UINT16 crc = 0xffff;
    uchar c;
    uchar *p = (uchar *)data;
    while ( len-- ) {
	c = *p++;
	crc = ( (crc >> 4) & 0x0fff ) ^ crc_tbl[((crc ^ c) & 15)];
	c >>= 4;
	crc = ( (crc >> 4) & 0x0fff ) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return ~crc & 0xffff;
}

/*!
    \fn QByteArray qCompress( const QByteArray& data )

    \relates QByteArray

    Compresses the array \a data and returns the compressed byte
    array.

    \sa qUncompress()
*/

/*!
    \relates QByteArray

    \overload

    Compresses the array \a data which is \a nbytes long and returns the
    compressed byte array.
*/

#ifndef QT_NO_COMPRESS
QByteArray qCompress( const uchar* data, int nbytes )
{
    if ( nbytes == 0 ) {
	QByteArray tmp( 4 );
	tmp.fill( 0 );
	return tmp;
    }
    if ( !data ) {
	qWarning( "qCompress: data is NULL." );
	return QByteArray();
    }

    ulong len = nbytes * 2;
    QByteArray bazip;
    int res;
    do {
	bazip.resize( len + 4 );
	res = ::compress(  (uchar*)bazip.data()+4, &len, (uchar*)data, nbytes );

	switch ( res ) {
	case Z_OK:
	    bazip.resize( len + 4 );
	    bazip[0] = ( nbytes & 0xff000000 ) >> 24;
	    bazip[1] = ( nbytes & 0x00ff0000 ) >> 16;
	    bazip[2] = ( nbytes & 0x0000ff00 ) >> 8;
	    bazip[3] = ( nbytes & 0x000000ff );
	    break;
	case Z_MEM_ERROR:
	    qWarning( "qCompress: Z_MEM_ERROR: Not enough memory." );
	    bazip.resize( 0 );
	    break;
	case Z_BUF_ERROR:
	    len *= 2;
	    break;
	}
    } while ( res == Z_BUF_ERROR );

    return bazip;
}
#endif

/*!
    \fn QByteArray qUncompress( const QByteArray& data )

    \relates QByteArray

    Uncompresses the array \a data and returns the uncompressed byte
    array.

    Returns an empty QByteArray if the input data was corrupt.
    \omit
    ADD THE FOLLOWING FOR Qt 4.0
    This function will uncompress data compressed with qCompress()
    from this and any earlier Qt version, back to Qt 3.1 when this
    feature was added.
    \endomit

    \sa qCompress()
*/

/*!
    \relates QByteArray

    \overload

    Uncompresses the array \a data which is \a nbytes long and returns
    the uncompressed byte array.
*/

#ifndef QT_NO_COMPRESS
QByteArray qUncompress( const uchar* data, int nbytes )
{
    if ( !data ) {
	qWarning( "qUncompress: data is NULL." );
	return QByteArray();
    }
    if ( nbytes <= 4 ) {
	if ( nbytes < 4 || ( data[0]!=0 || data[1]!=0 || data[2]!=0 || data[3]!=0 ) )
	    qWarning( "qUncompress: Input data is corrupted." );
	return QByteArray();
    }
    ulong expectedSize = ( data[0] << 24 ) | ( data[1] << 16 ) |
		       ( data[2] <<  8 ) | ( data[3]       );
    ulong len = qMax( expectedSize,  1ul );
    QByteArray baunzip;
    int res;
    do {
	baunzip.resize( len );
	res = ::uncompress( (uchar*)baunzip.data(), &len,
			    (uchar*)data+4, nbytes-4 );

	switch ( res ) {
	case Z_OK:
	    if ( (int)len != baunzip.size() )
		baunzip.resize( len );
	    break;
	case Z_MEM_ERROR:
	    qWarning( "qUncompress: Z_MEM_ERROR: Not enough memory." );
	    break;
	case Z_BUF_ERROR:
	    len *= 2;
	    break;
	case Z_DATA_ERROR:
	    qWarning( "qUncompress: Z_DATA_ERROR: Input data is corrupted." );
	    break;
	}
    } while ( res == Z_BUF_ERROR );

    if ( res != Z_OK )
	baunzip = QByteArray();

    return baunzip;
}
#endif



static char null_char = '\0';
Q_EXPORT QByteArray::Data QByteArray::shared_null = { Q_ATOMIC_INIT(1), 0, 0, &null_char, {0} };
QByteArray::Data QByteArray::shared_empty = { Q_ATOMIC_INIT(1), 0, 0, &null_char, {0} };

QByteArray::QByteArray() : d(&shared_null) 
{ 
    ++d->ref; 
}

QByteArray::QByteArray(const char *s)
{
    if (!s) {
	d = &shared_null;
    } else if (!*s) {
	d = &shared_empty;
    } else {
	int len = strlen(s);
	d = (Data*) qMalloc(sizeof(Data)+len);
	if (!d) {
	    d = &shared_null;
	} else {
	    d->ref = 0;
	    d->alloc = d->size = len;
	    d->data = d->array;
	    memcpy(d->array, s, len+1); // include null terminator
	}
    }
    ++d->ref;
}

QByteArray::QByteArray(const char*s, int size)
{
   if (!s) {
	d = &shared_null;
   } else if (size <= 0) {
       d = &shared_empty;
   } else {
       d = (Data*) qMalloc(sizeof(Data)+size);
       if (!d) {
	   d = &shared_null;
       } else {
	   d->ref = 0;
	   d->alloc = d->size = size;
	   d->data = d->array;
	   memcpy(d->array, s, size);
	   d->array[size] = '\0';
       }
   }
   ++d->ref;
}

QByteArray::QByteArray(int size, char c)
{
   if (size <= 0) {
	d = &shared_null;
    } else {
	d = (Data*) qMalloc(sizeof(Data)+size);
	if (!d) {
	    d = &shared_null;
	} else {
	    d->ref = 0;
	    d->alloc = d->size = size;
	    d->data = d->array;
	    d->array[size] = '\0';
	    memset(d->array, c, size);
	}
    }
    ++d->ref;
}


void QByteArray::resize(int size)
{
    if (size <= 0) {
	Data *x = &shared_empty;
	++x->ref;
	x = qAtomicSetPtr(&d, x);
	if (!--x->ref)
	    qFree(x);
    } else {
	if (d->ref != 1 || size > d->alloc || (size < d->size && size < d->alloc >> 1))
	    realloc(qAllocMore(size, sizeof(Data)));
	if (d->alloc >= size) {
	    d->size = size;
	    d->data = d->array;
	    d->array[size] = '\0';
	}
    }
}

QByteArray& QByteArray::fill(char c, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size)
	memset(d->data, c, d->size);
    return *this;
}

void QByteArray::reserve(int size)
{
    if (d->ref != 1 || size > d->alloc)
	realloc(size);
}

void QByteArray::realloc(int alloc)
{
    if (d->ref == 1) {
	Data *x = (Data*) qRealloc(d, sizeof(Data)+alloc);
	if (!x)
	    return;
	x->data = x->array;
	d = x;
    } else {
	Data *x = (Data*) qMalloc(sizeof(Data)+alloc);
	if (!x)
	    return;
	::memcpy(x, d, sizeof(Data)+qMin(alloc, d->alloc));
	x->ref = 1;
	x->data = x->array;
	x = qAtomicSetPtr(&d, x);
	if (!--x->ref)
	    qFree(x);
    }
    d->alloc = alloc;
}

void QByteArray::expand(int i)
{
    resize(qMax(i+1, d->size));
}

QByteArray &QByteArray::operator=(const char *s)
{
    Data *x;
    if (!s) {
	x = &shared_null;
    } else if (!*s) {
	x = &shared_empty;
    } else {
	int len = strlen(s);
	if (d->ref != 1 || len > d->alloc || (len < d->size && len < d->alloc >> 1))
	    realloc(len);
	x = d;
	memcpy(x->data, s, len+1); // include null terminator
	x->size = len;
    }
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	 qFree(x);
    return *this;
}

QByteArray& QByteArray::prepend(char c)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
	realloc(qAllocMore(d->size + 1, sizeof(Data)));
    memmove(d->data+1, d->data, d->size);
    d->data[0] = c;
    d->data[d->size] = '\0';
    ++d->size;
    return *this;
}

QByteArray& QByteArray::prepend(const char *s)
{
    if (s) {
	int len = strlen(s);
	if (d->ref != 1 || d->size + len > d->alloc)
	    realloc(qAllocMore(d->size + len, sizeof(Data)));
	memmove(d->data+len, d->data, d->size);
	memcpy(d->data, s, len);
	d->size += len;
	d->data[d->size] = '\0';
    }
    return *this;
}

QByteArray& QByteArray::prepend(const QByteArray& a)
{
    if (d == &shared_null || d == &shared_empty)
	*this = a;
    else if (a.d != &shared_null) {
	QByteArray tmp = *this;
	*this = a;
	append(tmp);
    }
    return *this;
}

QByteArray& QByteArray::append(char c)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
	realloc(qAllocMore(d->size + 1, sizeof(Data)));
    d->data[d->size++] = c;
    d->data[d->size] = '\0';
    return *this;
}

QByteArray& QByteArray::append(const char *s)
{
    if (s) {
	int len = strlen(s);
	if (d->ref != 1 || d->size + len > d->alloc)
	    realloc(qAllocMore(d->size + len, sizeof(Data)));
	memcpy(d->data + d->size, s, len+1); // include null terminator
	d->size += len;
    }
    return *this;
}

QByteArray& QByteArray::append(const QByteArray& a)
{
    if (d == &shared_null || d == &shared_empty)
	*this = a;
    else if (a.d != &shared_null) {
	if (d->ref != 1 || d->size + a.d->size > d->alloc)
	    realloc(qAllocMore(d->size + a.d->size, sizeof(Data)));
	memcpy(d->data + d->size, a.d->data, a.d->size+1); // include null terminator
	d->size += a.d->size;
    }
    return *this;
}


QByteArray &QByteArray::insert(int i, char c)
{
    if (i < 0)
	i += d->size;
    if (i < 0)
	return *this;
    int oldsize = d->size;
    expand(qMax(i, d->size));
    if (i > oldsize)
	memset(d->data + oldsize, 0x20, i - oldsize);
    else
	::memmove(d->data + i + 1, d->data + i, (d->size - i)*sizeof(char));
    d->data[i] = c;
    return *this;
}

QByteArray &QByteArray::insert(int i, const char *s)
{
    if (!s || *s == '\0' || i < 0)
	return *this;
    int l = strlen(s);
    int oldsize = d->size;
    expand(qMax(d->size, i) + l - 1);
    if (i > oldsize)
	memset(d->data + oldsize, 0x20, i - oldsize);
    else
	::memmove(d->data + i + l, d->data + i, (d->size - i - l)*sizeof(char));
    memcpy(d->data + i, s, l*sizeof(char));
    return *this;
}

QByteArray &QByteArray::insert(int i, const QByteArray &a)
{
    if (i < 0 || a.d->size == 0)
	return *this;
    expand(qMax(d->size, i) + a.d->size - 1);
    ::memmove(d->data + i + a.d->size, d->data + i, (d->size - i - a.d->size)*sizeof(char));
    memcpy(d->data + i, a.d->data, a.d->size*sizeof(char));
    return *this;
}

/*!
    Removes \a len characters from the array, starting at position \a
    index, and returns a reference to the array.

    If \a index is out of range, nothing happens. If \a index is
    valid, but \a index + \a len is larger than the length of the
    array, the array is truncated at position \a index.

    \code
    QByteArray s = "Montreal";
    s.remove( 1, 4 );         // s == "Meal"
    \endcode

    \sa insert(), replace()
*/

QByteArray &QByteArray::remove( int index, int len )
{
    if (len == 0 || index >= d->size)
	return *this;
    detach();
    if ( index + len >= d->size ) {
	resize( index );
    } else {
	memmove( d->data+index, d->data+index+len, d->size-index-len );
	resize(d->size-len);
    }
    return *this;
}

QByteArray &QByteArray::replace(int index, int len, const QByteArray &after)
{
    remove(index, len);
    return insert(index, after);
}

QByteArray &QByteArray::replace(char before, const QByteArray &after)
{
    char b[2] = { before, '\0' };
    QConstByteArray cb(b, 1);
    return replace(cb, after);
}

QByteArray &QByteArray::replace(const QByteArray &before, const QByteArray &after)
{
    if ( isNull() || before == after )
	return *this;

    int index = 0;
    const int bl = before.d->size;
    const int al = after.d->size;
    int len = d->size;
    char *d = data();

    if ( bl == al ) {
	if ( bl ) {
	    while( (index = indexOf( before, index ) ) != -1 ) {
		memcpy( d+index, after, al );
		index += bl;
	    }
	}
    } else if ( al < bl ) {
	uint to = 0;
	uint movestart = 0;
	uint num = 0;
	while( (index = indexOf( before, index ) ) != -1 ) {
	    if ( num ) {
		int msize = index - movestart;
		if ( msize > 0 ) {
		    memmove( d + to, d + movestart, msize );
		    to += msize;
		}
	    } else {
		to = index;
	    }
	    if ( al ) {
		memcpy( d + to, after, al );
		to += al;
	    }
	    index += bl;
	    movestart = index;
	    num++;
	}
	if ( num ) {
	    int msize = len - movestart;
	    if ( msize > 0 )
		memmove( d + to, d + movestart, msize );
	    resize( len - num*(bl-al) );
	}
    } else {
	// the most complex case. We don't want to loose performance by doing repeated
	// copies and reallocs of the string.
	while( index != -1 ) {
	    uint indices[4096];
	    uint pos = 0;
	    while( pos < 4095 ) {
		index = indexOf(before, index);
		if ( index == -1 )
		    break;
		indices[pos++] = index;
		index += bl;
		// avoid infinite loop
		if ( !bl )
		    index++;
	    }
	    if ( !pos )
		break;

	    // we have a table of replacement positions, use them for fast replacing
	    int adjust = pos*(al-bl);
	    // index has to be adjusted in case we get back into the loop above.
	    if ( index != -1 )
		index += adjust;
	    int newlen = len + adjust;
	    int moveend = len;
	    if ( newlen > len ) {
		resize( newlen );
		len = newlen;
	    }
	    d = this->d->data;

	    while( pos ) {
		pos--;
		int movestart = indices[pos] + bl;
		int insertstart = indices[pos] + pos*(al-bl);
		int moveto = insertstart + al;
		memmove( d + moveto, d + movestart, (moveend - movestart) );
		if ( !!after )
		    memcpy( d + insertstart, after, al );
		moveend = movestart - bl;
	    }
	}
    }
    return *this;
}

QByteArray &QByteArray::replace(char before, char after)
{
    if (d->size) {
	char *i = data();
	char *e = i + d->size;
	for (; i != e; ++i)
	    if (*i == before)
		* i = after;
    }
    return *this;
}

int QByteArray::indexOf(char c, int i) const
{
    if (i < 0)
	i = qMax(i + d->size, 0);
    if (i < d->size) {
	const char *n = d->data + i - 1;
	const char *e = d->data + d->size;
	while (++n != e)
	if (*n == c)
	    return  n - d->data;
    }
    return -1;
}

int QByteArray::lastIndexOf(char c, int i) const
{
    if (i < 0)
	i += d->size;
    else if (i > d->size)
	i = d->size-1;
    if (i >= 0) {
	const char *b = d->data;
	const char *n = d->data + i + 1;
	while (n-- != b)
	    if (*n == c)
		return  n - b;
    }
    return -1;
}

#define REHASH(a) \
    if (sl_minus_1 < sizeof(uint)  *CHAR_BIT) \
	hashHaystack -= (a) << sl_minus_1; \
    hashHaystack <<= 1


int QByteArray::indexOf(const QByteArray& a, int i) const
{
    const int l = d->size;
    const int sl = a.d->size;
    if (i > d->size || sl + i > l)
	return -1;
    if (sl == 0)
	return i;
    if (sl == 1)
	return indexOf(*a.d->data, i);
    const char *needle = a.d->data;
    const char *haystack = d->data + i;
    const char *end = d->data + (l-sl);
    const uint sl_minus_1 = sl-1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < sl; ++idx) {
	hashNeedle = ((hashNeedle<<1) + needle[idx]);
	hashHaystack = ((hashHaystack<<1) + haystack[idx]);
    }
    hashHaystack -= *(haystack+sl_minus_1);

    while (haystack <= end) {
	hashHaystack += *(haystack+sl_minus_1);
	if (hashHaystack == hashNeedle  && *needle == *haystack
	     && strncmp(needle, haystack, sl) == 0)
	    return haystack - d->data;

	REHASH(*haystack);
	++haystack;
    }
    return -1;
}

int QByteArray::lastIndexOf(const QByteArray& a, int i) const
{
    const int sl = a.d->size;
    const int l = d->size;
    int delta = l-sl;
    if (i < 0)
	i = delta;
    if (i < 0 || i > l)
	return -1;
    if (i > delta)
	i = delta;
    if (sl == 1)
	return findRev(*a.d->data, i);

    const char *needle = a.d->data;
    const char *haystack = d->data + i;
    const char *end = d->data;
    const uint sl_minus_1 = sl-1;
    const char *n = needle+sl_minus_1;
    const char *h = haystack+sl_minus_1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < sl; ++idx) {
	hashNeedle = ((hashNeedle<<1) + *(n-idx));
	hashHaystack = ((hashHaystack<<1) + *(h-idx));
    }
    hashHaystack -= *haystack;
    while (haystack >= end) {
	hashHaystack += *haystack;
	if (hashHaystack == hashNeedle  && strncmp(needle, haystack, sl) == 0)
	    return haystack-d->data;
	--haystack;
	REHASH(*(haystack+sl));
    }
    return -1;

}

int QByteArray::count(char c) const
{
    int num = 0;
    const char *i = d->data + d->size;
    const char *b = d->data;
    while (i != b)
	if (*--i == c)
	    ++num;
    return num;
}

int QByteArray::count(const char *a) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(a, i+1)) != -1)
	++num;
    return num;
}

int QByteArray::count(const QByteArray& a) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(a, i+1)) != -1)
	++num;
    return num;
}

QByteArray QByteArray::left(int len)  const
{
    if (d == &shared_null)
	return QByteArray();
    if (len > d->size)
	return *this;
    if (len < 0)
	len = 0;
    return QByteArray(d->data, len);
}

QByteArray QByteArray::right(int len) const
{
    if (d == &shared_null)
	return QByteArray();
    if (len > d->size)
	return *this;
    if (len < 0)
	len = 0;
    QByteArray ba = QByteArray(d->data + d->size - len, len);
    return ba;
}

QByteArray QByteArray::mid(int i, int len) const
{
    if (d == &shared_null || d == &shared_empty || i >= d->size)
	return QByteArray();
    if (len < 0)
	len = d->size - i;
    if (i < 0) {
	len += i;
	i = 0;
    }
    if (len + i > d->size)
	len = d->size - i;
    if (i == 0 && len == d->size)
	return *this;
    return QByteArray(d->data + i, len);
}

/*!
    Returns a new string that is a copy of this string converted to lower
    case.

    Example:
    \code
    QCString s("Credit");
    QCString t = s.toLower();  // t == "credit"
    \endcode

    \sa toUpper()
      \link #asciinotion Note on character comparisons \endlink
*/
QByteArray QByteArray::toLower() const
{
    QByteArray s(*this);
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = tolower( (uchar) *p );
	    p++;
	}
    }
    return s;
}

/*!
    Returns a new string that is a copy of this string converted to upper case.

    Example:
    \code
    QCString s( "Debit" );
    QCString t = s.toUpper();  // t == "DEBIT"
    \endcode

    \sa toLower()
      \link #asciinotion Note on character comparisons \endlink
*/

QByteArray QByteArray::toUpper() const
{
    QByteArray s(*this);
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = toupper(*p);
	    p++;
	}
    }
    return s;
}


/*!
    \relates QByteArray

    Writes byte array \a a to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
#ifndef QT_NO_DATASTREAM

QDataStream &operator<<( QDataStream &s, const QByteArray &a )
{
    return s.writeBytes( a, a.size() );
}

void QByteArray::clear() 
{ 
    if (!--d->ref) 
        qFree(d); 
    d = &shared_null; 
    ++d->ref; 
}

/*!
    \relates QByteArray

    Reads a byte array into \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QByteArray &a )
{
    Q_INT32 len;
    s >> len;					// read size of array
    if ( len == 0 || s.eof() ) {		// end of file reached
	a.clear();
	return s;
    }
    a.resize( len );
    if (a.size() != len) {
	qWarning( "QDataStream: Not enough memory to read QByteArray" );
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( a.data(), (uint)len );
    return s;
}
#endif //QT_NO_DATASTREAM


QConstByteArray::QConstByteArray(const char *chars, int length)
    : QByteArray((Data *)qMalloc(sizeof(Data))) {
    d->ref = 1;
    d->alloc = d->size = length;
    d->data = chars ? (char *)chars : d->array;
    *d->array = '\0';
}
/*!
    Returns a byte array that has whitespace removed from the start
    and the end, and which has each sequence of internal whitespace
    replaced with a single space.

    \sa trimmed()
*/
QByteArray QByteArray::simplified() const
{
    if (d->size == 0)
	return *this;
    QByteArray result;
    result.resize(d->size);
    const char *from = (const char*) d->data;
    const char *fromend = (const char*) from+d->size;
    int outc=0;
    char *to   = (char*) result.d->data;
    for (;;) {
	while (from!=fromend && isspace((uchar)*from))
	    from++;
	while (from!=fromend && !isspace((uchar)*from))
	    to[outc++] = *from++;
	if (from!=fromend)
	    to[outc++] = ' ';
	else
	    break;
    }
    if (outc > 0 && to[outc-1] == ' ')
	outc--;
    result.resize(outc);
    return result;
}

/*!
    Returns a byte array that has whitespace removed from the start
    and the end.

    \sa simplified()
*/
QByteArray QByteArray::trimmed() const
{
    if (d->size == 0)
	return *this;
    const char *s = (const char*)d->data;
    if (!isspace((uchar)*s) && !isspace((uchar)s[d->size-1]))
	return *this;
    int start = 0;
    int end = d->size - 1;
    while (start<=end && isspace((uchar)s[start]))  // skip white space from start
	start++;
    if (start <= end) {                          // only white space
	while (end && isspace((uchar)s[end]))           // skip white space from end
	    end--;
    }
    int l = end - start + 1;
    if (l <= 0)
    	return QByteArray();
    return QByteArray(s+start, l);
}

/*!
  \internal
*/
bool QByteArray::ensure_constructed() 
{ 
    if (!d) { 
       d = &shared_null; 
       ++d->ref; 
       return false; 
    } 
    return true; 
}

#ifndef QT_NO_COMPAT
bool QByteArray::isNull() const { return d == &shared_null; }
#endif

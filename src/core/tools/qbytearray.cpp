/****************************************************************************
**
** Implementation of QByteArray class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include "qstring.h"
#include "qlocale.h"
#include "qlocale_p.h"
#ifndef QT_NO_DATASTREAM
#include <qdatastream.h>
#endif

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
    \relates QByteArray

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

    \relates QByteArray

    A safe strcpy() function.

    Copies all characters up to and including the '\0' from \a src
    into \a dst and returns a pointer to \a dst.
*/

/*!
    \relates QByteArray

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

    \relates QByteArray

    A safe strlen function.

    Returns the number of characters that precede the terminating '\0'.
    or 0 if \a str is 0.
*/

/*!
    \fn int qstrcmp( const char *str1, const char *str2 );

    \relates QByteArray

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

    \relates QByteArray

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
    \relates QByteArray

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
    \relates QByteArray

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


// the CRC table below is created by the following piece of code
#if 0
static void createCRC16Table()			// build CRC16 lookup table
{
    register unsigned int i;
    register unsigned int j;
    unsigned short crc_tbl[16];
    unsigned int v0, v1, v2, v3;
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
    printf("static const Q_UINT16 crc_tbl[16] = {\n");
    for (int i = 0; i < 16; i +=4)
	printf("    0x%04x, 0x%04x, 0x%04x, 0x%04x,\n", crc_tbl[i], crc_tbl[i+1], crc_tbl[i+2], crc_tbl[i+3]);
    printf("};\n");
}
#endif

static const Q_UINT16 crc_tbl[16] = {
    0x0000, 0x1081, 0x2102, 0x3183,
    0x4204, 0x5285, 0x6306, 0x7387,
    0x8408, 0x9489, 0xa50a, 0xb58b,
    0xc60c, 0xd68d, 0xe70e, 0xf78f
};

/*!
    \relates QByteArray

    Returns the CRC-16 checksum of \a len bytes starting at \a data.

    The checksum is independent of the byte order (endianness).
*/

Q_UINT16 qChecksum( const char *data, uint len )
{
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
    \fn QByteArray qCompress( const QByteArray& data, int compressionLevel )

    \relates QByteArray

    Compresses the array \a data and returns the compressed byte
    array.

    The \a compressionLevel parameter specifies how much compression
    is desired. The values are between 0 and 9. The value 9
    corresponds to the best compression (i.e. smaller compressed data)
    at the cost of a slower algorithm. The value 0 corresponds to no
    compression at all. The default value is -1, which specifies
    zlib's default compression.

    \sa qUncompress()
*/

/*!
    \relates QByteArray

    \overload

    Compresses the array \a data which is \a nbytes long and returns the
    compressed byte array.
*/

#ifndef QT_NO_COMPRESS
QByteArray qCompress( const uchar* data, int nbytes, int compressionLevel )
{
    if ( nbytes == 0 ) {
	return QByteArray(4, '\0');
    }
    if ( !data ) {
	qWarning( "qCompress: Data is null" );
	return QByteArray();
    }
    if (compressionLevel < -1 || compressionLevel > 9)
	compressionLevel = -1;

    ulong len = nbytes * 2;
    QByteArray bazip;
    int res;
    do {
	bazip.resize( len + 4 );
	res = ::compress2((uchar*)bazip.data()+4, &len, (uchar*)data, nbytes, compressionLevel);

	switch ( res ) {
	case Z_OK:
	    bazip.resize( len + 4 );
	    bazip[0] = ( nbytes & 0xff000000 ) >> 24;
	    bazip[1] = ( nbytes & 0x00ff0000 ) >> 16;
	    bazip[2] = ( nbytes & 0x0000ff00 ) >> 8;
	    bazip[3] = ( nbytes & 0x000000ff );
	    break;
	case Z_MEM_ERROR:
	    qWarning( "qCompress: Z_MEM_ERROR: Not enough memory" );
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

    This function will uncompress data compressed with qCompress()
    from this and any earlier Qt version, back to Qt 3.1 when this
    feature was added.

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




inline bool qIsUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

inline bool qIsDigit(char c)
{
    return c >= '0' && c <= '9';
}

inline char qToLower(char c)
{
    if (c >= 'A' && c <= 'Z')
    	return c - 'A' + 'a';
    else
    	return c;
}


Q_CORE_EXPORT QByteArray::Data QByteArray::shared_null = { Q_ATOMIC_INIT(1), 0, 0, shared_null.array, {0} };
QByteArray::Data QByteArray::shared_empty = { Q_ATOMIC_INIT(1), 0, 0, shared_empty.array, {0} };

/*!
    \class QByteArray
    \brief The QByteArray class provides an array of bytes.

    \ingroup tools

    It is useful for manipulating memory areas with custom data.
    QByteArrays are implicitly shared, and they are always
    \c{\0}-terminated.
*/

/*!
    \fn QByteArray::QByteArray(const QByteArray &other)

    Constructs a byte array that is a copy of \a other.
*/

/*!
    \fn QByteArray::~QByteArray()

    Destructor.
*/

/*!
    \fn QByteArray &QByteArray::operator=(const QByteArray &other)

    Assign's byte array \a other to this byte array.
*/

/*!
    \fn int QByteArray::size() const

    Returns the size of this byte array.
*/

/*!
    \fn bool QByteArray::isEmpty() const

    Returns true if this byte array is empty; otherwise returns false.
*/

/*!
    \fn bool QByteArray::operator!() const

    Returns true if this byte array is empty; otherwise returns false.
*/

/*!
    \fn int QByteArray::capacity() const

    Returns the maximum number of bytes that can be stored in the
    byte array without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function. If you want to know how many
    items are in the byte array, call size().

    \sa reserve(), squeeze()
*/

/*!
    \fn void QByteArray::reserve(int size)

    Attempts to allocate memory for at least \a size bytes. If you
    know in advance how large the byte array will be, you can call
    this function, and if you call resize() often you are likely to
    get better performance. If \a size is an underestimate, the worst
    that will happen is that the QByteArray will be a bit slower.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function. If you want to change the size
    of the byte array, call resize().

    \sa squeeze(), capacity()
*/

/*! \fn void QByteArray::squeeze()

    Releases any memory not required to store the array data.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function.

    \sa reserve(), capacity()
*/

/*!
    \fn char *QByteArray::data()

    \overload

    Returns a pointer to the byte array's data. The QByteArray is
    implicitly shared, so if you modify the data, a detach() will
    occur (which could be expensive if there's a lot of data to copy).
*/

/*!
    \fn const char *QByteArray::data() const

    Returns a pointer which provides read-only access to the byte
    array's data.
*/

/*!
    \fn const char *QByteArray::constData() const

    Returns a pointer which provides read-only access to the byte
    array's data.
*/

/*!
    \fn void QByteArray::detach()

    \internal
*/

/*!
    \fn bool QByteArray::isDetached() const

    \internal
*/

/*!
    \fn char QByteArray::at(int i) const

    Returns the byte at position \a i.

    \sa operator[]()
*/

/*!
    \fn char QByteArray::operator[](int i) const

    Returns the byte at position \a i.

    \sa at()
*/

/*!
    \fn QByteRef QByteArray::operator[](int i)

    \overload

    Returns a reference to the byte at position \a i.

    \sa at()
*/

/*!
    \fn char QByteArray::operator[](uint i) const

    \overload
*/

/*!
    \fn QByteRef QByteArray::operator[](uint i)

    \overload

    Returns a reference to the byte at position \a i.

    \sa at()
*/

/*!
    \fn int QByteArray::indexOf(const char *c, int from) const

    \overload

    Returns the position of the first occurrence of the string \a c,
    starting at position \a from, or -1 if \a c is not found.
*/

/*!
    \fn int QByteArray::lastIndexOf(const char *c, int i) const

    \overload

    Returns the position of the last occurrence of the string \a c,
    starting at position \a i and working backwards, or -1 if \a c is
    not found.
*/

/*!
    \fn QBool QByteArray::contains(char c) const

    \overload

    Returns true if the byte array contains the byte \a c.
*/

/*!
    \fn QBool QByteArray::contains(const char *a) const

    \overload

    Returns true if the byte array contains the string \a a.
*/

/*!
    \fn QBool QByteArray::contains(const QByteArray &a) const

    Returns true if the byte array contains the byte array \a a.
*/

/*!
    \fn  void QByteArray::truncate(int maxSize)

    If \a maxSize is less than the length of the string, then the
    string is truncated at position \a maxSize. Otherwise nothing
    happens.

    \code
	QByteArray s = "truncate me";
	s.truncate(5);            // s == "trunc"
    \endcode

    \sa resize();
*/


/*!
    \fn QByteArray QByteArray::leftJustified(int width, char fill, bool truncate) const

    Returns a byte array of length \a width that contains this byte
    array padded by the \a fill character.

    If \a truncate is false and the length of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the length of the byte array is more
    than \a width, then any characters in a copy of the byte array
    after length \a width are removed, and the copy is returned.

    \code
    QByteArray s("apple");
    QByteArray t = s.leftJustified(8, '.');        // t == "apple..."
    \endcode

    \sa rightJustified()
*/

/*!
    \fn QByteArray QByteArray::rightJustified(int width, char fill, bool truncate) const

    Returns a byte array of length \a width that contains the \a fill
    character followed by this byte array.

    If \a truncate is false and the length of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the length of the byte array is more
    than \a width, then the resulting byte array is truncated at
    position \a width.

    \code
    QString s("apple");
    QString t = s.rightJustified(8, '.');  // t == "...apple"
    \endcode

    \sa leftJustified()
*/

/*!
    \fn QByteArray &QByteArray::replace(int index, int len, const char *s)

    \overload

    Replaces \a len characters in this byte array from position \a
    index with the string in \a s and returns a reference to this byte
    array.
*/

/*!
    \fn QByteArray &QByteArray::replace(char before, const char *after)

    \overload

    Replaces every occurrence of character \a before with the string
    \a after and returns a reference to this byte array.
*/

/*!
    \fn QByteArray &QByteArray::replace(const char *before, const char *after)

    \overload

    Replaces every occurrence of the string \a before with the string
    \a after and returns a reference to this byte array.
*/

/*!
    \fn QByteArray &QByteArray::operator+=(char c)

    \overload

    Appends character \a c to this byte array and returns a reference
    to this byte array.
*/

/*!
    \fn QByteArray &QByteArray::operator+=(const char *s)

    \overload

    Appends the string \a s to this byte array and returns a reference
    to this byte array.
*/

/*!
    \fn QByteArray &QByteArray::operator+=(const QByteArray &a)

    Appends the byte array \a a to this byte array and returns a
    reference to this byte array.
*/

/*!
    \fn int QByteArray::count() const

    Returns the size of the byte array.
*/

/*!
    \fn int QByteArray::length() const

    Returns the size of the byte array. (This is a synonym for count().)
*/

/*!
    \fn bool QByteArray::isNull() const

    Returns true if this byte array is empty.
*/

/*!
    \fn QByteArray::QByteArray()

    Constructs a new empty byte array.
*/

QByteArray::QByteArray() : d(&shared_null)
{
    ++d->ref;
}

/*!
    \fn QByteArray::QByteArray(const char *s)

    Constructs a new byte array that contains string \a s.
*/

QByteArray::QByteArray(const char *s)
{
    if (!s) {
	d = &shared_null;
    } else if (!*s) {
	d = &shared_empty;
    } else {
	int len = strlen(s);
	d = (Data *)qMalloc(sizeof(Data)+len);
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

/*!
    \fn QByteArray::QByteArray(const char *s, int size)

    Constructs a byte array containing the first \a size bytes of
    string \a s.
*/

QByteArray::QByteArray(const char*s, int size)
{
   if (!s) {
	d = &shared_null;
   } else if (size <= 0) {
       d = &shared_empty;
   } else {
       int len = 0;
       if (size < 0)
	   len = strlen(s);
       else
	   while (len < size && s[len] )
	       ++len;
       d = (Data *)qMalloc(sizeof(Data)+len);
       if (!d) {
	   d = &shared_null;
       } else {
	   d->ref = 0;
	   d->alloc = d->size = len;
	   d->data = d->array;
	   memcpy(d->array, s, len);
	   d->array[len] = '\0';
       }
   }
   ++d->ref;
}

/*!
    \fn QByteArray::QByteArray(int size, char c)

    Constructs a byte array of size \a size with every byte set to
    character \a c.
*/

QByteArray::QByteArray(int size, char c)
{
   if (size <= 0) {
	d = &shared_null;
    } else {
	d = (Data *)qMalloc(sizeof(Data)+size);
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

/*!
    \fn void QByteArray::resize(int size)

    Resizes the byte array to \a size bytes.
*/

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

/*!
    Resizes the byte array to \a size and fills every byte with
    character \a c.
*/

QByteArray& QByteArray::fill(char c, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size)
	memset(d->data, c, d->size);
    return *this;
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
	Data *x = (Data *)qMalloc(sizeof(Data)+alloc);
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

/*!
    \fn QByteArray &QByteArray::operator=(const char *s)

    \overload

    Assigns the string \a s to this byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::prepend(char c)

    \overload

    Prepend's character \a c to this byte array.
*/

QByteArray& QByteArray::prepend(char c)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
	realloc(qAllocMore(d->size + 1, sizeof(Data)));
    memmove(d->data+1, d->data, d->size);
    d->data[0] = c;
    ++d->size;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    \fn QByteArray &QByteArray::prepend(const char *s)

    \overload

    Prepend's the string \a s to this byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::prepend(const QByteArray &a)

    Prepend's the byte array \a a to this byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::append(char c)

    \overload

    Appends character \a c to this byte array.
*/

QByteArray& QByteArray::append(char c)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
	realloc(qAllocMore(d->size + 1, sizeof(Data)));
    d->data[d->size++] = c;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    \fn QByteArray &QByteArray::append(const char *s)

    \overload

    Appends the string \a s to this byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::append(const QByteArray &a)

    Appends the byte array \a a to this byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::insert(int i, char c)

    \overload

    Inserts character \a c at position \a i in the byte array. If \a i
    is \> size(), the array is first expanded (padded with spaces).
*/

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

/*!
    \fn QByteArray &QByteArray::insert(int i, const char *s)

    \overload

    Inserts the string \a s at position \a i in the byte array. If \a
    i is \> size(), the array is first expanded (padded with spaces).
*/

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

/*!
    \fn QByteArray &QByteArray::insert(int i, const QByteArray &a)

    Inserts the byte array \a a at position \a i in the byte array.
*/

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

/*!
    \fn QByteArray &QByteArray::replace(int index, int len, const QByteArray &after)

    Replaces \a len bytes from position \a index with the byte array
    \a after.
*/

QByteArray &QByteArray::replace(int index, int len, const QByteArray &after)
{
    remove(index, len);
    return insert(index, after);
}

/*!
    \fn QByteArray &QByteArray::replace(char before, const QByteArray &after)

    \overload

    Replaces every occurrence of character \a before with the byte
    array \a after.
*/

QByteArray &QByteArray::replace(char before, const QByteArray &after)
{
    char b[2] = { before, '\0' };
    QConstByteArray cb(b, 1);
    return replace(cb, after);
}

/*!
    \fn QByteArray &QByteArray::replace(const QByteArray &before, const QByteArray &after)

    \overload

    Replaces every occurrence of the byte array \a before with the
    byte array \a after.
*/

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
		if (after.size())
		    memcpy( d + insertstart, after, al );
		moveend = movestart - bl;
	    }
	}
    }
    return *this;
}

/*!
    \fn QByteArray &QByteArray::replace(char before, char after)

    \overload

    Replaces every occurrence of character \a before with the
    character \a after.
*/

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

/*!
    \fn int QByteArray::indexOf(char c, int i) const

    \overload

    Returns the position of the first occurrence of character \a c in
    the byte array starting at position \a i, or -1 if \a c could not
    be found.
*/

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

/*!
    \fn int QByteArray::lastIndexOf(char c, int i) const

    \overload

    Returns the position of the last occurrence of character \a c in
    the byte array starting at position \a i and working backwards, or
    -1 if \a c could not be found.
*/

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


/*!
    \fn int QByteArray::indexOf(const QByteArray &a, int i) const

    Returns the position of the first occurrence of the byte array \a
    a in the byte array starting at position \a i, or -1 if \a a could
    not be found.
*/

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

/*!
    \fn int QByteArray::lastIndexOf(const QByteArray &a, int i) const

    Returns the position of the last occurrence of byte array \a a in
    the byte array starting at position \a i and working backwards, or
    -1 if \a a could not be found.
*/

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
	return lastIndexOf(*a.d->data, i);

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

/*!
    \fn int QByteArray::count(char c) const

    \overload

    Returns the number of occurrences of character \a c in the byte
    array.
*/

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

/*!
    \fn int QByteArray::count(const char *a) const

    \overload

    Returns the number of occurrences of the string \a a in the byte
    array.
*/

int QByteArray::count(const char *a) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(a, i+1)) != -1)
	++num;
    return num;
}

/*!
    \fn int QByteArray::count(const QByteArray &a) const

    \overload

    Returns the number of occurrences of the byte array \a a in the
    byte array.
*/

int QByteArray::count(const QByteArray& a) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(a, i+1)) != -1)
	++num;
    return num;
}

/*!
    Returns true if the byte array starts with the byte array \a a;
    otherwise returns false.
 */
bool QByteArray::startsWith(const QByteArray& a) const
{
    if (d == a.d)
        return true;
    if (d->size == 0 || a.d->size == 0)
        return false;
    if (d->size < a.d->size)
        return false;
    return memcmp(d->data, a.d->data, a.d->size) == 0;
}

/*!
    \overload

    Returns true if the byte array's first character is \a c;
    otherwise returns false.
 */
bool QByteArray::startsWith(char c) const
{
    if (d == &shared_null)
        return false;
    return d->data[0] == c;
}

/*!
    \overload

    Returns true if the byte array's first character is \a c;
    otherwise returns false.
 */
bool QByteArray::startsWith(const char *c) const
{
    if (d == &shared_null || !c)
        return false;
    int len = qstrlen(c);
    if (!len && !d->size)
        return true;
    if (!len || d->size < len)
        return false;
    return qstrncmp(d->data, c, len) == 0;
}

/*!
    Returns true if the byte array ends with the byte array \a a;
    otherwise returns false.
 */
bool QByteArray::endsWith(const QByteArray& a) const
{
    if (d == a.d)
        return true;
    if (d->size == 0 || a.d->size == 0)
        return false;
    if (d->size < a.d->size)
        return false;
    return memcmp(d->data + d->size - a.d->size, a.d->data, a.d->size) == 0;
}

/*!
    \overload

    Returns true if the byte array's last character is \a c;
    otherwise returns false.
 */
bool QByteArray::endsWith(char c) const
{
    if (d == &shared_null)
        return false;
    if (d->size == 0)
        return c == '\0';
    return d->data[d->size - 1] == c;
}

/*!
    \overload

    Returns true if the byte array's last character is \a c;
    otherwise returns false.
 */
bool QByteArray::endsWith(const char *c) const
{
    if (d == &shared_null || !c)
        return false;
    int len = qstrlen(c);
    if (!len && !d->size)
        return true;
    if (!len || d->size < len)
        return false;
    return qstrncmp(d->data + d->size - len, c, len) == 0;
}

/*!
    \fn QByteArray QByteArray::left(int len)  const

    Returns a byte array containing the left-most \a len bytes of this
    byte array.
*/

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

/*!
    \fn QByteArray QByteArray::right(int len) const

    Returns a byte array containing the right-most \a len bytes of
    this byte array.
*/

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

/*!
    \fn QByteArray QByteArray::mid(int i, int len) const

    Returns a byte array containing \a len bytes from this byte array,
    starting at position \a i.
*/

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
    Returns a new byte array that is a copy of this byte array
    with the charcters converted to lower case.

    Example:
    \code
    QByteArray s("Credit");
    QByteArray t = s.toLower();  // t == "credit"
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
    Returns a new byte array that is a copy of this byte array
    with the charcters converted to upper case.

    Example:
    \code
    QByteArray s( "Debit" );
    QByteArray t = s.toUpper();  // t == "DEBIT"
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
    \fn void QByteArray::clear()

    Clears the contents of the byte array and makes it null.
*/

void QByteArray::clear()
{
    if (!--d->ref)
        qFree(d);
    d = &shared_null;
    ++d->ref;
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
    if (a.isNull() && s.version() >= 6) {
        s << (Q_UINT32)0xffffffff;
        return s;
    }
    return s.writeBytes( a, a.size() );
}

/*!
    \relates QByteArray

    Reads a byte array into \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QByteArray &a )
{
    Q_UINT32 len;
    s >> len;					// read size of array
    if ( len == 0xffffffff ) {
	a.clear();
	return s;
    }
    if ((int)len <= 0) {
        a = QByteArray("");
        return s;
    }
    if (s.eof()) {
        a.clear();
        return s;
    }
    a.resize( (int)len );
    if (a.size() != (int)len) {
	qWarning( "QDataStream: Not enough memory to read QByteArray" );
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( a.data(), (uint)len );
    return s;
}
#endif //QT_NO_DATASTREAM


QConstByteArray::QConstByteArray(const char *chars, int length)
    : QByteArray((Data *)qMalloc(sizeof(Data)), 0)
{
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

QByteArray QByteArray::leftJustified(int width, char fill, bool truncate) const
{
    QByteArray result;
    int len = qstrlen(d->data);
    int padlen = width - len;
    if (padlen > 0) {
	result.resize(len+padlen);
	if (len)
	    memcpy(result.d->data, d->data, sizeof(char)*len);
	memset(result.d->data+len, fill, padlen);
    } else {
	if (truncate)
	    result = left(width);
	else
	    result = *this;
    }
    return result;
}

QByteArray QByteArray::rightJustified(int width, char fill, bool truncate) const
{
    QByteArray result;
    int len = qstrlen(d->data);
    int padlen = width - len;
    if (padlen > 0) {
        result.resize(len+padlen);
        if (len)
	    memcpy(result.d->data+padlen, data(), len);
        memset(result.d->data, fill, padlen);
    } else {
        if (truncate)
            result = left(width);
        else
            result = *this;
    }
    return result;
}

bool QByteArray::isNull() const { return d == &shared_null; }


/*!
    Returns the string converted to a \c {long long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

Q_LLONG QByteArray::toLongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if ( base != 0 && (base < 2 || base > 36) ) {
	qWarning( "QByteArray::toLongLong: Invalid base (%d)", base );
	base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToLongLong(*this, base, ok);
}

/*!
    Returns the string converted to an \c {unsigned long long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

Q_ULLONG QByteArray::toULongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if ( base != 0 && (base < 2 || base > 36) ) {
	qWarning( "QByteArray::toULongLong: Invalid base %d", base );
	base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToUnsLongLong(*this, base, ok);
}

/*!
    Returns the string converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

long QByteArray::toLong(bool *ok, int base) const
{
    Q_LLONG v = toLongLong(ok, base);
    if (v < LONG_MIN || v > LONG_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return long(v);
}

/*!
    Returns the string converted to an \c {unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

ulong QByteArray::toULong(bool *ok, int base) const
{
    Q_ULLONG v = toULongLong(ok, base);
    if (v > ULONG_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return ulong(v);
}

/*!
    Returns the string converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \code
	QByteArray str("FF");
	bool ok;
	int hex = str.toInt(&ok, 16);     // hex == 255, ok == true
	int dec = str.toInt(&ok, 10);     // dec == 0, ok == false
    \endcode

    \sa number()
*/

int QByteArray::toInt(bool *ok, int base) const
{
    long v = toLongLong(ok, base);
    if (v < INT_MIN || v > INT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (int)v;
}

/*!
    Returns the string converted to an \c {unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

uint QByteArray::toUInt(bool *ok, int base) const
{
    ulong v = toULongLong(ok, base);
    if (v > UINT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (uint)v;
}

/*!
    Returns the string converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

short QByteArray::toShort(bool *ok, int base) const
{
    long v = toLongLong(ok, base);
    if (v < SHRT_MIN || v > SHRT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (short)v;
}

/*!
    Returns the string converted to an \c {unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

ushort QByteArray::toUShort(bool *ok, int base) const
{
    ulong v = toULongLong(ok, base);
    if (v > USHRT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (ushort)v;
}


/*!
    Returns the string converted to a \c double value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \code
	QByteArray string("1234.56");
	double a = string.toDouble();   // a == 1234.56
    \endcode

    \sa number()
*/

double QByteArray::toDouble(bool *ok) const
{
    return QLocalePrivate::bytearrayToDouble(*this, ok);
}

/*!
    Returns the string converted to a \c float value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \sa number()
*/

float QByteArray::toFloat(bool *ok) const
{
    return (float)toDouble(ok);
}


/*!
    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.

    \code
	QByteArray string;
	string = string.setNum(1234);     // string == "1234"
    \endcode
*/

QByteArray &QByteArray::setNum(Q_LLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QByteArray::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->longLongToString(n, -1, base).latin1();
    return *this;
}

/*!
    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

QByteArray &QByteArray::setNum(Q_ULLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QByteArray::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->unsLongLongToString(n, -1, base).latin1();
    return *this;
}

/*!
    \fn QByteArray &QByteArray::setNum(long n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QByteArray &QByteArray::setNum(ulong n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QByteArray &QByteArray::setNum(int n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QByteArray &QByteArray::setNum(uint n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QByteArray &QByteArray::setNum(short n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QByteArray &QByteArray::setNum(ushort n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/

QByteArray &QByteArray::setNum(double n, char f, int prec)
{
    QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
    uint flags = 0;

    if (qIsUpper(f))
    	flags = QLocalePrivate::CapitalEorX;
    f = qToLower(f);

    switch (f) {
	case 'f':
	    form = QLocalePrivate::DFDecimal;
	    break;
	case 'e':
	    form = QLocalePrivate::DFExponent;
	    break;
	case 'g':
	    form = QLocalePrivate::DFSignificantDigits;
	    break;
	default:
#if defined(QT_CHECK_RANGE)
	    qWarning( "QByteArray::setNum: Invalid format char '%c'", f );
#endif
	    break;
    }

    QLocale locale(QLocale::C);
    *this = locale.d->doubleToString(n, prec, form, -1, flags).latin1();
    return *this;
}

/*!
    \fn QByteArray &QByteArray::setNum(float n, char f, int prec)

    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/


/*!
    A convenience function that returns a string equivalent of the
    number \a n to base \a base, which is 10 by default and must be
    between 2 and 36.

    \code
	long a = 63;
	QByteArray str = QByteArray::number(a, 16);             // str == "3f"
	QByteArray str = QByteArray::number(a, 16).upper();     // str == "3F"
    \endcode

    \sa setNum()
*/
QByteArray QByteArray::number(long n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QByteArray QByteArray::number(ulong n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QByteArray QByteArray::number(int n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    A convenience factory function that returns a string
    representation of the number \a n to the base \a base, which is 10
    by default and must be between 2 and 36.

    \sa setNum()
*/
QByteArray QByteArray::number(uint n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QByteArray QByteArray::number( Q_LLONG n, int base )
{
    QByteArray s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QByteArray QByteArray::number( Q_ULLONG n, int base )
{
    QByteArray s;
    s.setNum( n, base );
    return s;
}


/*!
    \overload

    Argument \a n is formatted according to the \a f format specified,
    which is \c g by default, and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \code
    double d = 12.34;
    QByteArray ds = QByteArray("'E' format, precision 3, gives %1")
		    .arg(d, 0, 'E', 3);
    // ds == "1.234E+001"
    \endcode

    \sa setNum()
*/
QByteArray QByteArray::number(double n, char f, int prec)
{
    QByteArray s;
    s.setNum(n, f, prec);
    return s;
}


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
#include "qlist.h"
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

/*! \relates QByteArray

    Returns a duplicate string.

    Allocates space for a copy of \a src, copies it, and returns a
    pointer to the copy. If \a src is 0, it immediately returns 0.

    Ownership is passed to the caller, so the returned string must be
    deleted using \c delete[].
*/

char *qstrdup(const char *src)
{
    if (!src)
        return 0;
    char *dst = new char[strlen(src) + 1];
    return strcpy(dst, src);
}

/*! \fn char *qstrcpy(char *dst, const char *src)

    \relates QByteArray

    Copies all the characters up to and including the '\\0' from \a
    src into \a dst and returns a pointer to \a dst. If \a src is 0,
    it immediately returns 0.

    This function assumes that \a dst is large enough to hold the
    contents of \a src.

    \sa qstrncpy()
*/

/*! \relates QByteArray

    A safe strncpy() function.

    Copies at most \a len bytes from \a src (stopping at \a len or the
    terminating '\\0' whichever comes first) into \a dst and returns a
    pointer to \a dst. Guarantees that \a dst is '\\0'-terminated. If
    \a src or \a dst is 0, returns 0 immediately.

    This function assumes that \a dst is at least \a len characters
    long.

    \sa qstrcpy()
*/

char *qstrncpy(char *dst, const char *src, uint len)
{
    if (!src || !dst)
        return 0;
    strncpy(dst, src, len);
    if (len > 0)
        dst[len-1] = '\0';
    return dst;
}

/*! \fn uint qstrlen(const char *str);

    \relates QByteArray

    A safe strlen() function.

    Returns the number of characters that precede the terminating '\\0',
    or 0 if \a str is 0.
*/

/*! \fn int qstrcmp(const char *str1, const char *str2);

    \relates QByteArray

    A safe strcmp() function.

    Compares \a str1 and \a str2. Returns a negative value if \a str1
    is less than \a str2, 0 if \a str1 is equal to \a str2 or a
    positive value if \a str1 is greater than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrncmp(), qstricmp(), qstrnicmp(),
        \link #asciinotion Note on character comparisons \endlink
*/

/*! \fn int qstrncmp(const char *str1, const char *str2, uint len);

    \relates QByteArray

    A safe strncmp() function.

    Compares at most \a len bytes of \a str1 and \a str2.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstricmp(), qstrnicmp(),
        \link #asciinotion Note on character comparisons \endlink
*/

/*! \relates QByteArray

    A safe stricmp() function.

    Compares \a str1 and \a str2 ignoring the case of the characters.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp(), qstrnicmp(),
        \link #asciinotion Note on character comparisons \endlink
*/

int qstricmp(const char *str1, const char *str2)
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if (!s1 || !s2)
        return s1 ? 1 : (s2 ? -1 : 0);
    for (; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++)
        if (!c)                                // strings are equal
            break;
    return res;
}

/*! \relates QByteArray

    A safe strnicmp() function.

    Compares at most \a len bytes of \a str1 and \a str2 ignoring the
    case of the characters.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a str1
    is equal to \a str2 or a positive value if \a str1 is greater than \a
    str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp(), qstricmp(),
        \link #asciinotion Note on character comparisons \endlink
*/

int qstrnicmp(const char *str1, const char *str2, uint len)
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if (!s1 || !s2)
        return s1 ? 1 : (s2 ? -1 : 0);
    for (; len--; s1++, s2++) {
        if ((res = (c=tolower(*s1)) - tolower(*s2)))
            return res;
        if (!c)                                // strings are equal
            break;
    }
    return 0;
}


// the CRC table below is created by the following piece of code
#if 0
static void createCRC16Table()                        // build CRC16 lookup table
{
    register unsigned int i;
    register unsigned int j;
    unsigned short crc_tbl[16];
    unsigned int v0, v1, v2, v3;
    for (i = 0; i < 16; i++) {
        v0 = i & 1;
        v1 = (i >> 1) & 1;
        v2 = (i >> 2) & 1;
        v3 = (i >> 3) & 1;
        j = 0;
#undef SET_BIT
#define SET_BIT(x, b, v) (x) |= (v) << (b)
        SET_BIT(j,  0, v0);
        SET_BIT(j,  7, v0);
        SET_BIT(j, 12, v0);
        SET_BIT(j,  1, v1);
        SET_BIT(j,  8, v1);
        SET_BIT(j, 13, v1);
        SET_BIT(j,  2, v2);
        SET_BIT(j,  9, v2);
        SET_BIT(j, 14, v2);
        SET_BIT(j,  3, v3);
        SET_BIT(j, 10, v3);
        SET_BIT(j, 15, v3);
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

/*! \relates QByteArray

    Returns the CRC-16 checksum of the first \a len bytes of \a data.

    The checksum is independent of the byte order (endianness).
*/

Q_UINT16 qChecksum(const char *data, uint len)
{
    register Q_UINT16 crc = 0xffff;
    uchar c;
    uchar *p = (uchar *)data;
    while (len--) {
        c = *p++;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
        c >>= 4;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return ~crc & 0xffff;
}

/*! \fn QByteArray qCompress(const QByteArray& data, int compressionLevel)

    \relates QByteArray

    Compresses the \a data byte array and returns the compressed data
    in a new byte array.

    The \a compressionLevel parameter specifies how much compression
    should be used. Valid values are between 0 and 9, with 9
    corresponding to the greatest compression (i.e. smaller compressed
    data) at the cost of using a slower algorithm. Smaller values (8,
    7, ..., 1) provide successively less compression at slightly
    faster speeds. The value 0 corresponds to no compression at all.
    The default value is -1, which specifies zlib's default
    compression.

    \sa qUncompress()
*/

/*! \relates QByteArray

    \overload

    Compresses the first \a nbytes of \a data and returns the
    compressed data in a new byte array.
*/

#ifndef QT_NO_COMPRESS
QByteArray qCompress(const uchar* data, int nbytes, int compressionLevel)
{
    if (nbytes == 0) {
        return QByteArray(4, '\0');
    }
    if (!data) {
        qWarning("qCompress: Data is null");
        return QByteArray();
    }
    if (compressionLevel < -1 || compressionLevel > 9)
        compressionLevel = -1;

    ulong len = nbytes * 2;
    QByteArray bazip;
    int res;
    do {
        bazip.resize(len + 4);
        res = ::compress2((uchar*)bazip.data()+4, &len, (uchar*)data, nbytes, compressionLevel);

        switch (res) {
        case Z_OK:
            bazip.resize(len + 4);
            bazip[0] = (nbytes & 0xff000000) >> 24;
            bazip[1] = (nbytes & 0x00ff0000) >> 16;
            bazip[2] = (nbytes & 0x0000ff00) >> 8;
            bazip[3] = (nbytes & 0x000000ff);
            break;
        case Z_MEM_ERROR:
            qWarning("qCompress: Z_MEM_ERROR: Not enough memory");
            bazip.resize(0);
            break;
        case Z_BUF_ERROR:
            len *= 2;
            break;
        }
    } while (res == Z_BUF_ERROR);

    return bazip;
}
#endif

/*! \fn QByteArray qUncompress(const QByteArray& data)

    \relates QByteArray

    Uncompresses the \a data byte array and returns a new byte array
    with the uncompressed data.

    Returns an empty QByteArray if the input data was corrupt.

    This function will uncompress data compressed with qCompress()
    from this and any earlier Qt version, back to Qt 3.1 when this
    feature was added.

    \sa qCompress()
*/

/*! \relates QByteArray

    \overload

    Uncompresses the first \a nbytes of \a data and returns a new byte
    array with the uncompressed data.
*/

#ifndef QT_NO_COMPRESS
QByteArray qUncompress(const uchar* data, int nbytes)
{
    if (!data) {
        qWarning("qUncompress: data is NULL.");
        return QByteArray();
    }
    if (nbytes <= 4) {
        if (nbytes < 4 || (data[0]!=0 || data[1]!=0 || data[2]!=0 || data[3]!=0))
            qWarning("qUncompress: Input data is corrupted.");
        return QByteArray();
    }
    ulong expectedSize = (data[0] << 24) | (data[1] << 16) |
                       (data[2] <<  8) | (data[3]      );
    ulong len = qMax(expectedSize,  1ul);
    QByteArray baunzip;
    int res;
    do {
        baunzip.resize(len);
        res = ::uncompress((uchar*)baunzip.data(), &len,
                            (uchar*)data+4, nbytes-4);

        switch (res) {
        case Z_OK:
            if ((int)len != baunzip.size())
                baunzip.resize(len);
            break;
        case Z_MEM_ERROR:
            qWarning("qUncompress: Z_MEM_ERROR: Not enough memory.");
            break;
        case Z_BUF_ERROR:
            len *= 2;
            break;
        case Z_DATA_ERROR:
            qWarning("qUncompress: Z_DATA_ERROR: Input data is corrupted.");
            break;
        }
    } while (res == Z_BUF_ERROR);

    if (res != Z_OK)
        baunzip = QByteArray();

    return baunzip;
}
#endif

static inline bool qIsUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static inline bool qIsDigit(char c)
{
    return c >= '0' && c <= '9';
}

static inline char qToLower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    else
        return c;
}

Q_CORE_EXPORT QByteArray::Data QByteArray::shared_null = {Q_ATOMIC_INIT(1), 0, 0, shared_null.array, {0} };
QByteArray::Data QByteArray::shared_empty = { Q_ATOMIC_INIT(1), 0, 0, shared_empty.array, {0} };

/*! \class QByteArray
    \brief The QByteArray class provides an array of bytes.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass
    \reentrant

    QByteArray can be used to store both raw bytes (including '\\0's)
    and traditional 8-bit '\\0'-terminated strings. Using QByteArray
    is much more convenient than using \c{const char *}. Behind the
    scenes, it always ensures that the data is followed by a '\\0'
    terminator, and uses \l{implicit sharing} (copy-on-write) to
    reduce memory usage and avoid needless copying of data.

    In addition to QByteArray, Qt also provides the QString class to
    store string data. For most purposes, QString is the class you
    want to use. It stores 16-bit Unicode characters, making it easy
    to store non-ASCII/non-Latin-1 characters in your application.
    Furthermore, QString is used throughout in the Qt API. The two
    main cases where QByteArray is appropriate are when you need to
    store raw binary data, and when memory conservation is critical
    (e.g. with Qt/Embedded).

    One way to initialize a QByteArray is simply to pass a \c{const
    char *} to its constructor. For example, the following code
    creates a byte array of size 5 containing the data "Hello":

    \code
        QByteArray ba("Hello");
    \endcode

    Although the size() is 5, the byte array also maintains an extra
    '\\0' character at the end so that if a function is used that
    asks for a pointer to the underlying data (e.g. a call to
    data()), the data pointed to is guaranteed to be
    '\\0'-terminated.

    QByteArray makes a deep copy of the \c{const char *} data, so you
    can modify it later without experiencing side effects. (If for
    performance reasons you don't want to take a deep copy of the
    character data, use QConstByteArray instead.)

    Another approach is to set the size of the array using resize()
    and to initialize the data byte per byte. QByteArray uses 0-based
    indexes, just like C++ arrays. To access the byte at a particular
    index position, you can use operator[](). On non-const byte
    arrays, operator[]() returns a reference to a byte that can be
    used on the left side of an assignment. For example:

    \code
        QByteArray ba;
        ba.resize(5);
        ba[0] = 0x3c;
        ba[1] = 0xb8;
        ba[2] = 0x64;
        ba[3] = 0x18;
        ba[4] = 0xca;
    \endcode

    For read-only access, an alternative syntax is to use at():

    \code
        for (int i = 0; i < ba.size(); ++i) {
            if (ba.at(i) >= 'a' && ba.at(i) <= 'f')
                cout << "Found character in range [a-f]" << endl;
        }
    \endcode

    at() can be faster than operator[](), because it never causes a
    \l{deep copy} to occur.

    To extract many bytes at a time, use left(), right(), or mid().

    A QByteArray can embed '\\0' bytes. The size() function always
    returns the size of the whole array, including embedded '\\0'
    bytes. If you want to obtain the length of the data up to and
    excluding the first '\\0' character, call qstrlen() on the byte
    array.

    After a call to resize(), newly allocated bytes have undefined
    values. To set all the bytes to a particular value, call fill().

    To obtain a pointer to the actual character data, call data() or
    constData(). These functions return a pointer to the beginning of
    the data. The pointer is guaranteed to remain valid until a
    non-const function is called on the QByteArray. It is also
    guaranteed that the data ends with a '\\0' byte. This '\\0' byte
    is automatically provided by QByteArray and is not counted in
    size().

    QByteArray provides the following basic functions for modifying
    the byte data: append(), prepend(), insert(), replace(), and
    remove(). For example:

    \code
        QByteArray x("and");
        x.prepend("rock ");         // x == "rock and"
        x.append(" roll");          // x == "rock and roll"
        x.replace(5, 3, "&");       // x == "rock & roll"
    \endcode

    The replace() and remove() functions' first two arguments are the
    position from which to start erasing and the number of bytes that
    should be erased.

    If you are building a QByteArray gradually and know in advance
    approximately how many bytes the QByteArray will contain,
    you can call reserve(), asking QByteArray to preallocate a
    certain amount of memory. You can also call capacity() to find
    out how much memory QByteArray actually allocated.

    A frequent requirement is to remove whitespace characters from a
    byte array ('\\n', '\\t', ' ', etc.). If you want to remove
    whitespace from both ends of a QByteArray, use trimmed(). If you
    want to remove whitespace from both ends and replace multiple
    consecutive whitespaces with a single space character within the
    byte array, use simplified().

    If you want to find all occurrences of a particular character or
    substring in a QByteArray, use indexOf() or lastIndexOf(). The
    former searches forward starting from a given index position, the
    latter searches backward. Both return the index position of the
    character or substring if they find it; otherwise, they return -1.
    For example, here's a typical loop that finds all occurrences of a
    particular substring:

    \code
        QByteArray ba("We must be <b>bold</b>, very <b>bold</b>");
        int j = 0;
        while ((j = ba.indexOf("<b>", j)) != -1) {
            cout << "Found <b> tag at index position " << j << endl;
            ++j;
        }
    \endcode

    If you simply want to check whether a QByteArray contains a
    particular character or substring, use contains(). If you want to
    find out how many times a particular character or substring
    occurs in the byte array, use count(). If you want to replace all
    occurrences of a particular value with another, use one of the
    two-parameter replace() overloads.

    For historical reasons, QByteArray distinguishes between a null
    byte array and an empty byte array. A \e null byte array is a
    byte array that is initialized using QByteArray's default
    constructor or by passing (const char *)0 to the constructor. An
    \e empty byte array is any byte array with size 0. A null byte
    array is always empty, but an empty byte array isn't necessarily
    null:

    \code
        QByteArray().isNull();          // returns true
        QByteArray().isEmpty();         // returns true

        QByteArray("").isNull();        // returns false
        QByteArray("").isEmpty();       // returns true

        QByteArray("abc").isNull();     // returns false
        QByteArray("abc").isEmpty();    // returns false
    \endcode

    All functions except isNull() treat null byte arrays the same as
    empty byte arrays. For example, data() returns a pointer to a
    '\\0' character for a null byte array (\e not a null pointer),
    and QByteArray() compares equal to QByteArray(""). We recommend
    that you always use isEmpty() and avoid isNull().

    \sa QConstByteArray, QBitArray, QString
*/

/*! \fn QByteArray::iterator QByteArray::begin()

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::begin() const

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::constBegin() const

    \internal
*/

/*! \fn QByteArray::iterator QByteArray::end()

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::end() const

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::constEnd() const

    \internal
*/

/*! \fn void QByteArray::push_back(const QByteArray &other)

    This function is provided for STL compatibility. It is equivalent
    to append(\a other).
*/

/*! \fn void QByteArray::push_back(const char *str)

    \overload

    Same as append(\a str).
*/

/*! \fn void QByteArray::push_back(char ch)

    \overload

    Same as append(\a ch).
*/

/*! \fn void QByteArray::push_front(const QByteArray &other)

    This function is provided for STL compatibility. It is equivalent
    to prepend(\a other).
*/

/*! \fn void QByteArray::push_front(const char *str)

    \overload

    Same as prepend(\a str).
*/

/*! \fn void QByteArray::push_front(char ch)

    \overload

    Same as prepend(\a ch).
*/

/*! \fn QByteArray::QByteArray(const QByteArray &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QByteArray is
    \l{implicitly shared}. This makes returning a QByteArray from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*! \fn QByteArray::~QByteArray()

    Destroys the byte array.
*/

/*! \fn QByteArray &QByteArray::operator=(const QByteArray &other)

    Assigns \a other to this byte array and returns a reference to
    this byte array.
*/

/*!
    \overload

    Assigns \a str to this byte array.
*/

QByteArray &QByteArray::operator=(const char *str)
{
    Data *x;
    if (!str) {
        x = &shared_null;
    } else if (!*str) {
        x = &shared_empty;
    } else {
        int len = strlen(str);
        if (d->ref != 1 || len > d->alloc || (len < d->size && len < d->alloc >> 1))
            realloc(len);
        x = d;
        memcpy(x->data, str, len+1); // include null terminator
        x->size = len;
    }
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
         qFree(x);
    return *this;
}

/*! \fn int QByteArray::size() const

    Returns the number of bytes in this byte array.

    The last byte in the byte array is at position size() - 1. In
    addition, QByteArray ensures that the byte at position size() is
    always '\\0', so that you can use the return value of data() and
    constData() as arguments to functions that expect
    '\\0'-terminated strings.

    Example:
    \code
        QByteArray ba("Hello");
        int n = ba.size();          // n == 5
        ba.data()[0];               // returns 'H'
        ba.data()[4];               // returns 'o'
        ba.data()[5];               // returns '\0'
    \endcode

    \sa isEmpty(), resize()
*/

/*! \fn bool QByteArray::isEmpty() const

    Returns true if the byte array has size 0; otherwise returns false.

    Example:
    \code
        QByteArray().isEmpty();         // returns true
        QByteArray("").isEmpty();       // returns true
        QByteArray("abc").isEmpty();    // returns false
    \endcode

    \sa size()
*/

/*! \fn int QByteArray::capacity() const

    Returns the maximum number of bytes that can be stored in the
    byte array without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function. If you want to know how many
    bytes are in the byte array, call size().

    \sa reserve(), squeeze()
*/

/*! \fn void QByteArray::reserve(int size)

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

    Releases any memory not required to store the array's data.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function.

    \sa reserve(), capacity()
*/

/*! \fn QByteArray::operator const char *() const

    Returns a pointer to the data stored in the byte array. The
    pointer can be used to access the bytes that compose the array.
    The data is '\\0'-terminated. The pointer remains valid as long
    as the array isn't reallocated.

    This operator is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa constData()
*/

/*! \fn QByteArray::operator const void *() const

    Returns a void pointer to the data.

    This operator is mostly useful to pass a byte array to a function
    that accepts a void *.

    \sa constData()
*/

/*! \fn char *QByteArray::data()

    Returns a pointer to the data stored in the byte array. The
    pointer can be used to access and modify the bytes that compose
    the array. The data is '\\0'-terminated.

    Example:
    \code
        QByteArray ba("Hello world");
        char *data = ba.data();
        while (*data) {
            cout << "[" + *data + "]" << endl;
            ++data;
        }
    \endcode

    The pointer remains valid as long as the byte array isn't
    reallocated.

    This function is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa constData(), operator[]()
*/

/*! \fn const char *QByteArray::data() const

    \overload
*/

/*! \fn const char *QByteArray::constData() const

    Returns a pointer to the data stored in the byte array. The
    pointer can be used to access the bytes that compose the array.
    The data is '\\0'-terminated. The pointer remains valid as long
    as the byte array isn't reallocated.

    This function is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa data(), operator[]()
*/

/*! \fn void QByteArray::detach()

    \internal
*/

/*! \fn bool QByteArray::isDetached() const

    \internal
*/

/*! \fn char QByteArray::at(int i) const

    Returns the character at index position \a i in the byte array.

    \a i must be a valid index position in the byte array (i.e., 0 <=
    \a i < size()).

    \sa operator[]()
*/

/*! \fn QByteRef QByteArray::operator[](int i)

    Returns the byte at index position \a i as a modifiable reference.

    If an assignment is made beyond the end of the byte array, the
    array is extended with resize() before the assignment takes
    place.

    Example:
    \code
        QByteArray ba;
        for (int i = 0; i < 10; ++i)
            ba[i] = 'A' + i;
        // ba == "ABCDEFGHIJ"
    \endcode

    The return value is of type QByteRef, a helper class for
    QByteArray. When you get an object of type QByteRef, you can use
    it as if it were a char &. If you assign to it, the assignment
    will apply to the character in the QByteArray from which you got
    the reference.

    \sa at()
*/

/*! \fn char QByteArray::operator[](int i) const

    \overload

    Same as at(\a i).
*/

/*! \fn QByteRef QByteArray::operator[](uint i)

    \overload
*/

/*! \fn char QByteArray::operator[](uint i) const

    \overload
*/

/*! \fn QBool QByteArray::contains(const QByteArray &ba) const

    Returns true if the byte array contains an occurrence of the byte
    array \a ba; otherwise returns false.

    \sa indexOf(), count()
*/

/*! \fn QBool QByteArray::contains(const char *str) const

    \overload

    Returns true if the byte array contains the string \a str;
    otherwise returns false.
*/

/*! \fn QBool QByteArray::contains(char ch) const

    \overload

    Returns true if the byte array contains the character \a ch;
    otherwise returns false.
*/

/*! \fn void QByteArray::truncate(int pos)

    Truncates the byte array at index position \a pos.

    If \a pos is beyond the end of the array, nothing happens.

    Example:
    \code
        QByteArray ba("Stockholm");
        ba.truncate(5);             // ba == "Stock"
    \endcode

    \sa resize(), left()
*/

/*! \fn QByteArray &QByteArray::operator+=(const QByteArray &ba)

    Appends the byte array \a ba onto the end of this byte array and
    returns a reference to this byte array.

    Example:
    \code
        QByteArray x("free");
        QByteArray y("dom");
        x += y;
        // x == "freedom"
    \endcode

    This operation is typically very fast (\l{constant time}),
    because QByteArray preallocates extra space at the end of the
    character data so it can grow without reallocating the entire
    data each time.

    \sa append(), prepend()
*/

/*! \fn QByteArray &QByteArray::operator+=(const QString &str)

    \overload

    Appends the string \a str onto the end of this byte array and
    returns a reference to this byte array. The Unicode data is
    converted into 8-bit characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    operator can lead to loss of information. You can disable this
    operator by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::operator+=(const char *str)

    \overload

    Appends the string \a str onto the end of this byte array and
    returns a reference to this byte array.
*/

/*! \fn QByteArray &QByteArray::operator+=(char ch)

    \overload

    Appends the character \a ch onto the end of this byte array and
    returns a reference to this byte array.
*/

/*! \fn int QByteArray::length() const

    Same as size().
*/

/*! \fn bool QByteArray::isNull() const

    Returns true if this byte array is null; otherwise returns false.

    Example:
    \code
        QByteArray().isNull();          // returns true
        QByteArray("").isNull();        // returns false
        QByteArray("abc").isNull();     // returns false
    \endcode

    Qt makes a distinction between null byte arrays and empty byte
    arrays for historical reasons. For most applications, what
    matters is whether or not a byte array contains any data,
    and this can be determined using isEmpty().

    \sa isEmpty()
*/

/*! \fn QByteArray::QByteArray()

    Constructs an empty byte array.

    \sa isEmpty()
*/

QByteArray::QByteArray() : d(&shared_null)
{
    ++d->ref;
}

/*! \fn QByteArray::QByteArray(const char *str)

    Constructs a byte array initialized with the string \a str.

    QByteArray makes a deep copy of the string data.
*/

QByteArray::QByteArray(const char *str)
{
    if (!str) {
        d = &shared_null;
    } else if (!*str) {
        d = &shared_empty;
    } else {
        int len = strlen(str);
        d = (Data *)qMalloc(sizeof(Data)+len);
        if (!d) {
            d = &shared_null;
        } else {
            d->ref = 0;
            d->alloc = d->size = len;
            d->data = d->array;
            memcpy(d->array, str, len+1); // include null terminator
        }
    }
    ++d->ref;
}

/*!
    Constructs a byte array containing the first \a size bytes of
    string \a str.

    If \a str is 0, a null byte array is constructed.

    QByteArray makes a deep copy of the string data.
*/

QByteArray::QByteArray(const char *str, int size)
{
    if (!str) {
        d = &shared_null;
    } else if (size <= 0) {
        d = &shared_empty;
    } else {
        d = (Data *)qMalloc(sizeof(Data) + size);
        if (!d) {
            d = &shared_null;
        } else {
            d->ref = 0;
            d->alloc = d->size = size;
            d->data = d->array;
            memcpy(d->array, str, size);
            d->array[size] = '\0';
        }
    }
    ++d->ref;
}

/*!
    Constructs a byte array of size \a size with every byte set to
    character \a ch.

    \sa fill()
*/

QByteArray::QByteArray(int size, char ch)
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
            memset(d->array, ch, size);
        }
    }
    ++d->ref;
}

/*!
    Sets the size of the byte array to \a size bytes.

    If \a size is greater than the current size, the byte array is
    extended to make it \a size bytes with the extra bytes added to
    the end. The new bytes are uninitialized.

    If \a size is less than the current size, bytes are removed from
    the end.

    \sa size()
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
    Sets every byte in the byte array to character \a ch. If \a size
    is different from -1 (the default), the byte array is resized to
    size \a size beforehand.

    Example:
    \code
        QByteArray ba("Istambul");
        ba.fill("o");
        // ba == "oooooooo"

        ba.fill("X", 2);
        // ba == "XX"
    \endcode

    \sa resize()
*/

QByteArray &QByteArray::fill(char ch, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size)
        memset(d->data, ch, d->size);
    return *this;
}

void QByteArray::realloc(int alloc)
{
    if (d->ref != 1 || d->data != d->array) {
        Data *x = static_cast<Data *>(qMalloc(sizeof(Data) + alloc));
        if (!x)
            return;
        ::memcpy(x->array, d->data, qMin(alloc, d->alloc) + 1);
        x->ref = 1;
        x->size = d->size;
        x->data = x->array;
        x = qAtomicSetPtr(&d, x);
        if (!--x->ref)
            qFree(x);
    } else {
        Data *x = static_cast<Data *>(qRealloc(d, sizeof(Data) + alloc));
        if (!x)
            return;
        x->data = x->array;
        d = x;
    }
    d->alloc = alloc;
}

void QByteArray::expand(int i)
{
    resize(qMax(i + 1, d->size));
}

/*!
    Prepends the byte array \a ba to this byte array and returns a
    reference to this byte array.

    Example:
    \code
        QByteArray x("ship");
        QByteArray y("air");
        x.prepend(y);
        // x == "airship"
    \endcode

    This is the same as insert(0, \a ba).

    \sa append(), insert()
*/

QByteArray &QByteArray::prepend(const QByteArray &ba)
{
    if (d == &shared_null || d == &shared_empty) {
        *this = ba;
    } else if (ba.d != &shared_null) {
        QByteArray tmp = *this;
        *this = ba;
        append(tmp);
    }
    return *this;
}

/*!
    \overload

    Prepends the string \a str to this byte array.
*/

QByteArray &QByteArray::prepend(const char *str)
{
    if (str) {
        int len = strlen(str);
        if (d->ref != 1 || d->size + len > d->alloc)
            realloc(qAllocMore(d->size + len, sizeof(Data)));
        memmove(d->data+len, d->data, d->size);
        memcpy(d->data, str, len);
        d->size += len;
        d->data[d->size] = '\0';
    }
    return *this;
}

/*!
    \overload

    Prepends the character \a ch to this byte array.
*/

QByteArray &QByteArray::prepend(char ch)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
        realloc(qAllocMore(d->size + 1, sizeof(Data)));
    memmove(d->data+1, d->data, d->size);
    d->data[0] = ch;
    ++d->size;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    Appends the byte array \a ba onto the end of this byte array.

    Example:
    \code
        QByteArray x("free");
        QByteArray y("dom");
        x.append(y);
        // x == "freedom"
    \endcode

    This is the same as insert(size(), \a ba).

    This operation is typically very fast (\l{constant time}),
    because QByteArray preallocates extra space at the end of the
    character data so it can grow without reallocating the entire
    data each time.

    \sa operator+=(), prepend(), insert()
*/

QByteArray &QByteArray::append(const QByteArray &ba)
{
    if (d == &shared_null || d == &shared_empty) {
        *this = ba;
    } else if (ba.d != &shared_null) {
        if (d->ref != 1 || d->size + ba.d->size > d->alloc)
            realloc(qAllocMore(d->size + ba.d->size, sizeof(Data)));
        memcpy(d->data + d->size, ba.d->data, ba.d->size + 1); // include null terminator
        d->size += ba.d->size;
    }
    return *this;
}

/*! \fn QByteArray &QByteArray::append(const QString &str)

    \overload

    Appends the string \a str to this byte array. The Unicode data is
    converted into 8-bit characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*!
    \overload

    Appends the string \a str to this byte array.
*/

QByteArray& QByteArray::append(const char *str)
{
    if (str) {
        int len = strlen(str);
        if (d->ref != 1 || d->size + len > d->alloc)
            realloc(qAllocMore(d->size + len, sizeof(Data)));
        memcpy(d->data + d->size, str, len + 1); // include null terminator
        d->size += len;
    }
    return *this;
}

/*!
    \overload

    Appends the character \a ch to this byte array.
*/

QByteArray& QByteArray::append(char ch)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
        realloc(qAllocMore(d->size + 1, sizeof(Data)));
    d->data[d->size++] = ch;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    Inserts the byte array \a ba at index position \a i and returns a
    reference to this byte array.

    Example:
    \code
        QByteArray ba("Meal");
        ba.insert(1, QByteArray("ontr"));
        // ba == "Montreal"
    \endcode

    \sa append(), prepend(), replace(), remove()
*/

QByteArray &QByteArray::insert(int i, const QByteArray &ba)
{
    if (i < 0 || ba.d->size == 0)
        return *this;
    expand(qMax(d->size, i) + ba.d->size - 1);
    ::memmove(d->data + i + ba.d->size, d->data + i, (d->size - i - ba.d->size) * sizeof(char));
    memcpy(d->data + i, ba.d->data, ba.d->size*sizeof(char));
    return *this;
}

/*!
    \fn QByteArray &QByteArray::insert(int i, const QString &str)

    \overload

    Inserts the string \a str at index position \a i in the byte
    array. The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If \a i is greater than size(), the array is first extended using
    resize().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*!
    \overload

    Inserts the string \a str at position \a i in the byte array.

    If \a i is greater than size(), the array is first extended using
    resize().
*/

QByteArray &QByteArray::insert(int i, const char *str)
{
    if (!str || !*str || i < 0)
        return *this;
    int l = strlen(str);
    int oldsize = d->size;
    expand(qMax(d->size, i) + l - 1);
    if (i > oldsize)
        memset(d->data + oldsize, 0x20, i - oldsize);
    else
        ::memmove(d->data + i + l, d->data + i, (d->size - i - l)*sizeof(char));
    memcpy(d->data + i, str, l * sizeof(char));
    return *this;
}

/*!
    \overload

    Inserts character \a ch at index position \a i in the byte array.
    If \a i is greater than size(), the array is first extended using
    resize().
*/

QByteArray &QByteArray::insert(int i, char ch)
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
        ::memmove(d->data + i + 1, d->data + i, (d->size - i) * sizeof(char));
    d->data[i] = ch;
    return *this;
}

/*!
    Removes \a len bytes from the array, starting at index position \a
    pos, and returns a reference to the array.

    If \a pos is out of range, nothing happens. If \a pos is valid,
    but \a pos + \a len is larger than the size of the array, the
    array is truncated at position \a pos.

    Example:
    \code
        QByteArray ba("Montreal");
        ba.remove(1, 4);
        // ba == "Meal"
    \endcode

    \sa insert(), replace()
*/

QByteArray &QByteArray::remove(int pos, int len)
{
    if (len == 0 || pos >= d->size)
        return *this;
    detach();
    if (pos + len >= d->size) {
        resize(pos);
    } else {
        memmove(d->data + pos, d->data + pos + len, d->size - pos - len);
        resize(d->size - len);
    }
    return *this;
}

/*!
    Replaces \a len bytes from index position \a pos with the byte
    array \a after, and returns a reference to this byte array.

    Example:
    \code
        QByteArray x("Say yes!");
        QByteArray y("no");
        x.replace(4, 3, y);
        // x == "Say no!"
    \endcode

    \sa insert(), remove()
*/

QByteArray &QByteArray::replace(int pos, int len, const QByteArray &after)
{
    remove(pos, len);
    return insert(pos, after);
}

/*! \fn QByteArray &QByteArray::replace(int pos, int len, const char *after)

    \overload
*/

/*!
    \overload

    Replaces every occurrence of the byte array \a before with the
    byte array \a after.

    Example:
    \code
        QByteArray ba("colour behaviour flavour neighbour");
        ba.replace(QByteArray("ou"), QByteArray("o"));
        // ba == "color behavior flavor neighbor"
    \endcode
*/

QByteArray &QByteArray::replace(const QByteArray &before, const QByteArray &after)
{
    if (isNull() || before == after)
        return *this;

    int index = 0;
    const int bl = before.d->size;
    const int al = after.d->size;
    int len = d->size;
    char *d = data();

    if (bl == al) {
        if (bl) {
            while((index = indexOf(before, index)) != -1) {
                memcpy(d+index, after, al);
                index += bl;
            }
        }
    } else if (al < bl) {
        uint to = 0;
        uint movestart = 0;
        uint num = 0;
        while((index = indexOf(before, index)) != -1) {
            if (num) {
                int msize = index - movestart;
                if (msize > 0) {
                    memmove(d + to, d + movestart, msize);
                    to += msize;
                }
            } else {
                to = index;
            }
            if (al) {
                memcpy(d + to, after, al);
                to += al;
            }
            index += bl;
            movestart = index;
            num++;
        }
        if (num) {
            int msize = len - movestart;
            if (msize > 0)
                memmove(d + to, d + movestart, msize);
            resize(len - num*(bl-al));
        }
    } else {
        // the most complex case. We don't want to loose performance by doing repeated
        // copies and reallocs of the string.
        while(index != -1) {
            uint indices[4096];
            uint pos = 0;
            while(pos < 4095) {
                index = indexOf(before, index);
                if (index == -1)
                    break;
                indices[pos++] = index;
                index += bl;
                // avoid infinite loop
                if (!bl)
                    index++;
            }
            if (!pos)
                break;

            // we have a table of replacement positions, use them for fast replacing
            int adjust = pos*(al-bl);
            // index has to be adjusted in case we get back into the loop above.
            if (index != -1)
                index += adjust;
            int newlen = len + adjust;
            int moveend = len;
            if (newlen > len) {
                resize(newlen);
                len = newlen;
            }
            d = this->d->data;

            while(pos) {
                pos--;
                int movestart = indices[pos] + bl;
                int insertstart = indices[pos] + pos*(al-bl);
                int moveto = insertstart + al;
                memmove(d + moveto, d + movestart, (moveend - movestart));
                if (after.size())
                    memcpy(d + insertstart, after, al);
                moveend = movestart - bl;
            }
        }
    }
    return *this;
}

/*! \fn QByteArray &QByteArray::replace(const QByteArray &before, const char *after)
    \overload

    Replaces every occurrence of the byte array \a before with the
    string \a after.
*/

/*! \fn QByteArray &QByteArray::replace(const QString &before, const QByteArray &after)

    \overload

    Replaces every occurrence of the string \a before with the byte
    array \a after. The Unicode data is converted into 8-bit
    characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::replace(const QString &before, const char *after)
    \overload

    Replaces every occurrence of the string \a before with the string
    \a after.
*/

/*! \fn QByteArray &QByteArray::replace(const char *before, const char *after)

    \overload

    Replaces every occurrence of the string \a before with the string
    \a after.
*/

/*!
    \overload

    Replaces every occurrence of the character \a before with the
    byte array \a after.
*/

QByteArray &QByteArray::replace(char before, const QByteArray &after)
{
    char b[2] = { before, '\0' };
    QConstByteArray cb(b, 1);
    return replace(cb, after);
}

/*! \fn QByteArray &QByteArray::replace(char before, const QString &after)

    \overload

    Replaces every occurrence of the character \a before with the
    string \a after. The Unicode data is converted into 8-bit
    characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::replace(char before, const char *after)

    \overload

    Replaces every occurrence of the character \a before with the
    string \a after.
*/

/*!
    \overload

    Replaces every occurrence of the character \a before with the
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
    Splits the byte array into subarrays wherever \a sep occurs, and
    returns the list of those arrays. If \a sep does not match
    anywhere in the byte array, split() returns a single-element list
    containing this byte array.
*/

QList<QByteArray> QByteArray::split(char sep) const
{
    QList<QByteArray> list;
    int start = 0;
    int end;
    while ((end = indexOf(sep, start)) != -1) {
        list.append(mid(start, end - start));
        start = end + 1;
    }
    list.append(mid(start));
    return list;
}

#define REHASH(a) \
    if (ol_minus_1 < sizeof(uint) * CHAR_BIT) \
        hashHaystack -= (a) << ol_minus_1; \
    hashHaystack <<= 1

/*!
    Returns the index position of the first occurrence of the byte
    array \a ba in this byte array, searching forward from index
    position \a from. Returns -1 if \a ba could not be found.

    Example:
    \code
        QByteArray x("sticky question");
        QByteArray y("sti");
        x.indexOf(y);               // returns 0
        x.indexOf(y, 1);            // returns 10
        x.indexOf(y, 10);           // returns 10
        x.indexOf(y, 11);           // returns -1
    \endcode

    \sa lastIndexOf(), contains(), count()
*/

int QByteArray::indexOf(const QByteArray &ba, int from) const
{
    const int l = d->size;
    const int ol = ba.d->size;
    if (from > d->size || ol + from > l)
        return -1;
    if (ol == 0)
        return from;
    if (ol == 1)
        return indexOf(*ba.d->data, from);
    const char *needle = ba.d->data;
    const char *haystack = d->data + from;
    const char *end = d->data + (l - ol);
    const uint ol_minus_1 = ol - 1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + needle[idx]);
        hashHaystack = ((hashHaystack<<1) + haystack[idx]);
    }
    hashHaystack -= *(haystack + ol_minus_1);

    while (haystack <= end) {
        hashHaystack += *(haystack + ol_minus_1);
        if (hashHaystack == hashNeedle  && *needle == *haystack
             && strncmp(needle, haystack, ol) == 0)
            return haystack - d->data;

        REHASH(*haystack);
        ++haystack;
    }
    return -1;
}

/*! \fn int QByteArray::indexOf(const QString &str, int from) const

    \overload

    Returns the index position of the first occurrence of the string
    \a str in the byte array, searching forward from index position
    \a from. Returns -1 if \a str could not be found.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn int QByteArray::indexOf(const char *str, int from) const

    \overload

    Returns the index position of the first occurrence of the string
    \a str in the byte array, searching forward from index position \a
    from. Returns -1 if \a str could not be found.
*/

/*!
    \overload

    Returns the index position of the first occurrence of the
    character \a ch in the byte array, searching forward from index
    position \a from. Returns -1 if \a ch could not be found.

    Example:
    \code
        QByteArray ba("ABCBA");
        ba.indexOf("B");            // returns 1
        ba.indexOf("B", 1);         // returns 1
        ba.indexOf("B", 2);         // returns 3
        ba.indexOf("X");            // returns -1
    \endcode

    \sa lastIndexOf(), contains()
*/

int QByteArray::indexOf(char ch, int from) const
{
    if (from < 0)
        from = qMax(from + d->size, 0);
    if (from < d->size) {
        const char *n = d->data + from - 1;
        const char *e = d->data + d->size;
        while (++n != e)
        if (*n == ch)
            return  n - d->data;
    }
    return -1;
}

/*!
    Returns the index position of the last occurrence of the byte
    array \a ba in this byte array, searching backward from index
    position \a from. If \a from is -1 (the default), the search
    starts at the last byte. Returns -1 if \a ba could not be found.

    Example:
    \code
        QByteArray x("crazy azimuths");
        QByteArray y("azy");
        x.lastIndexOf(y);           // returns 6
        x.lastIndexOf(y, 6);        // returns 6
        x.lastIndexOf(y, 5);        // returns 2
        x.lastIndexOf(y, 1);        // returns -1
    \endcode

    \sa indexOf(), contains(), count()
*/

int QByteArray::lastIndexOf(const QByteArray &ba, int from) const
{
    const int ol = ba.d->size;
    const int l = d->size;
    int delta = l - ol;
    if (from < 0)
        from = delta;
    if (from < 0 || from > l)
        return -1;
    if (from > delta)
        from = delta;
    if (ol == 1)
        return lastIndexOf(*ba.d->data, from);

    const char *needle = ba.d->data;
    const char *haystack = d->data + from;
    const char *end = d->data;
    const uint ol_minus_1 = ol - 1;
    const char *n = needle + ol_minus_1;
    const char *h = haystack + ol_minus_1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + *(n-idx));
        hashHaystack = ((hashHaystack<<1) + *(h-idx));
    }
    hashHaystack -= *haystack;
    while (haystack >= end) {
        hashHaystack += *haystack;
        if (hashHaystack == hashNeedle  && strncmp(needle, haystack, ol) == 0)
            return haystack-d->data;
        --haystack;
        REHASH(*(haystack + ol));
    }
    return -1;

}

/*! \fn int QByteArray::lastIndexOf(const QString &str, int from) const

    \overload

    Returns the index position of the last occurrence of the string \a
    str in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a str could not be found.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::ascii() (or
    QString::latin1() or QString::utf8() or QString::local8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn int QByteArray::lastIndexOf(const char *str, int from) const
    \overload

    Returns the index position of the last occurrence of the string \a
    str in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a str could not be found.
*/

/*!
    \overload

    Returns the index position of the last occurrence of character \a
    ch in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a ch could not be found.

    Example:
    \code
        QByteArray ba("ABCBA");
        ba.lastIndexOf("B");        // returns 3
        ba.lastIndexOf("B", 3);     // returns 3
        ba.lastIndexOf("B", 2);     // returns 1
        ba.lastIndexOf("X");        // returns -1
    \endcode

    \sa indexOf(), contains()
*/

int QByteArray::lastIndexOf(char ch, int from) const
{
    if (from < 0)
        from += d->size;
    else if (from > d->size)
        from = d->size-1;
    if (from >= 0) {
        const char *b = d->data;
        const char *n = d->data + from + 1;
        while (n-- != b)
            if (*n == ch)
                return  n - b;
    }
    return -1;
}

/*!
    Returns the number of (potentially overlapping) occurrences of
    byte array \a ba in this byte array.

    \sa contains(), indexOf()
*/

int QByteArray::count(const QByteArray &ba) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(ba, i + 1)) != -1)
        ++num;
    return num;
}

/*!
    \overload

    Returns the number of (potentially overlapping) occurrences of
    string \a str in the byte array.
*/

int QByteArray::count(const char *str) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(str, i + 1)) != -1)
        ++num;
    return num;
}

/*!
    \overload

    Returns the number of occurrences of character \a ch in the byte
    array.

    \sa contains(), indexOf()
*/

int QByteArray::count(char ch) const
{
    int num = 0;
    const char *i = d->data + d->size;
    const char *b = d->data;
    while (i != b)
        if (*--i == ch)
            ++num;
    return num;
}

/*! \fn int QByteArray::count() const

    \overload

    Same as size().
*/

/*!
    Returns true if this byte array starts with byte array \a ba;
    otherwise returns false.

    Example:
    \code
        QByteArray url("ftp://ftp.trolltech.com/");
        if (url.startsWith("ftp:"))
            ...
    \endcode

    \sa endsWith(), left()
*/
bool QByteArray::startsWith(const QByteArray &ba) const
{
    if (d == ba.d || ba.d->size == 0)
        return true;
    if (d->size < ba.d->size)
        return false;
    return memcmp(d->data, ba.d->data, ba.d->size) == 0;
}

/*! \overload

    Returns true if this byte array starts with string \a str;
    otherwise returns false.
*/
bool QByteArray::startsWith(const char *str) const
{
    if (!str || !*str)
        return true;
    int len = qstrlen(str);
    if (d->size < len)
        return false;
    return qstrncmp(d->data, str, len) == 0;
}

/*! \overload

    Returns true if this byte array starts with character \a ch;
    otherwise returns false.
*/
bool QByteArray::startsWith(char ch) const
{
    if (d->size == 0)
        return false;
    return d->data[0] == ch;
}

/*!
    Returns true if this byte array ends with byte array \a ba;
    otherwise returns false.

    Example:
    \code
        QByteArray url("http://www.trolltech.com/index.html");
        if (url.endsWith(".html"))
            ...
    \endcode

    \sa startsWith(), right()
*/
bool QByteArray::endsWith(const QByteArray &ba) const
{
    if (d == ba.d || ba.d->size == 0)
        return true;
    if (d->size < ba.d->size)
        return false;
    return memcmp(d->data + d->size - ba.d->size, ba.d->data, ba.d->size) == 0;
}

/*! \overload

    Returns true if this byte array ends with string \a str; otherwise
    returns false.
*/
bool QByteArray::endsWith(const char *str) const
{
    if (!str || !*str)
        return true;
    int len = qstrlen(str);
    if (d->size < len)
        return false;
    return qstrncmp(d->data + d->size - len, str, len) == 0;
}

/*! \overload

    Returns true if this byte array ends with character \a ch;
    otherwise returns false.
*/
bool QByteArray::endsWith(char ch) const
{
    if (d->size == 0)
        return false;
    return d->data[d->size - 1] == ch;
}

/*!
    Returns a byte array that contains the leftmost \a len bytes of
    this byte array.

    The entire byte array is returned if \a len is greater than
    size().

    Example:
    \code
        QByteArray x("Pineapple");
        QByteArray y = x.left(4);
        // y == "Pine"
    \endcode

    \sa right(), mid(), startsWith(), truncate()
*/

QByteArray QByteArray::left(int len)  const
{
    if (len >= d->size)
        return *this;
    if (len < 0)
        len = 0;
    return QByteArray(d->data, len);
}

/*!
    Returns a byte array that contains the rightmost \a len bytes of
    this byte array.

    The entire byte array is returned if \a len is greater than
    size().

    Example:
    \code
        QByteArray x("Pineapple");
        QByteArray y = x.right(5);
        // y == "apple"
    \endcode

    \sa endsWith(), left(), mid()
*/

QByteArray QByteArray::right(int len) const
{
    if (len >= d->size)
        return *this;
    if (len < 0)
        len = 0;
    return QByteArray(d->data + d->size - len, len);
}

/*!
    Returns a byte array containing \a len bytes from this byte array,
    starting at position \a pos.

    If \a len is -1 (the default), or \a pos + \a len >= size(),
    returns a byte array containing all bytes starting at position \a
    pos until the end of the byte array.

    Example:
    \code
        QByteArray x("Five pineapples");
        QByteArray y = x.mid(5, 4);     // y == "pine"
        QByteArray z = x.mid(5);        // z == "pineapples"
    \endcode

    \sa left(), right()
*/

QByteArray QByteArray::mid(int pos, int len) const
{
    if (d == &shared_null || d == &shared_empty || pos >= d->size)
        return QByteArray();
    if (len < 0)
        len = d->size - pos;
    if (pos < 0) {
        len += pos;
        pos = 0;
    }
    if (len + pos > d->size)
        len = d->size - pos;
    if (pos == 0 && len == d->size)
        return *this;
    return QByteArray(d->data + pos, len);
}

/*!
    Returns a lower-case copy of the byte array.

    Example:
    \code
        QByteArray x("TROlltECH");
        QByteArray y = x.toLower();
        // y == "trolltech"
    \endcode

    \sa toUpper(),
        \link #asciinotion Note on character comparisons \endlink
*/
QByteArray QByteArray::toLower() const
{
    QByteArray s(*this);
    register char *p = s.data();
    if (p) {
        while (*p) {
            *p = tolower((uchar) *p);
            p++;
        }
    }
    return s;
}

/*!
    Returns an upper-case copy of the byte array.

    Example:
    \code
        QByteArray x("TROlltECH");
        QByteArray y = x.toUpper();
        // y == "TROLLTECH"
    \endcode

    \sa toLower(),
        \link #asciinotion Note on character comparisons \endlink
*/

QByteArray QByteArray::toUpper() const
{
    QByteArray s(*this);
    register char *p = s.data();
    if (p) {
        while (*p) {
            *p = toupper(*p);
            p++;
        }
    }
    return s;
}

/*! \fn void QByteArray::clear()

    Clears the contents of the byte array and makes it empty.

    \sa resize(), isEmpty()
*/

void QByteArray::clear()
{
    if (!--d->ref)
        qFree(d);
    d = &shared_null;
    ++d->ref;
}

/*! \relates QByteArray

    Writes byte array \a ba to the stream \a out and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &out, const QByteArray &ba)
{
    if (ba.isNull() && out.version() >= 6) {
        out << (Q_UINT32)0xffffffff;
        return out;
    }
    return out.writeBytes(ba, ba.size());
}

/*! \relates QByteArray

    Reads a byte array into \a ba from the stream \a in and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &in, QByteArray &ba)
{
    Q_UINT32 len;
    in >> len;                                        // read size of array
    if (len == 0xffffffff) {
        ba.clear();
        return in;
    }
    if ((int)len <= 0) {
        ba = QByteArray("");
        return in;
    }
    if (in.eof()) {
        ba.clear();
        return in;
    }
    ba.resize((int)len);
    if (ba.size() != (int)len) {
        qWarning("QDataStream: Not enough memory to read QByteArray");
        len = 0;
    }
    if (len > 0)                                // not null array
        in.readRawBytes(ba.data(), (uint)len);
    return in;
}
#endif //QT_NO_DATASTREAM

/*! \fn bool QByteArray::operator==(const QString &str) const

    Returns true if this byte array is equal to string \a str;
    otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator!=(const QString &str) const

    Returns true if this byte array is not equal to string \a str;
    otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator<(const QString &str) const

    Returns true if this byte array is lexically less than string \a
    str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator>(const QString &str) const

    Returns true if this byte array is lexically greater than string
    \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator<=(const QString &str) const

    Returns true if this byte array is lexically less than or equal
    to string \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator>=(const QString &str) const

    Returns true if this byte array is greater than or equal to string
    \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool operator==(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator==(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is equal to string \a a2;
    otherwise returns false.
*/

/*! \fn bool operator==(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is not equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is not equal to string \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is not equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator<(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than byte array
    \a a2; otherwise returns false.
*/

/*! \fn inline bool operator<(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than string
    \a a2; otherwise returns false.
*/

/*! \fn bool operator<(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically less than byte array
    \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than or equal
    to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than or equal
    to string \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically less than or equal
    to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than byte
    array \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than string
    \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically greater than byte array
    \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than or
    equal to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than or
    equal to string \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically greater than or
    equal to byte array \a a2; otherwise returns false.
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    Returns a byte array that is the result of concatenating byte
    array \a a1 and byte array \a a2.

    \sa QByteArray::operator+=()
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, char a2)
    \relates QByteArray

    \overload
*/

/*! \fn const QByteArray operator+(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload
*/

/*! \fn const QByteArray operator+(char a1, const QByteArray &a2)
    \relates QByteArray

    \overload
*/

/*!
    Returns a byte array that has whitespace removed from the start
    and the end, and which has each sequence of internal whitespace
    replaced with a single space.

    Whitespace means any character for which the standard C++
    isspace() function returns true. This includes the ASCII
    characters '\\t', '\\n', '\\v', '\\f', '\\r', and ' '.

    Example:
    \code
        QByteArray ba("  lots\t of\nwhitespace\r\n ");
        ba = ba.trimmed();
        // ba == "lots of whitespace";
    \endcode

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

    Whitespace means any character for which the standard C++
    isspace() function returns true. This includes the ASCII
    characters '\\t', '\\n', '\\v', '\\f', '\\r', and ' '.

    Example:
    \code
        QByteArray ba("  lots\t of\nwhitespace\r\n ");
        ba = ba.trimmed();
        // ba == "lots\t of\nwhitespace";
    \endcode

    Unlike simplified(), trimmed() leaves internal whitespace alone.

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

/*!
    Returns a byte array of size \a width that contains this byte
    array padded by the \a fill character.

    If \a truncate is false and the size() of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the size() of the byte array is more
    than \a width, then any bytes in a copy of the byte array
    after position \a width are removed, and the copy is returned.

    Example:
    \code
        QByteArray x("apple");
        QByteArray y = x.leftJustified(8, '.');   // y == "apple..."
    \endcode

    \sa rightJustified()
*/

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

/*!
    Returns a byte array of size \a width that contains the \a fill
    character followed by this byte array.

    If \a truncate is false and the size of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the size of the byte array is more
    than \a width, then the resulting byte array is truncated at
    position \a width.

    Example:
    \code
        QByteArray x("apple");
        QByteArray y = x.rightJustified(8, '.');    // y == "...apple"
    \endcode

    \sa leftJustified()
*/

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
    Returns the byte array converted to a \c {long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \sa number()
*/

Q_LLONG QByteArray::toLongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if (base != 0 && (base < 2 || base > 36)) {
        qWarning("QByteArray::toLongLong: Invalid base (%d)", base);
        base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToLongLong(*this, base, ok);
}

/*!
    Returns the byte array converted to an \c {unsigned long long}
    using base \a base, which is 10 by default and must be between 2
    and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \sa number()
*/

Q_ULLONG QByteArray::toULongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if (base != 0 && (base < 2 || base > 36)) {
        qWarning("QByteArray::toULongLong: Invalid base %d", base);
        base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToUnsLongLong(*this, base, ok);
}

/*!
    Returns the byte array converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to an \c {unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to an \c {unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to an \c {unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to a \c double value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

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
    Returns the byte array converted to a \c float value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \sa number()
*/

float QByteArray::toFloat(bool *ok) const
{
    return (float)toDouble(ok);
}


/*! \fn QByteArray &QByteArray::setNum(int n, int base)

    Sets the byte array to the printed value of \a n in base \a base (10
    by default) and returns a reference to the byte array. The \a base can
    be any value between 2 and 36.

    Example:
    \code
        QByteArray ba;
        int n = 63;
        ba.setNum(n);           // ba == "63"
        ba.setNum(n, 16);       // ba == "3f"
    \endcode

    \sa number(), toInt()
*/

/*! \fn QByteArray &QByteArray::setNum(uint n, int base)
    \overload

    \sa toUInt()
*/

/*! \fn QByteArray &QByteArray::setNum(short n, int base)
    \overload

    \sa toShort()
*/

/*! \fn QByteArray &QByteArray::setNum(ushort n, int base)
    \overload

    \sa toUShort()
*/

/*! \fn QByteArray &QByteArray::setNum(long n, int base)
    \overload

    \sa toLong()
*/

/*! \fn QByteArray &QByteArray::setNum(ulong n, int base)
    \overload

    \sa toULong()
*/

/*!
    \overload

    \sa toLongLong()
*/

QByteArray &QByteArray::setNum(Q_LLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (base < 2 || base > 36) {
        qWarning("QByteArray::setNum: Invalid base %d", base);
        base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->longLongToString(n, -1, base).latin1();
    return *this;
}

/*!
    \overload

    \sa toULongLong()
*/

QByteArray &QByteArray::setNum(Q_ULLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (base < 2 || base > 36) {
        qWarning("QByteArray::setNum: Invalid base %d", base);
        base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->unsLongLongToString(n, -1, base).latin1();
    return *this;
}

/*! \overload

    Sets the byte array to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    byte array.

    The format \a f can be any of the following:

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

    \sa toDouble()
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
            qWarning("QByteArray::setNum: Invalid format char '%c'", f);
#endif
            break;
    }

    QLocale locale(QLocale::C);
    *this = locale.d->doubleToString(n, prec, form, -1, flags).latin1();
    return *this;
}

/*! \fn QByteArray &QByteArray::setNum(float n, char f, int prec)
    \overload

    Sets the byte array to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    byte array.

    \sa toFloat()
*/

/*!
    Returns a byte array containing the string equivalent of the
    number \a n to base \a base (10 by default). The \a base can be
    any value between 2 and 36.

    Example:
    \code
        int n = 63;
        QByteArray::number(n);              // returns "63"
        QByteArray::number(n, 16);          // returns "3f"
        QByteArray::number(n, 16).upper();  // returns "3F"
    \endcode

    \sa setNum(), toInt()
*/
QByteArray QByteArray::number(int n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa toUInt()
*/
QByteArray QByteArray::number(uint n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa toLong()
*/
QByteArray QByteArray::number(long n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa toULong()
*/
QByteArray QByteArray::number(ulong n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa toLongLong()
*/
QByteArray QByteArray::number(Q_LLONG n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa toULongLong()
*/
QByteArray QByteArray::number(Q_ULLONG n, int base)
{
    QByteArray s;
    s.setNum(n, base);
    return s;
}

/*! \overload

    Returns a byte array that contains the printed value of \a n,
    formatted in format \a f with precision \a prec.

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
        QByteArray ba = QByteArray::number(12.3456, 'E', 3);
        // ba == 1.235E+01
    \endcode

    \sa toDouble()
*/
QByteArray QByteArray::number(double n, char f, int prec)
{
    QByteArray s;
    s.setNum(n, f, prec);
    return s;
}

/*! \class QConstByteArray
    \brief The QConstByteArray class provides a QByteArray object using constant byte data.

    \ingroup tools
    \reentrant

    To minimize copying, highly optimized applications can use
    QConstByteArray to create a QByteArray-compatible object from
    existing byte data. It is then the programmer's responsibility to
    ensure that the byte data exists for the entire lifetime of the
    QConstByteArray object.

    The resulting QConstByteArray object can be used as a const
    QByteArray. Any attempts to modify copies of the QConstByteArray
    will cause it to create a deep copy of the data, ensuring that
    the raw data isn't modified. The QConstByteArray object itself
    should never be modified.

    Here's an example of how we can read data using a QDataStream on
    raw data in memory without requiring to copy the data into a
    QByteArray:

    \code
	static const char mydata[] = {
            0x00, 0x00, 0x03, 0x84, 0x78, 0x9c, 0x3b, 0x76,
            0xec, 0x18, 0xc3, 0x31, 0x0a, 0xf1, 0xcc, 0x99,
	    ...
            0x6d, 0x5b
        };

	QConstByteArray data(mydata, sizeof(mydata));
        QBuffer buffer(data);
        buffer.open(IO_ReadOnly);
        QDataStream in(&buffer);
	...
    \endcode
*/

/*!
    Constructs a QConstByteArray that uses the first \a size characters
    in the array \a bytes.

    Only a pointer to the data in \a bytes is copied. The caller must
    be able to guarantee that \a bytes will not be delete or modified
    during the QConstByteArray's lifetime.
*/

QConstByteArray::QConstByteArray(const char *bytes, int size)
    : QByteArray((Data *)qMalloc(sizeof(Data)), 0)
{
    d->ref = 1;
    d->alloc = d->size = size;
    d->data = bytes ? (char *)bytes : d->array;
    *d->array = '\0';
}

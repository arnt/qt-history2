#include "qbitarray.h"
#include <string.h>
#include <qdatastream.h>

/*!
    \class QBitArray
    \brief The QBitArray class provides an array of bits.

    \ingroup tools
    \ingroup shared
    \reentrant

    A QBitArray is an array that offers access to individual bits and
    provides operators (AND, OR, XOR, and NOT) that work on entire
    arrays of bits. It uses \l{implicit sharing} (copy-on-write) to
    reduce memory usage and avoid needless copying of data.

    The following code constructs a QBitArray containing 200 bits
    initialized to false (0):

    \code
        QBitArray ba(200);
    \endcode

    To initialize the bits to true, either pass \c true as second
    argument to the constructor, or call fill() later on.

    QBitArray uses 0-based indexes, just like C++ arrays. To access
    the bit at a particular index position, you can use operator[]().
    On non-const bit arrays, operator[]() returns a reference to a
    bit that can be used on the left side of an assignment. For
    example:

    \code
        QBitArray ba;
        ba.resize(3);
        ba[0] = true;
        ba[1] = false;
        ba[2] = true;
    \endcode

    For technical reasons, it is more efficient to use testBit() and
    setBit() to access bits in the array than operator[](). For
    example:

    \code
        QBitArray ba(3);
        ba.setBit(0, true);
        ba.setBit(1, false);
        ba.setBit(2, true);
    \endcode

    QBitArray supports \c{&} (AND), \c{|} (OR), \c{^} (XOR), \c{~}
    (NOT), as well as \c{&=}, \c{|=}, and \c{^=}. These operators
    work in the same way as the built-in C++ bitwise operators of the
    same name. For example:

    \code
        QBitArray x(5);
        x.setBit(3, true);
        // x: [ 0, 0, 0, 1, 0 ]

        QBitArray y(5);
        y.setBit(4, true);
        // y: [ 0, 0, 0, 0, 1 ]

        x |= y;
        // x: [ 0, 0, 0, 1, 1 ]
    \endcode

    \sa QByteArray, QVector
*/

/*! \fn QBitArray::QBitArray()

    Constructs an empty bit array.

    \sa isEmpty()
*/

/*!
    Constructs a bit array containing \a size bits. The bits are
    initialized with \a value.
*/
QBitArray::QBitArray(int size, bool value)
{
    d.resize(1 + (size+7)/8);
    uchar* c = (uchar*)d.data();
    memset(c, value ? 0xff : 0, d.size());
    *c = d.size()*8 - size;
    if (value && size && size % 8)
        *(c+1+size/8) &= (1 << (size%8)) - 1;
}

/*! \fn int QBitArray::size() const

    Returns the number of bits stored in the bit array.

    \sa resize()
*/

/*! \fn int QBitArray::count() const

    Same as size().
*/

/*!
    Resizes the bit array to \a size bits.

    If \a size is greater than the current size, the bit array is
    extended to make it \a size bits with the extra bits added to the
    end. The new bits are initialized to false (0).

    If \a size is less than the current size, bits are removed from
    the end.

    \sa size()
*/
void QBitArray::resize(int size)
{
    int s = d.size();
    d.resize(1 + (size+7)/8);
    uchar* c = (uchar*)d.data();
    if (size > (s << 3))
        memset(c + s, 0, d.size() - s);
    *c = d.size()*8 - size;
}

/*! \fn bool QBitArray::isEmpty() const

    Returns true if this bit array has size 0; otherwise returns
    false.

    \sa size()
*/

/*! \fn bool QBitArray::isNull() const

    \internal
*/

/*! \fn bool QBitArray::ensure_constructed()

    \internal
*/

/*! \fn bool QBitArray::fill(bool value, int size = -1)

    Sets every bit in the bit array to \a value. If \a size is
    different from -1 (the default), the bit array is resized to \a
    size beforehand.

    Example:
    \code
        QBitArray ba(8);
        ba.fill(true);
        // ba: [ 1, 1, 1, 1, 1, 1, 1, 1 ]

        ba.fill(false, 2);
        // ba: [ 0, 0 ]
    \endcode

    \sa resize()
*/

/*!
    \overload

    Sets bits at index positions \a first to (and including) \a last
    to \a value.

    \a first and \a last must be a valid index position in the bit
    array (i.e., 0 <= \a first < size() and 0 <= \a last < size()).
*/

void QBitArray::fill(bool value, int first, int last)
{
    while (first <= last && first & 0x7)
        setBit(first++, value);
    int len = last - first + 1;
    if (len <= 0)
        return;
    int s = len & ~0x7;
    uchar *c = (uchar*)d.data();
    memset(c + (first >> 3) + 1, value ? 0xff : 0, s >> 3);
    s += first;
    while (s <= last)
        setBit(s++, value);
}

/*! \fn bool QBitArray::isDetached() const

    \internal
*/


/*! \fn void QBitArray::detach()

    \internal
*/

/*! \fn bool QBitArray::toggleBit(int i)

    Inverts the value of the bit at index position \a i.

    If the previous value was 0, the new value will be 1. If the
    previous value was 1, the new value will be 0.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    \sa setBit(), clearBit()
*/

/*! \fn bool QBitArray::testBit(int i) const

    Returns true if the bit at index position \a i is 1; otherwise
    returns false.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    \sa setBit(), clearBit()
*/

/*! \fn bool QBitArray::setBit(int i)

    Sets the bit at index position \a i to 1.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    \sa clearBit(), toggleBit()
*/

/*! \fn void QBitArray::setBit(int i, bool value)

    \overload

    Sets the bit at index position \a i to \a value.
*/

/*! \fn void QBitArray::clearBit(int i)

    Sets the bit at index position \a i to 0.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    \sa setBit(), toggleBit()
*/

/*! \fn bool QBitArray::at(int i) const

    Returns the value of the bit at index position \a i.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    \sa operator[]()
*/

/*! \fn QBitRef QBitArray::operator[](int i)

    Returns the bit at index position \a i as a modifiable reference.

    \a i must be a valid index position in the bit array (i.e., 0 <=
    \a i < size()).

    Example:
    \code
        QBitArray a(3);
        a[0] = false;
        a[1] = true;
        a[2] = a[0] ^ a[1];
    \endcode

    The return value is of type QBitRef, a helper class for QBitArray.
    When you get an object of type QBitRef, you can assign to
    it, and the assignment will apply to the bit in the QBitArray
    from which you got the reference.

    The functions testBit(), setBit(), and clearBit() are slightly
    faster.

    \sa at(), testBit(), setBit(), clearBit()
*/

/*! \fn bool QBitArray::operator[](int i) const

    \overload
*/

/*! \fn bool QBitArray::operator[](uint i)

    \overload
*/

/*! \fn bool QBitArray::operator[](uint i) const

    \overload
*/

/*! \fn QBitArray &QBitArray::operator=(const QBitArray &other)

    Assigns \a other to this bit array and returns a reference to
    this byte array.
*/

/*! \fn bool QBitArray::operator==(const QBitArray &other) const

    Returns true if \a other is equal to this bit array; otherwise
    returns false.

    \sa operator!=()
*/

/*! \fn bool QBitArray::operator!=(const QBitArray &other) const

    Returns true if \a other is not equal to this bit array;
    otherwise returns false.

    \sa operator==()
*/

/*!
    Performs the AND operation between all bits in this bit array and
    \a other. Assigns the result to this bit array, and returns a
    reference to it.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        a &= b;                         // a: [ 1, 0, 0 ]
    \endcode

    \sa operator&(), operator|=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator&=(const QBitArray &other)
{
    resize(qMax(size(), other.size()));
    uchar *a1 = (uchar *)d.data()+1;
    const uchar *a2 = (const uchar *)other.d.constData() + 1;
    int n = qMin(d.size(), other.d.size()) - 1;
    int p = qMax(d.size(), other.d.size()) - 1 - n;
    while (n-- > 0)
        *a1++ &= *a2++;
    while (p-- > 0)
        *a1++ = 0;
    return *this;
}

/*!
    Performs the OR operation between all bits in this bit array and
    \a other. Assigns the result to this bit array, and returns a
    reference to it.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        a |= b;                         // a: [ 1, 1, 1 ]
    \endcode

    \sa operator|(), operator&=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator|=(const QBitArray &other)
{
    resize(qMax(size(), other.size()));
    uchar *a1 = (uchar *)d.data()+1;
    const uchar *a2 = (const uchar *)other.d.constData()+1;
    int n = qMin(d.size(), other.d.size()) - 1;
    while (n-- > 0)
        *a1++ |= *a2++;
    return *this;
}

/*!
    Performs the XOR operation between all bits in this bit array and
    \a other. Assigns the result to this bit array, and returns a
    reference to it.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        a ^= b;                         // a: [ 0, 1, 1 ]
    \endcode

    \sa operator^(), operator&=(), operator|=(), operator~()
*/

QBitArray &QBitArray::operator^=(const QBitArray &other)
{
    resize(qMax(size(), other.size()));
    uchar *a1 = (uchar *)d.data();
    const uchar *a2 = (const uchar *)other.d.constData();
    int n = qMin(d.size(), other.d.size()) - 1;
    while (n-- > 0)
        *a1++ ^= *a2++;
    return *this;
}

/*!
    Returns a bit array that contains the inverted bits of this bit
    array.

    Example:
    \code
        QBitArray a(3);
        QBitArray b;
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b = ~a;                         // b: [ 0, 1, 0 ]
    \endcode

    \sa operator&(), operator|(), operator^()
*/

QBitArray QBitArray::operator~() const
{
    int sz = size();
    QBitArray a(sz);
    const uchar *a1 = (const uchar *)d.constData() + 1;
    uchar *a2 = (uchar *)a.d.data() + 1;
    int n = d.size() - 1;
    while (n--)
        *a2++ = ~*a1++;
     if (sz && sz%8)
         *(a2-1) &= (1 << (sz%8)) - 1;
    return a;
}

/*!
    \relates QBitArray

    Returns the AND result between the bit arrays \a a1 and \a a2.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        QBitArray c;
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        c = a & b;                      // c: [ 1, 0, 0 ]
    \endcode

    \sa QBitArray::operator&=(), operator|(), operator^()
*/

QBitArray operator&(const QBitArray &a1, const QBitArray &a2)
{
    QBitArray tmp = a1;
    tmp &= a2;
    return tmp;
}

/*!
    Returns the OR result between the bit arrays \a a1 and \a a2.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        QBitArray c;
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        c = a | b;                      // c: [ 1, 1, 1 ]
    \endcode

    \sa QBitArray::operator|=(), operator&(), operator^()
*/

QBitArray operator|(const QBitArray &a1, const QBitArray &a2)
{
    QBitArray tmp = a1;
    tmp |= a2;
    return tmp;
}

/*!
    \relates QBitArray

    Returns the XOR result between the bit arrays \a a1 and \a a2.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (if one array is shorter than the other)
    taken to be 0.

    Example:
    \code
        QBitArray a(3);
        QBitArray b(2);
        QBitArray c;
        a[0] = 1; a[1] = 0; a[2] = 1;   // a: [ 1, 0, 1 ]
        b[0] = 1; b[1] = 0;             // b: [ 1, 1 ]
        c = a ^ b;                      // c: [ 0, 1, 1 ]
    \endcode

    \sa QBitArray::operator^=(), operator&(), operator|()
*/

QBitArray operator^(const QBitArray &a1, const QBitArray &a2)
{
    QBitArray tmp = a1;
    tmp ^= a2;
    return tmp;
}

/*!
    \class QBitRef
    \reentrant
    \brief The QBitRef class is an internal class, used with QBitArray.

    \internal

    The QBitRef is required by the indexing [] operator on bit arrays.
    It is not for use in any other context.
*/

/*! \fn QBitRef::QBitRef (QBitArray& a, int i)

    Constructs a reference to element \a i in the QBitArray \a a.
    This is what QBitArray::operator[] constructs its return value
    with.
*/

/*! \fn QBitRef::operator bool() const

    Returns the value referenced by the QBitRef.
*/

/*! \fn bool QBitRef::operator!() const

    \internal
*/

/*! \fn QBitRef& QBitRef::operator= (const QBitRef& v)

    Sets the value referenced by the QBitRef to that referenced by
    QBitRef \a v.
*/

/*! \fn QBitRef& QBitRef::operator= (bool v)
    \overload

    Sets the value referenced by the QBitRef to \a v.
*/


/*****************************************************************************
  QBitArray stream functions
 *****************************************************************************/

/*!
    \relates QBitArray

    Writes bit array \a a to stream \a out.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const QBitArray &ba)
{
    Q_UINT32 len = ba.size();
    out << len;
    if (len > 0)
        out.writeRawBytes(ba.d, ba.d.size());
    return out;
}

/*!
    \relates QBitArray

    Reads a bit array into \a ba from stream \a in.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &in, QBitArray &ba)
{
    Q_UINT32 len;
    in >> len;
    if (!len) {
        ba.d.clear();
    } else {
        ba.resize(len); // read size of array
        if (ba.size() != (int)len) {                // resize array
            qWarning("QDataStream: Not enough memory to read QBitArray");
            len = 0;
        }
        if (len > 0)                                // read data
            in.readRawBytes(ba.d.data(), ba.d.size());
    }
    return in;
}
#endif

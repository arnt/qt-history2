#include "qbitarray.h"
#include <string.h>
#include <qdatastream.h>

/*!
    \class QBitArray qbitarray.h
    \reentrant
    \brief The QBitArray class provides an array of bits.

    \ingroup collection
    \ingroup tools
    \ingroup shared

    QBitArray uses \link shclass.html implicit sharing\endlink, which
    makes it very efficient and easy to use.

    A QBitArray is an array that can access individual bits and
    perform bit-operations (AND, OR, XOR and NOT) on entire arrays or
    bits.

    Bits can be manipulated by the setBit() and clearBit() functions,
    but it is also possible to use the indexing [] operator to test
    and set individual bits.

    Example:
    \code
    QBitArray a;
    a.setBit(0);
    a.clearBit(1);
    a.setBit(2);     // a = [1 0 1]

    QBitArray b;
    b[0] = 1;
    b[1] = 1;
    b[2] = 0;          // b = [1 1 0]

    QBitArray c;
    c = ~a & b;        // c = [0 1 0]
    \endcode

    When a QBitArray is constructed all bits are 0. The array resizes
    automatically, or it can be resized with resize(). Bits can be set
    with setBit() and cleared with clearBit(). Bits can be toggled
    with toggleBit(). A bit's value can be obtained with testBit() and
    with at().

    QBitArray supports the \& (AND), | (OR), ^ (XOR) and ~ (NOT)
    operators.
*/


/*! \fn QBitArray::QBitArray()
    Constructs an empty bit array.
*/

/*
 */

/*!
    Constructs a bit array of \a size bits. The bits are initialized
    with \a val.
*/
QBitArray::QBitArray(int size, bool val)
{
    d.resize(1 + (size+7)/8);
    uchar* c = (uchar*)d.data();
    memset(c, val ? 0xff : 0, d.size());
    *c = d.size()*8 - size;
    if (val && size && size%8)
	*(c+1+size/8) &= (1 << (size%8)) - 1;
}

/*!
    \fn int QBitArray::size() const

    Returns the bit array's size (number of bits).

    \sa resize()
*/

/*!
    Resizes the bit array to \a size bits.

    If the array is expanded, the new bits are set to 0.

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

/*!
  Fills the array with with \a val from \a first to \a last.

  \sa fill()
*/

void QBitArray::fill(bool val, int first, int last)
{
    while (first <= last && first & 0x00000007)
	setBit(first++, val);
    int len = last - first;
    if (len < 0)
	return;
    int s = len & 0xfffffff8;
    uchar *c = (uchar*)d.data();
    memset(c + (first >> 3) + 1, val ? 0xff : 0, s >> 3);
    s += first;
    while (s <= last)
	setBit(s++, val);
    return;
}

/*! \fn void QBitArray::detach()
  \internal
*/


/*! \fn bool QBitArray::testBit(int i) const

  Returns true if the bit at position \a i is set, i.e. is 1; otherwise
  returns false.

  \sa setBit(), clearBit()
*/

/*! \fn bool QBitArray::setBit(int i)

  Sets the bit at position \a i to 1.

   \sa clearBit(), toggleBit()
*/

/*!
    \fn void QBitArray::setBit(int i, bool val)

    \overload

    Sets the bit at position \a i to \a val.

    Equivalent to:
    \code
    if (val)
	setBit(i);
    else
	clearBit(i);
    \endcode

    \sa clearBit() toggleBit()
*/

/*! \fn bool QBitArray::clearBit(int i) const
    Clears the bit at position \a i, i.e. sets it to 0.

    \sa setBit(), toggleBit()
*/

/*!
    \fn bool QBitArray::at(int i) const

    Returns the value (0 or 1) of the bit at position \a i.

    \sa operator[]()
*/

/*!
    \fn QBitRef QBitArray::operator[](int i)

    Implements the [] operator for bit arrays.

    The returned QBitRef is a context object. It makes it possible to
    get and set a single bit value by its \a i position.

    Example:
    \code
    QBitArray a;
    a[0] = 0;
    a[1] = 1;
    a[2] = a[0] ^ a[1];
    \endcode

    The functions testBit(), setBit() and clearBit() are slightly faster.

    \sa at()
*/

/*!
    \overload bool QBitArray::operator[](int i) const

    Implements the [] operator for constant bit arrays.
*/



/*!
    Performs the AND operation between all bits in this bit array and
    \a a. Returns a reference to this bit array.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.
    \code
    QBitArray a(3), b(2);
    a[0] = 1;  a[1] = 0;  a[2] = 1;     // a = [1 0 1]
    b[0] = 1;  b[1] = 0;                // b = [1 0]
    a &= b;                             // a = [1 0 0]
    \endcode

    \sa operator|=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator&=(const QBitArray &a)
{
    resize(qMax(size(), a.size()));
    uchar *a1 = (uchar *)d.data()+1;
    const uchar *a2 = (const uchar *)a.d.constData() + 1;
    int n = qMin(d.size(), a.d.size()) - 1;
    int p = qMax(d.size(), a.d.size()) - 1 - n;
    while (n-- > 0)
	*a1++ &= *a2++;
    while (p-- > 0)
	*a1++ = 0;
    return *this;
}

/*!
    Performs the OR operation between all bits in this bit array and
    \a a. Returns a reference to this bit array.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.
    \code
    QBitArray a(3), b(2);
    a[0] = 1;  a[1] = 0;  a[2] = 1;     // a = [1 0 1]
    b[0] = 1;  b[1] = 0;                // b = [1 0]
    a |= b;                             // a = [1 0 1]
    \endcode

    \sa operator&=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator|=(const QBitArray &a)
{
    resize(qMax(size(), a.size()));
    uchar *a1 = (uchar *)d.data()+1;
    const uchar *a2 = (const uchar *)a.d.constData()+1;
    int n = qMin(d.size(), a.d.size()) - 1;
    while (n-- > 0)
	*a1++ |= *a2++;
    return *this;
}

/*!
    Performs the XOR operation between all bits in this bit array and
    \a a. Returns a reference to this bit array.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.
    \code
    QBitArray a(3), b(2);
    a[0] = 1;  a[1] = 0;  a[2] = 1;     // a = [1 0 1]
    b[0] = 1;  b[1] = 0;                // b = [1 0]
    a ^= b;                             // a = [0 0 1]
    \endcode

    \sa operator&=(), operator|=(), operator~()
*/

QBitArray &QBitArray::operator^=(const QBitArray &a)
{
    resize(qMax(size(), a.size()));
    uchar *a1 = (uchar *)d.data();
    const uchar *a2 = (const uchar *)a.d.constData();
    int n = qMin(d.size(), a.d.size()) - 1;
    while (n-- > 0)
	*a1++ ^= *a2++;
    return *this;
}

/*!
    Returns a bit array that contains the inverted bits of this bit array.

    Example:
    \code
    QBitArray a(3), b;
    a[0] = 1;  a[1] = 0; a[2] = 1;	// a = [1 0 1]
    b = ~a;				// b = [0 1 0]
    \endcode
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
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.

    \sa QBitArray::operator&=()
*/

QBitArray operator&(const QBitArray &a1, const QBitArray &a2)
{
    QBitArray tmp = a1;
    tmp &= a2;
    return tmp;
}

/*!
    \relates QBitArray

    Returns the OR result between the bit arrays \a a1 and \a a2.

    The result has the length of the longest of the two bit arrays,
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.

    \sa QBitArray::operator|=()
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
    with any missing bits (i.e. if one array is shorter than the
    other), taken to be 0.

    \sa QBitArray::operator^()
*/

QBitArray operator^(const QBitArray &a1, const QBitArray &a2)
{
    QBitArray tmp = a1;
    tmp ^= a2;
    return tmp;
}


/*!
    \class QBitRef qbitarray.h
    \reentrant
    \brief The QBitRef class is an internal class, used with QBitArray.

    \ingroup collection

    The QBitRef is required by the indexing [] operator on bit arrays.
    It is not for use in any other context.
*/

/*!
    \fn QBitRef::QBitRef (QBitArray* a, uint i)

    Constructs a reference to element \a i in the QBitArray \a a.
    This is what QBitArray::operator[] constructs its return value
    with.
*/

/*!
    \fn QBitRef::operator int() const

    Returns the value referenced by the QBitRef.
*/

/*!
    \fn QBitRef& QBitRef::operator= (const QBitRef& v)

    Sets the value referenced by the QBitRef to that referenced by
    QBitRef \a v.
*/

/*!
    \overload QBitRef& QBitRef::operator= (bool v)

    Sets the value referenced by the QBitRef to \a v.
*/


/*****************************************************************************
  QBitArray stream functions
 *****************************************************************************/

/*!
    \relates QBitArray

    Writes bit array \a a to stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
#ifndef QT_NO_DATASTREAM
QDataStream &operator<<( QDataStream &s, const QBitArray &a )
{
    Q_UINT32 len = a.size();
    s << len;					// write size of array
    if ( len > 0 )				// write data
	s.writeRawBytes( a.d, a.d.size() );
    return s;
}

/*!
    \relates QBitArray

    Reads a bit array into \a a from stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QBitArray &a )
{
    Q_UINT32 len;
    s >> len;
    if (!len) {
	a.d.clear();
    } else {
	a.resize( len ); // read size of array
	if ( a.size() != (int)len ) {		// resize array
	    qWarning( "QDataStream: Not enough memory to read QBitArray" );
	    len = 0;
	}
	if ( len > 0 )				// read data
	    s.readRawBytes( a.d.data(), a.d.size() );
    }
    return s;
}
#endif


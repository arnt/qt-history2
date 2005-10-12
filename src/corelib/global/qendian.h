/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QENDIAN_H
#define QENDIAN_H
#include "qglobal.h"
/*
 * ENDIAN FUNCTIONS
 */ 
#if 1
inline void qbswap(const uchar *src, uchar *dest, int size)
{
    for (int i = 0; i < size ; ++i) dest[i] = src[size - 1 - i];
}

/*
 * qbswap(quint *dest, const uchar *src);
 * Changes the byte order of \a src from big endian to little endian or vice versa
 * and stores the result in \a dest.
 * \a src can be unaligned data, but \a dest must be aligned.
 */
template <typename T> inline void qbswap(const uchar *src, T *dest);

template <> inline void qbswap(const uchar *src, quint64 *dest)
{
    qbswap(src, reinterpret_cast<uchar*>(dest), 8);
}

template <> inline void qbswap(const uchar *src, quint32 *dest)
{
    qbswap(src, reinterpret_cast<uchar*>(dest), 4);
}

template <> inline void qbswap(const uchar *src, quint16 *dest)
{
    qbswap(src, reinterpret_cast<uchar*>(dest), 2);
}

// signed specializations
template <> inline void qbswap(const uchar *src, qint64 *dest)
{ qbswap<quint64>(src, reinterpret_cast<quint64*>(dest)); }

template <> inline void qbswap(const uchar *src, qint32 *dest)
{ qbswap<quint32>(src, reinterpret_cast<quint32*>(dest)); }

template <> inline void qbswap(const uchar *src, qint16 *dest)
{ qbswap<quint16>(src, reinterpret_cast<quint16*>(dest)); }


template <typename T> inline void qbswap(const T *src, uchar *dest);

template <> inline void qbswap(const quint64 *src, uchar *dest)
{
    qbswap(reinterpret_cast<const uchar*>(src), dest, 8);
}

template <> inline void qbswap(const quint32 *src, uchar *dest)
{
    qbswap(reinterpret_cast<const uchar*>(src), dest, 4);
}

template <> inline void qbswap(const quint16 *src, uchar *dest)
{
    qbswap(reinterpret_cast<const uchar*>(src), dest, 2);
}

// signed specializations
template <> inline void qbswap(const qint64 *src, uchar *dest)
{ qbswap<quint64>(reinterpret_cast<const quint64*>(src), dest); }

template <> inline void qbswap(const qint32 *src, uchar *dest)
{ qbswap<quint32>(reinterpret_cast<const quint32*>(src), dest); }

template <> inline void qbswap(const qint16 *src, uchar *dest)
{ qbswap<quint16>(reinterpret_cast<const quint16*>(src), dest); }

// Used to implement qhtole,qletoh on little-endian systems. (and similar cases on big-endian systems)
template <typename T> inline void qToUnaligned(const uchar *src, T *dest);

template <> inline void qToUnaligned(const uchar *src, quint64 *dest)
{ qMemCopy(dest, src, 8); }
template <> inline void qToUnaligned(const uchar *src, quint32 *dest)
{ qMemCopy(dest, src, 4); }
template <> inline void qToUnaligned(const uchar *src, quint16 *dest)
{ qMemCopy(dest, src, 2); }

// signed specializations
template <> inline void qToUnaligned(const uchar *src, qint64 *dest)
{ qToUnaligned<quint64>(src,reinterpret_cast<quint64*>(dest)); }
template <> inline void qToUnaligned(const uchar *src, qint32 *dest)
{ qToUnaligned<quint32>(src,reinterpret_cast<quint32*>(dest)); }
template <> inline void qToUnaligned(const uchar *src, qint16 *dest)
{ qToUnaligned<quint16>(src, reinterpret_cast<quint16*>(dest)); }
#endif

template <typename T> inline T qFromLittleEndian(const uchar *src);
template <> inline quint64 qFromLittleEndian(const uchar *src)
{
    return 0
        | src[0]
        | src[1] * Q_UINT64_C(0x0000000000000100)
        | src[2] * Q_UINT64_C(0x0000000000010000)
        | src[3] * Q_UINT64_C(0x0000000001000000)
        | src[4] * Q_UINT64_C(0x0000000100000000)
        | src[5] * Q_UINT64_C(0x0000010000000000)
        | src[6] * Q_UINT64_C(0x0001000000000000)
        | src[7] * Q_UINT64_C(0x0100000000000000);
}

template <> inline quint32 qFromLittleEndian(const uchar *src)
{
    return 0
        | src[0]
        | src[1] * Q_UINT64_C(0x0000000000000100)
        | src[2] * Q_UINT64_C(0x0000000000010000)
        | src[3] * Q_UINT64_C(0x0000000001000000);
}

template <> inline quint16 qFromLittleEndian(const uchar *src)
{
    return 0
        | src[0]
        | src[1] * Q_UINT64_C(0x0000000000000100);
}

template <> inline qint64 qFromLittleEndian(const uchar *src)
{ return static_cast<qint64>(qFromLittleEndian<quint64>(src)); }

template <> inline qint32 qFromLittleEndian(const uchar *src)
{ return static_cast<qint32>(qFromLittleEndian<quint32>(src)); }

template <> inline qint16 qFromLittleEndian(const uchar *src)
{ return static_cast<qint16>(qFromLittleEndian<quint16>(src)); }


template <typename T> inline T qFromBigEndian(const uchar *src);
template <> inline quint64 qFromBigEndian(const uchar *src)
{
    return 0
        | src[7]
        | src[6] * Q_UINT64_C(0x0000000000000100)
        | src[5] * Q_UINT64_C(0x0000000000010000)
        | src[4] * Q_UINT64_C(0x0000000001000000)
        | src[3] * Q_UINT64_C(0x0000000100000000)
        | src[2] * Q_UINT64_C(0x0000010000000000)
        | src[1] * Q_UINT64_C(0x0001000000000000)
        | src[0] * Q_UINT64_C(0x0100000000000000);
}

template <> inline quint32 qFromBigEndian(const uchar *src)
{
    return 0
        | src[3]
        | src[2] * Q_UINT64_C(0x0000000000000100)
        | src[1] * Q_UINT64_C(0x0000000000010000)
        | src[0] * Q_UINT64_C(0x0000000001000000);
}

template <> inline quint16 qFromBigEndian(const uchar *src)
{
    return 0
        | src[1]
        | src[0] * Q_UINT64_C(0x0000000000000100);
}

template <> inline qint64 qFromBigEndian(const uchar *src)
{ return static_cast<qint64>(qFromBigEndian<quint64>(src)); }

template <> inline qint32 qFromBigEndian(const uchar *src)
{ return static_cast<qint32>(qFromBigEndian<quint32>(src)); }

template <> inline qint16 qFromBigEndian(const uchar *src)
{ return static_cast<qint16>(qFromBigEndian<quint16>(src)); }

/*
template <typename T> inline void qToBigEndian(T src, uchar *dest);
template <> inline void qToBigEndian(T src, uchar *dest)
{
    qbswap<T>(
}
*/
/*
 * T qbswap(T source).
 * Changes the byte order of a value from big endian to little endian or vice versa.
 * This function can be used if you are not concerned about alignment issues, 
 * and it is therefore a bit more convenient and in most cases more efficient.
 */
template <typename T> T qbswap(T source);
template <> inline quint64 qbswap(quint64 source)
{
    return 0
        | ((source & Q_UINT64_C(0x00000000000000ff)) << 56)
        | ((source & Q_UINT64_C(0x000000000000ff00)) << 40)
        | ((source & Q_UINT64_C(0x0000000000ff0000)) << 24)
        | ((source & Q_UINT64_C(0x00000000ff000000)) << 8)
        | ((source & Q_UINT64_C(0x000000ff00000000)) >> 8)
        | ((source & Q_UINT64_C(0x0000ff0000000000)) >> 24)
        | ((source & Q_UINT64_C(0x00ff000000000000)) >> 40)
        | ((source & Q_UINT64_C(0xff00000000000000)) >> 56);
}

template <> inline quint32 qbswap(quint32 source)
{
    return 0
        | ((source & 0x000000ff) << 24)
        | ((source & 0x0000ff00) << 8)
        | ((source & 0x00ff0000) >> 8)
        | ((source & 0xff000000) >> 24);
}

template <> inline quint16 qbswap(quint16 source)
{
    return 0
        | ((source & 0x00ff) << 8)
        | ((source & 0xff00) >> 8);
}

// signed specializations
template <> inline qint64 qbswap(qint64 source)
{
    return qbswap<quint64>(quint64(source));
}

template <> inline qint32 qbswap(qint32 source)
{
    return qbswap<quint32>(quint32(source));
}

template <> inline qint16 qbswap(qint16 source)
{
    return qbswap<quint16>(quint16(source));
}


/* This function will read a big-endian (also known as network order) encoded value from \a src
 * and store the host-endian value in \a dest.
 * \a src can be unaligned data, but \a dest must be aligned.
 */
 
/* This function will read a little-endian encoded value
 * from a memory location and return the value in host byte order.
 * This function should also be alignment-safe.
 */

template <typename T> inline void qToBigEndian(T src, uchar *dest);
template <typename T> inline T qFromBigEndian(const uchar *src);
template <typename T> inline void qToLittleEndian(T src, uchar *dest);
template <typename T> inline T qFromLittleEndian(const uchar *src);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN

template <typename T> inline T qToBigEndian(T source)
{ return source; }
template <typename T> inline T qFromBigEndian(T source)
{ return source; }
template <typename T> inline T qToLittleEndian(T source)
{ return qbswap<T>(source); }
template <typename T> inline T qFromLittleEndian(T source)
{ return qbswap<T>(source); }
template <typename T> inline void qToBigEndian(T src, uchar *dest)
{ qToUnaligned<T>(src, dest); }
template <typename T> inline T qFromBigEndian(const uchar *src)
{ return qFromUnaligned<T>(src); }
template <typename T> inline void qToLittleEndian(T src, uchar *dest)
{ qbswap<T>(dest, src); }
template <typename T> inline T qFromLittleEndian(const uchar *src)
{ qbswap<T>(dest, src); }
#else // Q_LITTLE_ENDIAN

template <typename T> inline T qToBigEndian(T source)
{ return qbswap<T>(source); }
template <typename T> inline T qFromBigEndian(T source)
{ return qbswap<T>(source); }
template <typename T> inline T qToLittleEndian(T source)
{ return source; }
template <typename T> inline T qFromLittleEndian(T source)
{ return source; }

template <typename T> inline void qToBigEndian(T src, uchar *dest)
{ qbswap<T>(reinterpret_cast<uchar*>(&src), dest); }
//template <typename T> inline T qFromBigEndian(const uchar *src)
//{ qbswap<T>(dest, src); }
template <typename T> inline void qToLittleEndian(T src, uchar *dest)
{ qToUnaligned<T>(src, dest); }
//template <typename T> inline T qFromLittleEndian(const uchar *src)
//{ return qFromUnaligned<T>(src); }

#endif // Q_BYTE_ORDER == Q_BIG_ENDIAN

#endif
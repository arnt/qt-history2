#ifndef QNUMERIC_P_H
#define QNUMERIC_P_H

#include <string.h>

enum {
    QtLittleEndian,
    QtBigEndian

#ifdef Q_BYTE_ORDER
#  if Q_BYTE_ORDER == Q_BIG_ENDIAN
    , QtByteOrder = QtBigEndian
#  elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    , QtByteOrder = QtLittleEndian
#  else
#    error "undefined byte order"
#  endif
};
#else
};
static const unsigned int qt_one = 1;
static const bool QtByteOrder = ((*((unsigned char *) &qt_one) == 0) ? QtBigEndian : QtLittleEndian);
#endif

static const unsigned char qt_be_inf_bytes[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
static const unsigned char qt_le_inf_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
static inline double qtInf()
{
    return *reinterpret_cast<const double *>(QtByteOrder == QtBigEndian ? qt_be_inf_bytes : qt_le_inf_bytes);
}
#define Q_INFINITY (::qtInf())

// Signaling NAN
static const unsigned char qt_be_snan_bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
static const unsigned char qt_le_snan_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };
static inline double qtSnan()
{
    return *reinterpret_cast<const double *>(QtByteOrder == QtBigEndian ? qt_be_snan_bytes : qt_le_snan_bytes);
}
#define Q_SNAN (::qtSnan())

// Quiet NAN
static const unsigned char qt_be_qnan_bytes[] = { 0xff, 0xf8, 0, 0, 0, 0, 0, 0 };
static const unsigned char qt_le_qnan_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0xff };
static inline double qtQnan()
{
    return *reinterpret_cast<const double *>(QtByteOrder == QtBigEndian ? qt_be_qnan_bytes : qt_le_qnan_bytes);
}
#define Q_QNAN (::qtQnan())

static inline bool qt_compareBits(double d1, double d2)
{
    return memcmp(reinterpret_cast<const char*>(&d1), reinterpret_cast<const char*>(&d2), sizeof(double)) == 0;
}

static inline bool qIsInf(double d)
{
    return qt_compareBits(d, Q_INFINITY) || qt_compareBits(d, -Q_INFINITY);
}

static inline bool qIsNan(double d)
{
    return qt_compareBits(d, Q_QNAN) || qt_compareBits(d, Q_SNAN);
}

#endif

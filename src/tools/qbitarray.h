#ifndef QBITARRAY_H
#define QBITARRAY_H

#ifndef QT_H
#include "qbytearray.h"
#endif // QT_H

class QBitRef;
class Q_EXPORT QBitArray
{
    friend QDataStream &operator<<( QDataStream &, const QBitArray & );
    friend QDataStream &operator>>( QDataStream &, QBitArray & );
    QByteArray d;
public:
    inline QBitArray(){};
    QBitArray(int size, bool val = false);

    int size() const;
    void resize(int size);

    inline void  detach() { d.detach(); }
    inline bool isDetached() const { return d.isDetached(); }

    bool testBit(int i) const;
    void setBit(int i);
    void setBit(int i, bool val);
    void clearBit(int i);
    bool toggleBit(int i);

    const bool at(int i) const;
    QBitRef operator[](int i);
    const bool operator[](int i) const;

    QBitArray& operator&=(const QBitArray &);
    QBitArray& operator|=(const QBitArray &);
    QBitArray& operator^=(const QBitArray &);
    QBitArray  operator~() const;

    inline bool operator==(const QBitArray& a) const { return d == a.d; }
    inline bool operator!=(const QBitArray& a) const { return d != a.d; }

#ifndef QT_NO_COMPAT
    inline bool fill(bool val, int size = -1){ *this=QBitArray(size<0?d.size():size,val); return true; }
    inline bool isNull() { return d.isNull(); }
#endif

    inline bool ensure_constructed()
    { return d.ensure_constructed(); }
};

inline int QBitArray::size() const { return d.size() * 8 - *d.data(); }

QBitArray operator&(const QBitArray &, const QBitArray &);
QBitArray operator|(const QBitArray &, const QBitArray &);
QBitArray operator^(const QBitArray &, const QBitArray &);

inline bool QBitArray::testBit(int i) const
{ Q_ASSERT(i >= 0); if (i >= size()) return false;
 return (*((uchar*)d.data()+1+(i>>3)) & (1 << (i & 7))) != 0; }

inline void QBitArray::setBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 *((uchar*)d.detach()+1+(i>>3)) |= (1 << (i & 7)); }

inline void QBitArray::clearBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 *((uchar*)d.detach()+1+(i>>3)) &= ~(1 << (i & 7)); }

inline void QBitArray::setBit(int i, bool val)
{ if (val) setBit(i); else clearBit(i); }

inline bool QBitArray::toggleBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 uchar b = 1<< (i&7); uchar* p = (uchar*)d.detach()+1+(i>>3);
 uchar c = *p&b; *p^=b; return c!=0; }

inline const bool QBitArray::operator[](int i) const { return testBit(i); }
inline const bool QBitArray::at(int i) const { return testBit(i); }

class Q_EXPORT QBitRef
{
private:
    QBitArray& a;
    int i;
    inline QBitRef(QBitArray& array, int idx) : a(array), i(idx) {}
    friend class QBitArray;
public:
    inline operator bool() const { return a.testBit(i); }
    QBitRef& operator=(const QBitRef& val) { a.setBit(i, val); return *this; }
    QBitRef& operator=(bool val) { a.setBit(i, val); return *this; }
};

inline QBitRef QBitArray::operator[](int i)
{ Q_ASSERT(i >= 0); return QBitRef(*this, i); }


#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QBitArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBitArray & );
#endif


#endif // QBITARRAY_H

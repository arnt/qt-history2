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

#ifndef QDATASTREAM_H
#define QDATASTREAM_H

#include "QtCore/qiodevice.h"
#include "QtCore/qglobal.h"

class QByteArray;
class QIODevice;

template<typename T> class QList;
template<typename T> class QLinkedList;
template<typename T> class QVector;
template<class Key, class T> class QHash;
template<class Key, class T> class QMap;

class QDataStreamPrivate;

#ifndef QT_NO_DATASTREAM
class Q_CORE_EXPORT QDataStream
{
public:
    enum Version {
        Qt_1_0 = 1,
        Qt_2_0 = 2,
        Qt_2_1 = 3,
        Qt_3_0 = 4,
        Qt_3_1 = 5,
        Qt_3_3 = 6,
        Qt_4_0 = 7
#if QT_VERSION >= 0x040100
#error "Add Qt_4_1 = Qt_4_0"
#endif
    };

    enum ByteOrder {
        BigEndian = QSysInfo::BigEndian,
        LittleEndian = QSysInfo::LittleEndian
    };

#ifdef Status
#error This file has to be included after any system files that define Status
#endif
    enum Status {
        Ok,
        ReadPastEnd,
	ReadCorruptData
    };

    QDataStream();
    explicit QDataStream(QIODevice *);
#ifdef QT_COMPAT
    QDataStream(QByteArray *, int mode);
#endif
    explicit QDataStream(QByteArray *, QIODevice::OpenMode flags);
    explicit QDataStream(const QByteArray &);
    virtual ~QDataStream();

    QIODevice *device() const;
    void setDevice(QIODevice *);
    void unsetDevice();

    bool atEnd() const;
#ifdef QT_COMPAT
    inline QT_COMPAT bool eof() const { return atEnd(); }
#endif

    Status status() const;
    void setStatus(Status status);
    void resetStatus();

    ByteOrder byteOrder() const;
    void setByteOrder(ByteOrder);

    int version() const;
    void setVersion(int);

    QDataStream &operator>>(Q_INT8 &i);
    QDataStream &operator>>(Q_UINT8 &i);
    QDataStream &operator>>(Q_INT16 &i);
    QDataStream &operator>>(Q_UINT16 &i);
    QDataStream &operator>>(Q_INT32 &i);
    QDataStream &operator>>(Q_UINT32 &i);
    QDataStream &operator>>(Q_INT64 &i);
    QDataStream &operator>>(Q_UINT64 &i);

    QDataStream &operator>>(float &f);
    QDataStream &operator>>(double &f);
#ifdef QT_USE_FIXED_POINT
    inline QDataStream &operator>>(QFixedPoint &f) { double d; operator>>(d); f = d; return *this; }
#endif
    QDataStream &operator>>(char *&str);

    QDataStream &operator<<(Q_INT8 i);
    QDataStream &operator<<(Q_UINT8 i);
    QDataStream &operator<<(Q_INT16 i);
    QDataStream &operator<<(Q_UINT16 i);
    QDataStream &operator<<(Q_INT32 i);
    QDataStream &operator<<(Q_UINT32 i);
    QDataStream &operator<<(Q_INT64 i);
    QDataStream &operator<<(Q_UINT64 i);
    QDataStream &operator<<(float f);
    QDataStream &operator<<(double f);
#ifdef QT_USE_FIXED_POINT
    inline QDataStream &operator<<(QFixedPoint f) { return operator<<(f.toDouble()); }
#endif
    QDataStream &operator<<(const char *str);

    QDataStream &readBytes(char *&, uint &len);
    int readRawData(char *, int len);

    QDataStream &writeBytes(const char *, uint len);
    int writeRawData(const char *, int len);

#ifdef QT_COMPAT
    inline QT_COMPAT QDataStream &readRawBytes(char *str, uint len)
        { readRawData(str, (int)len); return *this; }
    inline QT_COMPAT QDataStream &writeRawBytes(const char *str, uint len)
        { writeRawData(str, (int)len); return *this; }
    inline QT_COMPAT bool isPrintableData() const { return false; }
    inline QT_COMPAT void setPrintableData(bool) {}
#endif

private:
    Q_DISABLE_COPY(QDataStream)

    QDataStreamPrivate *d;

    QIODevice *dev;
    bool owndev;
    bool noswap;
    ByteOrder byteorder;
    int ver;
    Status q_status;
};


/*****************************************************************************
  QDataStream inline functions
 *****************************************************************************/

inline QIODevice *QDataStream::device() const
{ return dev; }

inline QDataStream::ByteOrder QDataStream::byteOrder() const
{ return byteorder; }

inline int QDataStream::version() const
{ return ver; }

inline void QDataStream::setVersion(int v)
{ ver = v; }

inline QDataStream &QDataStream::operator>>(Q_UINT8 &i)
{ return *this >> reinterpret_cast<Q_INT8&>(i); }

inline QDataStream &QDataStream::operator>>(Q_UINT16 &i)
{ return *this >> reinterpret_cast<Q_INT16&>(i); }

inline QDataStream &QDataStream::operator>>(Q_UINT32 &i)
{ return *this >> reinterpret_cast<Q_INT32&>(i); }

inline QDataStream &QDataStream::operator>>(Q_UINT64 &i)
{ return *this >> reinterpret_cast<Q_INT64&>(i); }

inline QDataStream &QDataStream::operator<<(Q_UINT8 i)
{ return *this << Q_INT8(i); }

inline QDataStream &QDataStream::operator<<(Q_UINT16 i)
{ return *this << Q_INT16(i); }

inline QDataStream &QDataStream::operator<<(Q_UINT32 i)
{ return *this << Q_INT32(i); }

inline QDataStream &QDataStream::operator<<(Q_UINT64 i)
{ return *this << Q_INT64(i); }

template <typename T>
QDataStream& operator>>(QDataStream& s, QList<T>& l)
{
    l.clear();
    Q_UINT32 c;
    s >> c;
    for(Q_UINT32 i = 0; i < c; ++i)
    {
        T t;
        s >> t;
        l.append(t);
        if (s.atEnd())
            break;
    }
    return s;
}

template <typename T>
QDataStream& operator<<(QDataStream& s, const QList<T>& l)
{
    s << Q_UINT32(l.size());
    for (int i = 0; i < l.size(); ++i)
        s << l.at(i);
    return s;
}

template <typename T>
QDataStream& operator>>(QDataStream& s, QLinkedList<T>& l)
{
    l.clear();
    Q_UINT32 c;
    s >> c;
    for(Q_UINT32 i = 0; i < c; ++i)
    {
        T t;
        s >> t;
        l.append(t);
        if (s.atEnd())
            break;
    }
    return s;
}

template <typename T>
QDataStream& operator<<(QDataStream& s, const QLinkedList<T>& l)
{
    s << Q_UINT32(l.size());
    typename QLinkedList<T>::ConstIterator it = l.constBegin();
    for(; it != l.constEnd(); ++it)
        s << *it;
    return s;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QHash<Key, T> &hash)
{
    QDataStream::Status oldStatus = in.status();
    in.resetStatus();
    hash.clear();

    Q_UINT32 n;
    in >> n;

    for (Q_UINT32 i = 0; i < n; ++i) {
        if (in.status() != QDataStream::Ok)
            break;

        Key k;
        T t;
        in >> k >> t;
        hash.insert(k, t);
    }

    if (in.status() != QDataStream::Ok)
        hash.clear();
    if (oldStatus != QDataStream::Ok)
        in.setStatus(oldStatus);
    return in;
}

template<typename T>
QDataStream& operator>>(QDataStream& s, QVector<T>& v)
{
    v.clear();
    Q_UINT32 c;
    s >> c;
    v.resize(c);
    for(Q_UINT32 i = 0; i < c; ++i) {
        T t;
        s >> t;
        v[i] = t;
    }
    return s;
}

template<typename T>
QDataStream& operator<<(QDataStream& s, const QVector<T>& v)
{
    s << Q_UINT32(v.size());
    const T* it = v.begin();
    for(; it != v.end(); ++it)
        s << *it;
    return s;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QHash<Key, T>& hash)
{
    out << Q_UINT32(hash.size());
    typename QHash<Key, T>::ConstIterator it = hash.begin();
    while (it != hash.end()) {
        out << it.key() << it.value();
        ++it;
    }
    return out;
}
#ifdef qdoc
template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMap<Key, T> &map)
#else
template <class aKey, class aT>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMap<aKey, aT> &map)
#endif
{
    QDataStream::Status oldStatus = in.status();
    in.resetStatus();
    map.clear();

    Q_UINT32 n;
    in >> n;

    map.detach();
#if !defined(Q_CC_BOR)
    map.d->insertInOrder = true;
#endif
    for (Q_UINT32 i = 0; i < n; ++i) {
        if (in.status() != QDataStream::Ok)
            break;

        aKey key;
        aT value;
        in >> key >> value;
        map.insert(key, value);
    }
#if !defined(Q_CC_BOR)
    map.d->insertInOrder = false;
#endif
    if (in.status() != QDataStream::Ok)
        map.clear();
    if (oldStatus != QDataStream::Ok)
        in.setStatus(oldStatus);
    return in;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QMap<Key, T> &map)
{
    out << Q_UINT32(map.size());
    typename QMap<Key, T>::ConstIterator it = map.begin();
    while (it != map.end()) {
        out << it.key() << it.value();
        ++it;
    }
    return out;
}

#endif // QT_NO_DATASTREAM

#endif // QDATASTREAM_H

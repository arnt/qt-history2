/****************************************************************************
**
** Definition of some shared interal classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QINTERNAL_P_H
#define QINTERNAL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//
#ifndef QT_H
#include "qnamespace.h"
#include "qlist.h"
#include "qiodevice.h"
#include "qbytearray.h"
#endif // QT_H

class QWidget;
class QPainter;
class QPixmap;

class Q_CORE_EXPORT QMembuf
{
public:
    QMembuf();
    ~QMembuf();

    void append(QByteArray *ba);
    void clear();

    bool consumeBytes(Q_ULONG nbytes, char *sink);
    QByteArray readAll();
    bool scanNewline(QByteArray *store);
    bool canReadLine() const;

    int ungetch(int ch);

    QIODevice::Offset size() const;

private:

    QList<QByteArray *> buf;
    QIODevice::Offset _size;
    QIODevice::Offset _index;
};

inline void QMembuf::append(QByteArray *ba)
{ buf.append(ba); _size += ba->size(); }

inline void QMembuf::clear()
{ buf.clear(); _size=0; _index=0; }

inline QByteArray QMembuf::readAll()
{ QByteArray ba; ba.resize(_size); consumeBytes(_size,ba.data()); return ba; }

inline bool QMembuf::canReadLine() const
{ return ((QMembuf*)this)->scanNewline(0); }

inline QIODevice::Offset QMembuf::size() const
{ return _size; }



class QVirtualDestructor {
public:
    virtual ~QVirtualDestructor() {}
};

template <class T>
class QAutoDeleter : public QVirtualDestructor {
public:
    QAutoDeleter(T *p) : ptr(p) {}
    ~QAutoDeleter() { delete ptr; }
    T *data() const { return ptr; }
private:
    T *ptr;
};

template <class T>
T* qAutoDeleterData(QAutoDeleter<T> *ad)
{
    if (!ad)
       return 0;
    return ad->data();
}

template <class T>
QAutoDeleter<T> *qAutoDeleter(T *p)
{
    return new QAutoDeleter<T>(p);
}

#endif // QINTERNAL_P_H

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSHAREDDATA_H
#define QSHAREDDATA_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

template <class T> class QSharedDataPointer;

class Q_CORE_EXPORT QSharedData
{
public:
    QAtomicInt ref;

    inline QSharedData() : ref(0) { }
    inline QSharedData(const QSharedData &) : ref(0) { }

private:
    // using the assignment operator would lead to corruption in the ref-counting
    QSharedData &operator=(const QSharedData &);
};

template <class T> class QSharedDataPointer
{
public:
    inline void detach() { if (d && d->ref != 1) detach_helper(); }
    inline T &operator*() { detach(); return *d; }
    inline const T &operator*() const { return *d; }
    inline T *operator->() { detach(); return d; }
    inline const T *operator->() const { return d; }
    inline operator T *() { detach(); return d; }
    inline operator const T *() const { return d; }
    inline T *data() { detach(); return d; }
    inline const T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline bool operator==(const QSharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const QSharedDataPointer<T> &other) const { return d != other.d; }

    inline QSharedDataPointer() { d = 0; }
    inline ~QSharedDataPointer() { if (d && !d->ref.deref()) delete d; }

    explicit QSharedDataPointer(T *data);
    inline QSharedDataPointer(const QSharedDataPointer<T> &o) : d(o.d) { if (d) d->ref.ref(); }
    inline QSharedDataPointer<T> & operator=(const QSharedDataPointer<T> &o) {
        if (o.d != d) {
            if (o.d)
                o.d->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o.d;
        }
        return *this;
    }
    inline QSharedDataPointer &operator=(T *o) {
        if (o != d) {
            if (o)
                o->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o;
        }
        return *this;
    }

    inline bool operator!() const { return !d; }

private:
    void detach_helper();

    T *d;
};

template <class T> class QExplicitlySharedDataPointer
{
public:
    typedef T Type;

    inline T &operator*() { return *d; }
    inline const T &operator*() const { return *d; }
    inline T *operator->() { return d; }
    inline const T *operator->() const { return d; }
    inline operator T *() { return d; }
    inline operator const T *() const { return d; }
    inline T *data() { return d; }
    inline const T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline void detach() { if (d && d->ref != 1) detach_helper(); }

    //inline operator bool () const { return d != 0; }

    inline bool operator==(const QExplicitlySharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const QExplicitlySharedDataPointer<T> &other) const { return d != other.d; }
    inline bool operator==(const T *ptr) const { return d == ptr; }
    inline bool operator!=(const T *ptr) const { return d != ptr; }

    inline QExplicitlySharedDataPointer() { d = 0; }
    inline ~QExplicitlySharedDataPointer() { if (d && !d->ref.deref()) delete d; }

    explicit QExplicitlySharedDataPointer(T *data);
    inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<T> &o) : d(o.d) { if (d) d->ref.ref(); }
    inline QExplicitlySharedDataPointer<T> & operator=(const QExplicitlySharedDataPointer<T> &o) {
        if (o.d != d) {
            if (o.d)
                o.d->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o.d;
        }
        return *this;
    }
    inline QExplicitlySharedDataPointer &operator=(T *o) {
        if (o != d) {
            if (o)
                o->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o;
        }
        return *this;
    }

    inline bool operator!() const { return !d; }

private:
    void detach_helper();

    T *d;
};

template <class T>
Q_INLINE_TEMPLATE QSharedDataPointer<T>::QSharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedDataPointer<T>::detach_helper()
{
    T *x = new T(*d);
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

template <class T>
Q_OUTOFLINE_TEMPLATE void QExplicitlySharedDataPointer<T>::detach_helper()
{
    T *x = new T(*d);
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

template <class T>
Q_INLINE_TEMPLATE QExplicitlySharedDataPointer<T>::QExplicitlySharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSHAREDDATA_H

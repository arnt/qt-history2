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

#ifndef QSHAREDDATA_H
#define QSHAREDDATA_H

#include <QtCore/qatomic.h>
#include <QtCore/qglobal.h>

template <class T> class QSharedDataPointer;

class Q_CORE_EXPORT QSharedData
{
public:
    QAtomic ref;

    inline QSharedData() : ref(0) { }
    inline QSharedData(const QSharedData &) : ref(0) { }

private:
    // using the assignment operator would lead to corruption in the ref-counting
    QSharedData &operator=(const QSharedData &);
};

template <class T> class QExplicitlySharedDataPointer
{
public:
    inline T *operator->() { return d; }
    inline const T *operator->() const { return d; }
    inline operator T *() { return d; }
    inline operator const T *() const { return d; }
    inline T *data() { return d; }
    inline const T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline QExplicitlySharedDataPointer() { d = 0; }
    inline ~QExplicitlySharedDataPointer() { if (d && !--d->ref) delete d; }

    explicit inline QExplicitlySharedDataPointer(T *data) : d(data) { if (d) ++d->ref; }
    inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer &o) : d(o.d)
    { if (d) ++d->ref; }
    inline QExplicitlySharedDataPointer &operator=(const QExplicitlySharedDataPointer &o) {
        if (o.d != d) {
            T *x = o.d;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;
    }
    inline QExplicitlySharedDataPointer &operator=(T *o) {
        if (o != d) {
            T *x = o;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;
    }

    inline bool operator!() const { return !d; }

private:
    T *d;
};

template <class T> class QSharedDataPointer
{
public:
    inline void detach() { if (d && d->ref != 1) detach_helper(); }
    inline T *operator->() { detach(); return d; }
    inline const T *operator->() const { return d; }
    inline operator T *() { detach(); return d; }
    inline operator const T *() const { return d; }
    inline T *data() { detach(); return d; }
    inline const T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline QSharedDataPointer() { d = 0; }
    inline ~QSharedDataPointer() { if (d && !--d->ref) delete d; }

    explicit QSharedDataPointer(T *data) : d(data) { if (d) ++d->ref; }
    inline QSharedDataPointer(const QSharedDataPointer &o) : d(o.d) { if (d) ++d->ref; }
    inline QSharedDataPointer & operator=(const QSharedDataPointer &o) {
        if (o.d != d) {
            T *x = o.d;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;
    }
    inline QSharedDataPointer &operator=(T *o) {
        if (o != d) {
            T *x = o;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;
    }

    inline bool operator!() const { return !d; }

private:
    void detach_helper();

    T *d;
};

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedDataPointer<T>::detach_helper()
{
    T *x = new T(*d);
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;
}

#endif // QSHAREDDATA_H

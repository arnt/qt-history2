#ifndef QSHAREDDATAPOINTER_H
#define QSHAREDDATAPOINTER_H

#ifndef QT_H
#include <qatomic.h>
#include <qglobal.h>
#endif

template <class T> class QSharedDataPointer;

class Q_CORE_EXPORT QSharedObject
{
public:
    QAtomic ref;

    QSharedObject() { ref = 0; }
    QSharedObject(const QSharedObject &) { ref = 0; }

private:
    // using the assignment operator would lead to corruption in the refcounting
    QSharedObject &operator=(const QSharedObject &);
};

template <class T> class QExplicitlySharedDataPointer
{
protected:
    T *d;
public:
    T * operator->() { return d; }
    const T * operator->() const { return d; }
    operator T *() { return d; }
    operator const T *() const { return d; }
    T * data() { return d; }
    const T * data() const { return d; }
    const T * constData() const { return d; }

    QExplicitlySharedDataPointer() { d = 0; }
    ~QExplicitlySharedDataPointer() { if (d && !--d->ref) delete d; }

    Q_EXPLICIT QExplicitlySharedDataPointer(T *data) : d(data) { if (d) ++d->ref; }
    QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer &o) : d(o.d) { if (d) ++d->ref; }
    QExplicitlySharedDataPointer & operator=(const QExplicitlySharedDataPointer &o) {
        if (o.d != d) {
            T *x = o.d;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;
    }
    QExplicitlySharedDataPointer &operator=(T *o) {
        if (o != d) {
            T *x = o;
            if (x) ++x->ref;
            x = qAtomicSetPtr(&d, x);
            if (x && !--x->ref)
                delete x;
        }
        return *this;

    }

    bool operator!() const { return !d; }
    bool isNull() const { return !d; }
};

template<class T> class QSharedDataPointer : private QExplicitlySharedDataPointer<T>
{
public:
    T * operator->() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    const T * operator->() const { return this->d; }
    operator T *() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    operator const T *() const { return this->d; }
    T * data() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    const T * data() const { return this->d; }
    const T * constData() const { return this->d; }

    QSharedDataPointer() : QExplicitlySharedDataPointer<T>() {}
    ~QSharedDataPointer() {}

    Q_EXPLICIT QSharedDataPointer(T *data) : QExplicitlySharedDataPointer<T>(data) {}
    QSharedDataPointer(const QSharedDataPointer<T> &o) : QExplicitlySharedDataPointer<T>(o) {}

    QSharedDataPointer & operator=(const QSharedDataPointer<T> &o) {
        QExplicitlySharedDataPointer<T>::operator=(o);
        return *this;
    }
    QSharedDataPointer &operator=(T *o) {
        QExplicitlySharedDataPointer<T>::operator=(o);
        return *this;
    }

    bool operator!() const { return !this->d; }
    void detach();
};

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedDataPointer<T>::detach()
{
    if (!this->d) return;
    T *x = new T(*this->d);
    ++x->ref;
    x = qAtomicSetPtr(&this->d, x);
    if (!--x->ref)
        delete x;
}

#endif

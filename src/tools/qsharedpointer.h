#ifndef QSHAREDPOINTER_H
#define QSHAREDPOINTER_H

#ifndef QT_H
#include <qatomic.h>
#include <qglobal.h>
#endif

template <class T> class QSharedPointer;

class Q_CORE_EXPORT QSharedObject
{
public:
    QAtomic ref;

    QSharedObject() { ref = 0; }
    QSharedObject(const QSharedObject &) { ref = 0; }
private:
    // using the assignment operator would lead to corruption in the refcounting.
    QSharedObject &operator=(const QSharedObject &);
};

template <class T> class QExplicitSharedPointer
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

    QExplicitSharedPointer() { d = 0; }
    ~QExplicitSharedPointer() { if (d && !--d->ref) delete d; }

    Q_EXPLICIT QExplicitSharedPointer(T *data) : d(data) { if (d) ++d->ref; }
    QExplicitSharedPointer(const QExplicitSharedPointer &o) : d(o.d) { if (d) ++d->ref; }
    QExplicitSharedPointer & operator=(const QExplicitSharedPointer &o) {
	if (o.d != d) {
	    T *x = o.d;
	    if (x) ++x->ref;
	    x = qAtomicSetPtr(&d, x);
	    if (x && !--x->ref)
		delete x;
	}
	return *this;
    }
    QExplicitSharedPointer &operator=(T *o) {
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

template<class T> class QSharedPointer : public QExplicitSharedPointer<T>
{
public:
    T * operator->() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    const T * operator->() const { return this->d; }
    operator T *() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    operator const T *() const { return this->d; }
    T * data() { if (this->d && this->d->ref != 1) detach(); return this->d; }
    const T * data() const { return this->d; }

    QSharedPointer() : QExplicitSharedPointer<T>() {}

    Q_EXPLICIT QSharedPointer(T *data) : QExplicitSharedPointer<T>(data) {}
    QSharedPointer(const QSharedPointer<T> &o) : QExplicitSharedPointer<T>(o) {}

    QSharedPointer & operator=(const QSharedPointer<T> &o) {
	QExplicitSharedPointer<T>::operator=(o);
	return *this;
    }
    QSharedPointer &operator=(T *o) {
	QExplicitSharedPointer<T>::operator=(o);
	return *this;
    }

    bool operator!() const { return !this->d; }
    void detach();
};

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedPointer<T>::detach()
{
    if (!this->d) return;
    T *x = new T(*this->d);
    ++x->ref;
    x = qAtomicSetPtr(&this->d, x);
    if (!--x->ref)
	delete x;
}

#endif

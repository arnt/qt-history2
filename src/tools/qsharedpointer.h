#ifndef QSHAREDPOINTER_H
#define QSHAREDPOINTER_H

#ifndef QT_H
#include <qatomic.h>
#include <qglobal.h>
#endif

template <class T> class QSharedPointer;

class QSharedObject
{
public:
    QAtomic ref;

    QSharedObject() { ref = 0; }
    QSharedObject(const QSharedObject &o) { ref = 0; }
private:
    // using the assignment operator would lead to corruption in the refcounting.
    QSharedObject &operator=(const QSharedObject &);
};

template <class T> class QSharedPointer
{
protected:
    QSharedObject *d;
public:
    T * operator->() { if (d && d->ref != 1) detach(); return static_cast<T *>(d); }
    const T * operator->() const { return static_cast<T *>(d); }
    operator T *() { if (d && d->ref != 1) detach(); return static_cast<T *>(d); }
    operator const T *() const { return static_cast<T *>(d); }

    QSharedPointer() { d = 0; }
    ~QSharedPointer() { if (d && --d->ref) delete d; }

    Q_EXPLICIT QSharedPointer(T *data) { d = data; ++d->ref; }
    QSharedPointer(const QSharedPointer &o) { d = o.d; ++d->ref; }
    QSharedPointer & operator=(const QSharedPointer &o) {
	if (o.d != d) {
	    QSharedObject *x = o.d;
	    if (x) ++x->ref;
	    x = qAtomicSetPtr(&d, x);
	    if (x && !--x->ref)
		delete x;
	}
	return *this;
    }
    QSharedPointer &operator=(T *o) {
	if (o != d) {
	    QSharedObject *x = o;
	    if (x) ++x->ref;
	    x = qAtomicSetPtr(&d, x);
	    if (x && !--x->ref)
		delete x;
	}
	return *this;

    }
    void detach();
};

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedPointer<T>::detach()
{
    if (!d) return;
    T *x = new T(*static_cast<T *>(d));
    x = qAtomicSetPointer(&d, x);
    if (!--x->ref)
	delete x;
}
 
template<class T> class QExplicitSharedPointer : public QSharedPointer<T>
{
public:
    T * operator->() { return static_cast<T *>(d); }
    const T * operator->() const { return static_cast<T *>(d); }
    operator T *() { return static_cast<T *>(d); }
    operator const T *() const { return static_cast<T *>(d); }
    QExplicitSharedPointer() {}

    Q_EXPLICIT QExplicitSharedPointer(T *data) : QSharedPointer<T>(data) {}
    QExplicitSharedPointer(const QSharedPointer<T> &o) : QSharedPointer<T>(o) {}

    QExplicitSharedPointer & operator=(const QSharedPointer<T> &o) {
	QSharedPointer<T>::operator=(o);
	return *this;
    }
    QExplicitSharedPointer &operator=(T *o) {
	QSharedPointer<T>::operator=(o);
	return *this;
    }

};


#endif

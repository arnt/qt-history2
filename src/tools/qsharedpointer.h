#ifndef QSHAREDPOINTER_H
#define QSHAREDPOINTER_H

#ifndef QT_H
#include <qatomic.h>
#include <qglobal.h>
#endif

template class<T>class QSharedPointer;

class QSharedObject
{
    friend template class<T>class QSharedPtr;
    QAtomic ref;

public:
    QSharedObject() : ref(0) {}
    QSharedObject(const QSharedObject &o) : ref(0) {}
private:
    // using the assignment operator would lead to corruption in the refcounting.
    QSharedObject &operator=(const QSharedObject &);
};

template class QSharedPointer
{
protected:
    T *d;
public:
    T * operator->() { if (d && d->ref != 1) detach(); return d; }
    const T * operator->() const { return d; }
    QSharedPointer() { d = 0; }
    ~QSharedPointer() { if (d && --d->ref) delete d; }

    Q_EXPLICIT QSharedPointer(Data *data) { d = data; ++d->ref; }
    QSharedPointer(const QSharedPointer &o) { d = o.d; ++d->ref; }
    QSharedPointer & operator=(const QSharedPointer &o) {
	if (o.d != d) {
	    T *x = o.d;
	    if (x) ++x->ref;
	    x = qAtomicSetPointer(&d, x);
	    if (x && !--x->ref)
		delete x;
	}
	return *this;
    }
    void detach() {
	if (!d) return;
	T *x = new T(*d);
	x = qAtomicSetPointer(&d, x);
	if (!--x->ref)
	    delete x;
    }
};

template<class T> class QExplicitSharedPointer : public QSharedPointer<T>
{
    T * operator->() { return d; }
    QExplicitSharedPointer() {}

    Q_EXPLICIT QSharedPointer(Data *data) : QSharedPointer(data) {}
    QSharedPointer(const QSharedPointer &o) : QSharedPointer(o) {}
}


#endif

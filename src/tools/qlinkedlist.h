#ifndef QLINKEDLIST_H
#define QLINKEDLIST_H

#ifndef QT_H
#include "qglobal.h"
#include "qiterator.h"
#include "qatomic.h"
#endif // QT_H

struct QLinkedListData
{
    QLinkedListData *n, *p;
    QAtomic ref;
    int size;
    void *autoDelete;
    static QLinkedListData shared_null;
};

template <class T>
class QLinkedList
{
    struct Node { inline Node(const T &t): t(t){} Node *n, *p; T t; };
    union { QLinkedListData *d; Node *e; };

public:
    inline QLinkedList() : d(&QLinkedListData::shared_null) { ++d->ref; };
    inline QLinkedList(const QLinkedList &l):d(l.d) { ++d->ref; }
    ~QLinkedList();
    QLinkedList<T> &operator=(const QLinkedList &);
    bool operator==(const QLinkedList &l) const;
    inline bool operator!=(const QLinkedList &l) const { return !(*this == l); }

    bool autoDelete() const;
    void setAutoDelete(bool enable);

    inline int size() const { return d->size; }
    inline bool isEmpty() const { return d->size == 0; }
    inline void detach()
    { if ( d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline bool operator!() const { return d->size == 0; }

    void clear();

    void append(const T &);
    void prepend(const T &);

    int remove(const T &t);
    int take(const T &t);
    bool contains(const T &t) const;
    int count(const T &t) const;

    class Iterator
    {
	Node *i;
    public:
	typedef std::bidirectional_iterator_tag  iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	inline operator Node *() const { return i; }
	inline Iterator(Node *n = 0): i(n){}
	inline T &operator*() { return i->t; }
	inline Iterator operator++() { i = i->n; return *this; }
	inline Iterator operator++(int) { Node *n = i; i = i->n; return n; }
	inline Iterator operator--() { i = i->p; return *this; }
	inline Iterator operator--(int) { Node *n = i; i = i->p; return n; }
    };
    friend class Iterator;

    class ConstIterator
    {
	Node *i;
    public:
	typedef std::bidirectional_iterator_tag  iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	inline operator Node *() const { return i; }
	inline ConstIterator(Iterator ci): i((Node*)ci){}
	inline ConstIterator(Node *n = 0): i(n){}
	inline const T &operator*() const { return i->t; }
	inline ConstIterator operator++() { i = i->n; return *this; }
	inline ConstIterator operator++(int) { Node *n = i; i = i->n; return n; }
	inline ConstIterator operator--() { i = i->p; return *this; }
	inline ConstIterator operator--(int) { Node *n = i; i = i->p; return n; }
    };
    friend class ConstIterator;

    inline Iterator begin() { detach(); return e->n; }
    inline ConstIterator begin() const { return e->n; }
    inline ConstIterator constBegin() const { return e->n; }
    inline Iterator end() { detach(); return e; }
    inline ConstIterator end() const { return e; }
    inline ConstIterator constEnd() const { return e; }

    inline Iterator erase(Iterator it);
    Iterator insert(Iterator before, const T &t);

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline void push_back(const T &t) { append(t); }
    inline void push_front(const T &t) { prepend(t); }

#ifndef QT_NO_COMPAT
    // compatibility
    inline Iterator remove(Iterator it) { return erase(it); }
    inline int count() const { return d->size; }
#endif

    // comfort
    QLinkedList &operator+=(const QLinkedList &l);
    QLinkedList operator+(const QLinkedList &l) const;
    inline void operator+=(const T &t) { append(t); }
    inline QLinkedList &operator<< (const T &t) { append(t); return *this; }

    inline bool ensure_constructed()
    { if (!d) { d = &QLinkedListData::shared_null; ++d->ref; return false; } return true; }

private:
    inline bool canAutoDelete() const { return QTypeInfo<T>::isPointer; }
    void detach_helper();
    void free(QLinkedListData*);
};

template <class T>
inline QLinkedList<T>::~QLinkedList()
{
    if (!--d->ref  || (QTypeInfo<T>::isPointer && d->autoDelete == this))
	free(d);
}

template <class T>
void QLinkedList<T>::detach_helper() {
    union { QLinkedListData *d; Node *e; } x;
    x.d = new QLinkedListData;
    x.d->ref = 1;
    x.d->size = d->size;
    x.d->autoDelete = d->autoDelete;
    Node *i = e->n, *j = x.e;
    while (i != e) {
	j->n = new Node(i->t);
	j->n->p = j;
	i = i->n;
	j = j->n;
    }
    j->n = x.e;
    x.e->p = j;
    x.d = qAtomicSetPtr(&d, x.d);
    if (!--x.d->ref)
 	free(x.d);
}

template <class T>
void QLinkedList<T>::free(QLinkedListData *d)
{
    Node *e = (Node*)d;
    Node *i = e->n;
    if (QTypeInfo<T>::isPointer && d->autoDelete == this) {
    	while(i != e) {
	    qDelete(i->t);
	    i = i->n;
	}
	i = e->n;
	d->autoDelete = 0;
    }
    if ( d->ref == 0 ) {
	while(i != e) {
	    Node *n = i;
	    i = i->n;
	    delete n;
	}
	delete d;
    }
}

template <class T>
void QLinkedList<T>::clear()
{
    bool wasAutoDelete = d->autoDelete == this;
    *this = QLinkedList<T>();
    if (QTypeInfo<T>::isPointer && wasAutoDelete)
	setAutoDelete(wasAutoDelete);
}

template <class T>
inline bool QLinkedList<T>::autoDelete() const
{ return d->autoDelete == (void*) this; }

template <class T>
inline void QLinkedList<T>::setAutoDelete(bool enable)
{
    Q_ASSERT(canAutoDelete());
    if (QTypeInfo<T>::isPointer) {
	detach();
	d->autoDelete = enable ? this : 0;
    }
}

template <class T>
QLinkedList<T> &QLinkedList<T>::operator=(const QLinkedList<T> &l)
{
    if ( d != l.d ) {
	QLinkedListData *x = l.d;
	++x->ref;
	x = qAtomicSetPtr( &d, x );
	if (!--x->ref || (QTypeInfo<T>::isPointer && x->autoDelete == this))
	    free(x);
    }
    return *this;
}

template <class T>
bool QLinkedList<T>::operator== (const QLinkedList<T> &l) const
{
    if (d->size != l.d->size)
	return false;
    if (e == l.e)
	return true;
    Node *i = e->n;
    Node *il = l.e->n;
    while (i != e) {
	if (! (i->t == il->t))
	    return false;
	i = i->n;
	il = il->n;
    }
    return true;
}

template <class T>
void QLinkedList<T>::append(const T &t)
{
    detach();
    Node *i = new Node(t);
    i->n = e;
    i->p = e->p;
    i->p->n = i;
    e->p = i;
    d->size++;
}

template <class T>
void QLinkedList<T>::prepend(const T &t)
{
    detach();
    Node *i = new Node(t);
    i->n = e->n;
    i->p = e;
    i->n->p = i;
    e->n = i;
    d->size++;
}

template <class T>
int QLinkedList<T>::remove(const T &t)
{
    detach();
    Node *i = e->n;
    int c = 0;
    while (i != e) {
	if (i->t == t) {
	    Node *n = i;
	    i->n->p = i->p;
	    i->p->n = i->n;
	    i = i->n;
	    if (QTypeInfo<T>::isPointer && d->autoDelete == this)
		qDelete(n->t);
	    delete n;
	    c++;
	} else {
	    i = i->n;
	}
    }
    d->size-=c;
    return c;
}

template <class T>
int QLinkedList<T>::take(const T &t)
{
    detach();
    Node *i = e->n;
    int c = 0;
    while (i != e) {
	if (i->t == t) {
	    Node *n = i;
	    i->n->p = i->p;
	    i->p->n = i->n;
	    i = i->n;
	    delete n;
	    c++;
	} else {
	    i = i->n;
	}
    }
    d->size-=c;
    return c;
}

template <class T>
bool QLinkedList<T>::contains(const T &t) const
{
    Node *i = e;
    while ((i = i->n) != e)
	if (i->t == t)
	    return true;
    return false;
}

template <class T>
int QLinkedList<T>::count(const T &t) const
{
    Node *i = e;
    int c = 0;
    while ((i = i->n) != e)
	if (i->t == t)
	    c++;
    return c;
}


template <class T>
typename QLinkedList<T>::Iterator QLinkedList<T>::insert(Iterator pos, const T &t)
{
    Node *i = pos;
    Node *m = new Node(t);
    m->n = i;
    m->p = i->p;
    m->p->n = m;
    i->p = m;
    d->size++;
    return m;
}


template <class T>
typename QLinkedList<T>::Iterator QLinkedList<T>::erase(Iterator pos)
{
    detach();
    Node *i = pos;
    if (i != e) {
	Node *n = i;
	i->n->p = i->p;
	i->p->n = i->n;
	i = i->n;
	if (QTypeInfo<T>::isPointer && d->autoDelete == this)
	    qDelete(n->t);
	delete n;
	d->size--;
    }
    return i;
}

template <class T>
QLinkedList<T> &QLinkedList<T>::operator+=(const QLinkedList<T> &l)
{
    detach();
    int n = l.d->size;
    d->size += n;
    Node *o = l.e->n;
    while (n--) {
	Node *i = new Node(o->t);
	o = o->n;
	i->n = e;
	i->p = e->p;
	i->p->n = i;
	e->p = i;
    }
    return *this;
}

template <class T>
QLinkedList<T> QLinkedList<T>::operator+(const QLinkedList<T> &l) const
{
    QLinkedList<T> n = *this;
    n += l;
    return n;
}

Q_DECLARE_ITERATOR(QLinkedList)

#endif // QLINKEDLIST_H

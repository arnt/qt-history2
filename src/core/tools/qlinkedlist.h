#ifndef QLINKEDLIST_H
#define QLINKEDLIST_H

#ifndef QT_H
#include "qglobal.h"
#include "qiterator.h"
#include "qatomic.h"
#endif // QT_H

struct Q_CORE_EXPORT QLinkedListData
{
    QLinkedListData *n, *p;
    QAtomic ref;
    int size;

    static QLinkedListData shared_null;
};

template <typename T>
struct QLinkedListNode
{
    inline QLinkedListNode(const T &arg): t(arg) { }
    QLinkedListNode *n, *p;
    T t;
};

template <class T>
class QLinkedList
{
    typedef QLinkedListNode<T> Node;
    union { QLinkedListData *d; QLinkedListNode<T> *e; };

public:
    inline QLinkedList() : d(&QLinkedListData::shared_null) { ++d->ref; };
    inline QLinkedList(const QLinkedList &l):d(l.d) { ++d->ref; }
    ~QLinkedList();
    QLinkedList<T> &operator=(const QLinkedList &);
    bool operator==(const QLinkedList &l) const;
    inline bool operator!=(const QLinkedList &l) const { return !(*this == l); }

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
    public:
	typedef std::bidirectional_iterator_tag  iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	Node *i;
	inline Iterator(Node *n = 0): i(n){}
	inline Iterator(const Iterator &o): i(o.i){}
	inline T &operator*() { return i->t; }
	inline bool operator==(const Iterator &o) const { return i == o.i; }
	inline bool operator!=(const Iterator &o) const { return i != o.i; }
	inline Iterator operator++() { i = i->n; return *this; }
	inline Iterator operator++(int) { Node *n = i; i = i->n; return n; }
	inline Iterator operator--() { i = i->p; return *this; }
	inline Iterator operator--(int) { Node *n = i; i = i->p; return n; }
	inline Iterator operator+(int j) const { Node *n = i; while (j--) n = n->n; return n; }
	inline Iterator operator-(int j) const { Node *n = i; while (j--) n = n->p; return n; }
    };
    friend class Iterator;

    class ConstIterator
    {
    public:
	typedef std::bidirectional_iterator_tag  iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	Node *i;
	inline ConstIterator(Node *n = 0): i(n){}
	inline ConstIterator(const ConstIterator &o): i(o.i){}
	inline ConstIterator(Iterator ci): i(ci.i){}
	inline const T &operator*() const { return i->t; }
	inline bool operator==(const ConstIterator &o) const { return i == o.i; }
	inline bool operator!=(const ConstIterator &o) const { return i != o.i; }
	inline ConstIterator operator++() { i = i->n; return *this; }
	inline ConstIterator operator++(int) { Node *n = i; i = i->n; return n; }
	inline ConstIterator operator--() { i = i->p; return *this; }
	inline ConstIterator operator--(int) { Node *n = i; i = i->p; return n; }
	inline ConstIterator operator+(int j) const { Node *n = i; while (j--) n = n->n; return n; }
	inline ConstIterator operator-(int j) const { Node *n = i; while (j--) n = n->p; return n; }
    };
    friend class ConstIterator;

    // stl style
    inline Iterator begin() { detach(); return e->n; }
    inline ConstIterator begin() const { return e->n; }
    inline ConstIterator constBegin() const { return e->n; }
    inline Iterator end() { detach(); return e; }
    inline ConstIterator end() const { return e; }
    inline ConstIterator constEnd() const { return e; }
    Iterator insert(Iterator before, const T &t);
    Iterator erase(Iterator pos);
    Iterator erase(Iterator first, Iterator last);

    // more Qt
    inline int count() const { return d->size; }
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    T& last() { Q_ASSERT(!isEmpty()); return *(--end()); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(--end()); }
    inline void removeFirst() { Q_ASSERT(!isEmpty()); erase( begin() ); }
    inline void removeLast() { Q_ASSERT(!isEmpty()); erase( --end() ); }

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline void push_back(const T &t) { append(t); }
    inline void push_front(const T &t) { prepend(t); }
    inline T& front() { return first(); }
    inline const T& front() const { return first(); }
    inline T& back() { return last(); }
    inline const T& back() const { return last(); }
    inline void pop_front() { removeFirst(); }
    inline void pop_back() { removeLast(); }
    inline bool empty() const { return isEmpty(); }
    typedef int size_type;

#ifdef QT_COMPAT
    // compatibility
    inline QT_COMPAT Iterator remove(Iterator pos) { return erase(pos); }
    inline QT_COMPAT int findIndex( const T& t ) const
    { int i=0; for (ConstIterator it = begin(); it != end(); ++it, ++i) if(*it == t) return i; return -1;}
    inline QT_COMPAT Iterator find(Iterator from, const T& t)
    { while (from != end() && !(*from == t)) ++from; return from; }
    inline QT_COMPAT Iterator find(const T& t)
    { return find(begin(), t); }
    inline QT_COMPAT ConstIterator find(ConstIterator from, const T& t) const
    { while (from != end() && !(*from == t)) ++from; return from; }
    inline QT_COMPAT ConstIterator find(const T& t) const
    { return find(begin(), t); }
#endif

    // comfort
    QLinkedList &operator+=(const QLinkedList &l);
    QLinkedList operator+(const QLinkedList &l) const;
    inline void operator+=(const T &t) { append(t); }
    inline QLinkedList &operator<< (const T &t) { append(t); return *this; }

    inline bool ensure_constructed()
    { if (!d) { d = &QLinkedListData::shared_null; ++d->ref; return false; } return true; }

private:
    operator QNoImplicitBoolCast() const;
    void detach_helper();
    void free(QLinkedListData*);
};

template <typename T>
inline QLinkedList<T>::~QLinkedList()
{
    if (!--d->ref)
	free(d);
}

template <typename T>
void QLinkedList<T>::detach_helper() {
    union { QLinkedListData *d; Node *e; } x;
    x.d = new QLinkedListData;
    x.d->ref = 1;
    x.d->size = d->size;
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

template <typename T>
void QLinkedList<T>::free(QLinkedListData *x)
{
    Node *y = (Node*)x;
    Node *i = y->n;
    if ( x->ref == 0 ) {
	while(i != y) {
	    Node *n = i;
	    i = i->n;
	    delete n;
	}
	delete x;
    }
}

template <typename T>
void QLinkedList<T>::clear()
{
    *this = QLinkedList<T>();
}

template <typename T>
QLinkedList<T> &QLinkedList<T>::operator=(const QLinkedList<T> &l)
{
    if ( d != l.d ) {
	QLinkedListData *x = l.d;
	++x->ref;
	x = qAtomicSetPtr( &d, x );
	if (!--x->ref)
	    free(x);
    }
    return *this;
}

template <typename T>
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

template <typename T>
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

template <typename T>
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

template <typename T>
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
	    delete n;
	    c++;
	} else {
	    i = i->n;
	}
    }
    d->size-=c;
    return c;
}

template <typename T>
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

template <typename T>
bool QLinkedList<T>::contains(const T &t) const
{
    Node *i = e;
    while ((i = i->n) != e)
	if (i->t == t)
	    return true;
    return false;
}

template <typename T>
int QLinkedList<T>::count(const T &t) const
{
    Node *i = e;
    int c = 0;
    while ((i = i->n) != e)
	if (i->t == t)
	    c++;
    return c;
}


template <typename T>
typename QLinkedList<T>::Iterator QLinkedList<T>::insert(Iterator before, const T &t)
{
    Node *i = before.i;
    Node *m = new Node(t);
    m->n = i;
    m->p = i->p;
    m->p->n = m;
    i->p = m;
    d->size++;
    return m;
}

template <typename T>
typename QLinkedList<T>::Iterator QLinkedList<T>::erase( typename QLinkedList<T>::Iterator first,
							 typename QLinkedList<T>::Iterator last )
{
    while (first != last)
	erase(first++);
    return last;
}


template <typename T>
typename QLinkedList<T>::Iterator QLinkedList<T>::erase(Iterator pos)
{
    detach();
    Node *i = pos.i;
    if (i != e) {
	Node *n = i;
	i->n->p = i->p;
	i->p->n = i->n;
	i = i->n;
	delete n;
	d->size--;
    }
    return i;
}

template <typename T>
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

template <typename T>
QLinkedList<T> QLinkedList<T>::operator+(const QLinkedList<T> &l) const
{
    QLinkedList<T> n = *this;
    n += l;
    return n;
}

Q_DECLARE_ITERATOR(QLinkedList)

#endif // QLINKEDLIST_H

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
    inline void detach()
    { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline bool isEmpty() const { return d->size == 0; }

    void clear();

    void append(const T &);
    void prepend(const T &);
    T takeFirst();
    T takeLast();
    int removeAll(const T &t);
    bool contains(const T &t) const;
    int count(const T &t) const;

    class iterator
    {
    public:
        typedef std::bidirectional_iterator_tag  iterator_category;
        typedef ptrdiff_t  difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        Node *i;
        inline iterator() : i(0) {}
        inline iterator(Node *n) : i(n) {}
        inline iterator(const iterator &o): i(o.i){}
        inline T &operator*() const { return i->t; }
        inline T *operator->() const { return &i->t; }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }
        inline iterator operator++() { i = i->n; return *this; }
        inline iterator operator++(int) { Node *n = i; i = i->n; return n; }
        inline iterator operator--() { i = i->p; return *this; }
        inline iterator operator--(int) { Node *n = i; i = i->p; return n; }
        inline iterator operator+(int j) const
        { Node *n = i; if (j > 0) while (j--) n = n->n; else while (j++) n = n->p; return n; }
        inline iterator operator-(int j) const { return operator+(-j); }
        inline iterator &operator+=(int j) { return *this = *this + j; }
        inline iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class iterator;

    class const_iterator
    {
    public:
        typedef std::bidirectional_iterator_tag  iterator_category;
        typedef ptrdiff_t  difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        Node *i;
        inline const_iterator() : i(0) {}
        inline const_iterator(Node *n) : i(n) {}
        inline const_iterator(const const_iterator &o): i(o.i){}
        inline const_iterator(iterator ci): i(ci.i){}
        inline const T &operator*() const { return i->t; }
        inline const T *operator->() const { return &i->t; }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }
        inline const_iterator operator++() { i = i->n; return *this; }
        inline const_iterator operator++(int) { Node *n = i; i = i->n; return n; }
        inline const_iterator operator--() { i = i->p; return *this; }
        inline const_iterator operator--(int) { Node *n = i; i = i->p; return n; }
        inline const_iterator operator+(int j) const
        { Node *n = i; if (j > 0) while (j--) n = n->n; else while (j++) n = n->p; return n; }
        inline const_iterator operator-(int j) const { return operator+(-j); }
        inline const_iterator &operator+=(int j) { return *this = *this + j; }
        inline const_iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class const_iterator;

    // stl style
    inline iterator begin() { detach(); return e->n; }
    inline const_iterator begin() const { return e->n; }
    inline const_iterator constBegin() const { return e->n; }
    inline iterator end() { detach(); return e; }
    inline const_iterator end() const { return e; }
    inline const_iterator constEnd() const { return e; }
    iterator insert(iterator before, const T &t);
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    inline int count() const { return d->size; }
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    T& last() { Q_ASSERT(!isEmpty()); return *(--end()); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(--end()); }
    inline void removeFirst() { Q_ASSERT(!isEmpty()); erase(begin()); }
    inline void removeLast() { Q_ASSERT(!isEmpty()); erase(--end()); }

    // stl compatibility
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
    inline QT_COMPAT iterator remove(iterator pos) { return erase(pos); }
    inline QT_COMPAT int findIndex(const T& t) const
    { int i=0; for (const_iterator it = begin(); it != end(); ++it, ++i) if(*it == t) return i; return -1;}
    inline QT_COMPAT iterator find(iterator from, const T& t)
    { while (from != end() && !(*from == t)) ++from; return from; }
    inline QT_COMPAT iterator find(const T& t)
    { return find(begin(), t); }
    inline QT_COMPAT const_iterator find(const_iterator from, const T& t) const
    { while (from != end() && !(*from == t)) ++from; return from; }
    inline QT_COMPAT const_iterator find(const T& t) const
    { return find(begin(), t); }
#endif

    // comfort
    QLinkedList &operator+=(const QLinkedList &l);
    QLinkedList operator+(const QLinkedList &l) const;
    inline void operator+=(const T &t) { append(t); }
    inline QLinkedList &operator<< (const T &t) { append(t); return *this; }

private:
    void detach_helper();
    void free(QLinkedListData*);
};

template <typename T>
inline QLinkedList<T>::~QLinkedList()
{
    if (!d)
        return;
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
    if (x->ref == 0) {
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
    if (d != l.d) {
        QLinkedListData *x = l.d;
        ++x->ref;
        x = qAtomicSetPtr(&d, x);
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
int QLinkedList<T>::removeAll(const T &t)
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
inline T QLinkedList<T>::takeFirst()
{
    T t = first();
    removeFirst();
    return t;
}

template <typename T>
inline T QLinkedList<T>::takeLast()
{
    T t = last();
    removeLast();
    return t;
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
typename QLinkedList<T>::iterator QLinkedList<T>::insert(iterator before, const T &t)
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
typename QLinkedList<T>::iterator QLinkedList<T>::erase(typename QLinkedList<T>::iterator first,
                                                         typename QLinkedList<T>::iterator last)
{
    while (first != last)
        erase(first++);
    return last;
}


template <typename T>
typename QLinkedList<T>::iterator QLinkedList<T>::erase(iterator pos)
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

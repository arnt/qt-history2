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

#ifndef QLIST_H
#define QLIST_H

#ifndef QT_H
#include "qiterator.h"
#include "qatomic.h"
#include "qvector.h"
#endif // QT_H

struct Q_CORE_EXPORT QListData {
    struct DataHeader {
        QAtomic ref;
        int alloc, begin, end;
    };
    struct Data {
        QAtomic ref;
        int alloc, begin, end;
        void *array[1];
    };
    Data *detach();
    void realloc(int alloc);
    static Data shared_null;
    Data *d;
    void **erase(void **xi);
    void **append();
    void **append(const QListData &l);
    void **prepend();
    void **insert(int i);
    void remove(int i);
    void remove(int i, int n);
    void move(int from, int to);
    inline int size() const { return d->end - d->begin; }
    inline bool isEmpty() const { return d->end  == d->begin; }
    inline void **at(int i) const { return d->array + d->begin + i; }
    inline void **begin() const { return d->array + d->begin; }
    inline void **end() const { return d->array + d->end; }
};

template <typename T>
class QList
{
    struct Node { void *v;
#if defined(Q_CC_BOR)
        Q_INLINE_TEMPLATE T &t();
#else
        Q_INLINE_TEMPLATE T &t()
        { return QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic ? *(T*)v : *(T*)this; }
#endif
    };

    union { QListData p; QListData::Data *d; };

public:
    inline QList() : d(&QListData::shared_null) { ++d->ref; }
    inline QList(const QList &l) : d(l.d) { ++d->ref; }
    QList(const QVector<T> &vector);
    ~QList();
    QList &operator=(const QList &l);
    bool operator==(const QList &l) const;
    inline bool operator!=(const QList &l) const { return !(*this == l); }

    inline int size() const { return p.size(); }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline bool isEmpty() const { return p.isEmpty(); }

    void clear();

    const T &at(int i) const;
    const T &operator[](int i) const;
    T &operator[](int i);

    void append(const T &t);
    void prepend(const T &t);
    void insert(int i, const T &t);
    void replace(int i, const T &t);
    void removeAt(int i);
    int removeAll(const T &t);
    T takeAt(int i);
    T takeFirst();
    T takeLast();
    void move(int from, int to);
    void swap(int i, int j);
    int indexOf(const T &t, int from = 0) const;
    int lastIndexOf(const T &t, int from = -1) const;
    QBool contains(const T &t) const;
    int count(const T &t) const;

    class iterator {
    public:
        Node *i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef ptrdiff_t  difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline iterator() : i(0) {}
        inline iterator(Node *n) : i(n) {}
        inline iterator(const iterator &o): i(o.i){}
        inline T &operator*() const { return i->t(); }
        inline T *operator->() const { return &i->t(); }
        inline T &operator[](int j) const { return i[j].t(); }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }
        inline iterator operator++() { ++i; return *this; }
        inline iterator operator++(int) { Node *n = i; ++i; return n; }
        inline iterator operator--() { i--; return *this; }
        inline iterator operator--(int) { Node *n = i; i--; return n; }
        inline iterator &operator+=(int j) { i+=j; return *this; }
        inline iterator &operator-=(int j) { i-=j; return *this; }
        inline iterator operator+(int j) const { return iterator(i+j); }
        inline iterator operator-(int j) const { return iterator(i-j); }
        inline int operator-(iterator j) const { return i - j.i; }
    };

    class const_iterator {
    public:
        Node *i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline const_iterator() : i(0) {}
        inline const_iterator(Node *n) : i(n) {}
        inline const_iterator(const const_iterator &o): i(o.i) {}
        inline const_iterator(const iterator &o): i(o.i) {}
        inline const T &operator*() const { return i->t(); }
        inline const T *operator->() const { return &i->t(); }
        inline const T &operator[](int j) const { return i[j].t(); }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }
        inline const_iterator operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { Node *n = i; ++i; return n; }
        inline const_iterator operator--() { i--; return *this; }
        inline const_iterator operator--(int) { Node *n = i; i--; return n; }
        inline const_iterator &operator+=(int j) { i+=j; return *this; }
        inline const_iterator &operator-=(int j) { i+=j; return *this; }
        inline const_iterator operator+(int j) const { return const_iterator(i+j); }
        inline const_iterator operator-(int j) const { return const_iterator(i-j); }
        inline int operator-(const_iterator j) const { return i - j.i; }
    };

    // stl style
    inline iterator begin() { detach(); return (Node*) p.begin(); }
    inline const_iterator begin() const { return (Node*) p.begin(); }
    inline const_iterator constBegin() const { return (Node*) p.begin(); }
    inline iterator end() { detach(); return (Node*) p.end(); }
    inline const_iterator end() const { return (Node*) p.end(); }
    inline const_iterator constEnd() const { return (Node*) p.end(); }
    iterator insert(iterator before, const T &t);
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    inline int count() const { return p.size(); }
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    T& last() { Q_ASSERT(!isEmpty()); return *(--end()); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(--end()); }
    inline void removeFirst() { Q_ASSERT(!isEmpty()); erase(begin()); }
    inline void removeLast() { Q_ASSERT(!isEmpty()); erase(--end()); }
    QList<T> mid(int pos, int length = -1) const;

    T value(int i) const;
    T value(int i, const T &defaultValue) const;

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
    inline QT_COMPAT iterator remove(iterator pos) { return erase(pos); }
    inline QT_COMPAT int remove(const T &t) { return removeAll(t); }
    inline QT_COMPAT int findIndex(const T& t) const { return indexOf(t); }
    inline QT_COMPAT iterator find(const T& t)
    { int i = indexOf(t); return (i == -1 ? end() : (begin()+i)); }
    inline QT_COMPAT const_iterator find (const T& t) const
    { int i = indexOf(t); return (i == -1 ? end() : (begin()+i)); }
    inline QT_COMPAT iterator find(iterator from, const T& t)
    { int i = indexOf(t, from - begin()); return i == -1 ? end() : begin()+i; }
    inline QT_COMPAT const_iterator find(const_iterator from, const T& t) const
    { int i = indexOf(t, from - begin()); return i == -1 ? end() : begin()+i; }
#endif

    // comfort
    QList &operator+=(const QList &l);
    QList operator+(const QList &l) const
    { QList n = *this; n += l; return n; }
    inline void operator+=(const T &t)
    { append(t); }
    inline QList &operator<< (const T &t)
    { append(t); return *this; }

private:
    void detach_helper();
    void free(QListData::Data *d);

    void node_construct(Node *n, const T &t);
    void node_destruct(Node *n);
    void node_copy(Node *from, Node *to, Node *src);
    void node_destruct(Node *from, Node *to);
};

#if defined(Q_CC_BOR)
template <typename T>
Q_INLINE_TEMPLATE T &QList<T>::Node::t()
{ return QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic ? *(T*)v:*(T*)this; }
#endif

template <typename T>
Q_INLINE_TEMPLATE void QList<T>::node_construct(Node *n, const T &t)
{
    if (QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic) n->v = new T(t);
    else if (QTypeInfo<T>::isComplex) new (n) T(t);
    else *(T*)n = t;
}

template <typename T>
Q_INLINE_TEMPLATE void QList<T>::node_destruct(Node *n)
{
    if (QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic) delete (T*)n->v;
    else if (QTypeInfo<T>::isComplex) ((T*)n)->~T();
}

template <typename T>
Q_INLINE_TEMPLATE void QList<T>::node_copy(Node *from, Node *to, Node *src)
{
    if (QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic)
        while(from != to)
            (from++)->v = new T(*(T*)(src++)->v);
    else if (QTypeInfo<T>::isComplex)
        while(from != to)
            new (from++) T(*(T*)src++);
}

template <typename T>
Q_INLINE_TEMPLATE void QList<T>::node_destruct(Node *from, Node *to)
{
    if (QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic)
        while(from != to) --to, delete (T*) to->v;
    else if (QTypeInfo<T>::isComplex)
        while (from != to) --to, ((T*)to)->~T();
}

template <typename T>
Q_INLINE_TEMPLATE QList<T> &QList<T>::operator=(const QList<T> &l)
{
    if (d != l.d) {
        QListData::Data *x = l.d;
        ++x->ref;
        x = qAtomicSetPtr(&d, x);
        if (!--x->ref)
            free(x);
    }
    return *this;
}
template <typename T>
inline typename QList<T>::iterator QList<T>::insert(iterator before, const T &t)
{ Node *n = (Node*) p.insert(before.i-(Node*)p.begin());
 node_construct(n,t); return n; }
template <typename T>
inline typename QList<T>::iterator QList<T>::erase(iterator it)
{ node_destruct(it.i);
 return (Node*) p.erase((void**)it.i); }
template <typename T>
inline const T &QList<T>::at(int i) const
{ Q_ASSERT_X(i >= 0 && i < p.size(), "QList<T>::at", "index out of range");
 return ((Node*) p.at(i))->t(); }
template <typename T>
inline const T &QList<T>::operator[](int i) const
{ Q_ASSERT_X(i >= 0 && i < p.size(), "QList<T>::operator[]", "index out of range");
 return ((Node*) p.at(i))->t(); }
template <typename T>
inline T &QList<T>::operator[](int i)
{ Q_ASSERT_X(i >= 0 && i < p.size(), "QList<T>::operator[]", "index out of range");
  detach(); return ((Node*) p.at(i))->t(); }
template <typename T>
inline void QList<T>::removeAt(int i)
{ if(i >= 0 && i < p.size()) { detach();
 node_destruct((Node*) p.at(i)); p.remove(i); } }
template <typename T>
inline T QList<T>::takeAt(int i)
{ Q_ASSERT_X(i >= 0 && i < p.size(), "QList<T>::take", "index out of range");
 detach(); Node*n = (Node*)p.at(i); T t = n->t(); node_destruct(n);
 p.remove(i); return t; }
template <typename T>
inline T QList<T>::takeFirst()
{ T t = first(); removeFirst(); return t; }
template <typename T>
inline T QList<T>::takeLast()
{ T t = last(); removeLast(); return t; }
template <typename T>
inline void QList<T>::append(const T &t)
{ detach();node_construct((Node*)p.append(), t); }
template <typename T>
inline void QList<T>::prepend(const T &t)
{ detach(); node_construct(((Node*)p.prepend()), t); }
template <typename T>
inline void QList<T>::insert(int i, const T &t)
{ detach(); node_construct((Node*)p.insert(i), t); }
template <typename T>
inline void QList<T>::replace(int i, const T &t)
{ Q_ASSERT_X(i >= 0 && i < p.size(), "QList<T>::replace", "index out of range");
 detach(); ((Node*)p.at(i))->t() = t; }
template <typename T>
inline void QList<T>::swap(int i, int j)
{ Q_ASSERT_X(i >= 0 && i < p.size() && j >= 0 && j < p.size(),
               "QList<T>::swap", "index out of range");
 detach(); T t = ((Node*)p.at(i))->t();
 ((Node*)p.at(i))->t() = ((Node*)p.at(j))->t();
 ((Node*)p.at(j))->t() = t;
}

template <typename T>
inline void QList<T>::move(int from, int to)
{ Q_ASSERT_X(from >= 0 && from < p.size() && to >= 0 && to < p.size(),
               "QList<T>::move", "index out of range");
 detach(); p.move(from, to);
}

template<typename T>
Q_OUTOFLINE_TEMPLATE QList<T> QList<T>::mid(int pos, int length) const
{
    if (length < 0)
        length = size() - pos;
    if (pos == 0 && length == size())
        return *this;
    QList<T> copy;
    if (pos + length > size())
        length = size() - pos;
    for (int i=pos; pos<pos + length; ++i)
        copy += at(i);
    return copy;
}

template<typename T>
Q_OUTOFLINE_TEMPLATE T QList<T>::value(int i) const
{
    if(i < 0 || i >= p.size()) {
        T t;
        qInit(t);
        return t;
    }
    return ((Node*) p.at(i))->t();
}
template<typename T>
Q_OUTOFLINE_TEMPLATE T QList<T>::value(int i, const T& defaultValue) const
{
    return ((i < 0 || i >= p.size()) ? defaultValue : ((Node*) p.at(i))->t());
}

template <typename T>
Q_OUTOFLINE_TEMPLATE void QList<T>::detach_helper()
{
    Node *n = (Node*) p.begin();
    QListData::Data *x = p.detach();
    if (x)
        free(x);
    node_copy((Node*) p.begin(), (Node*) p.end(), n);
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QList<T>::~QList()
{
    if (!d)
        return;
    QListData::Data *x = &QListData::shared_null;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        free(x);
}

template <typename T>
Q_OUTOFLINE_TEMPLATE bool QList<T>::operator== (const QList<T> &l) const
{
    if (p.size() != l.p.size())
        return false;
    if (d == l.d)
        return true;
    Node *i = (Node*) p.end();
    Node *b = (Node*) p.begin();
    Node *li = (Node*) l.p.end();
    while (i != b) {
        --i; --li;
        if (! (i->t() == li->t()))
            return false;
    }
    return true;
}


template <typename T>
Q_OUTOFLINE_TEMPLATE void QList<T>::free(QListData::Data *data)
{
    node_destruct((Node*)(data->array + data->begin),
                  (Node*)(data->array + data->end));
    if (data->ref == 0)
        qFree(data);
}


template <typename T>
Q_OUTOFLINE_TEMPLATE void QList<T>::clear()
{
    *this = QList<T>();
}

template <typename T>
Q_OUTOFLINE_TEMPLATE int QList<T>::removeAll(const T &t)
{
    detach();
    int count=0, i=0;
    Node *n;
    while (i < p.size())
        if ((n = (Node*)p.at(i))->t() == t) {
            node_destruct(n);
            p.remove(i);
            ++count;
        } else {
            ++i;
        }
    return count;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE typename QList<T>::iterator QList<T>::erase(typename QList<T>::iterator first,
                                                                 typename QList<T>::iterator last)
{
    for (Node *n = first.i; n < last.i; ++n)
        node_destruct(n);
    int idx = first - begin();
    p.remove(idx, last - first);
    return begin() + idx;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QList<T> &QList<T>::operator+=(const QList<T> &l)
{
    detach();
    Node *n = (Node*)p.append(l.p);
    node_copy(n, (Node*) p.end(), (Node*) l.p.begin());
    return *this;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE int QList<T>::indexOf(const T &t, int from) const
{
    if (from < 0)
        from = qMax(from + p.size(), 0);
    if (from < p.size()) {
        Node *n = (Node*) p.at(from -1);
        Node *e = (Node*) p.end();
        while (++n != e)
            if (n->t() == t)
                return n - (Node*)p.begin();
    }
    return -1;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE int QList<T>::lastIndexOf(const T &t, int from) const
{
    if (from < 0)
        from += p.size();
    else if (from >= p.size())
        from = p.size()-1;
    if (from >= 0) {
        Node *b = (Node*) p.begin();
        Node *n = (Node*) p.at(from + 1);
        while (n-- != b) {
            if (n->t() == t)
                return n - b;
        }
    }
    return -1;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QBool QList<T>::contains(const T &t) const
{
    Node *b = (Node*) p.begin();
    Node *i = (Node*) p.end();
    while (i-- != b)
        if (i->t() == t)
            return QBool(true);
    return QBool(false);
}

template <typename T>
Q_OUTOFLINE_TEMPLATE int QList<T>::count(const T &t) const
{
    int c = 0;
    Node *b = (Node*) p.begin();
    Node *i = (Node*) p.end();
    while (i-- != b)
        if (i->t() == t)
            ++c;
    return c;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QList<T>::QList(const QVector<T> &vector)
    : d(&QListData::shared_null)
{
    ++d->ref;
    detach_helper();
    p.realloc(vector.size());
    for (int i = 0; i < vector.size(); ++i)
        append(vector.at(i));
}

Q_DECLARE_ITERATOR(QList)

#endif // QLIST_H

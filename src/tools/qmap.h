#ifndef QMAP_H
#define QMAP_H

#ifndef QT_H
#include "qatomic.h"
#include "qdatastream.h"
#include "qiterator.h"
#include "qlist.h"
#include "qpair.h"
#endif

#ifndef QT_NO_STL
#include <iterator>
#include <map>
#endif

#include <new>

struct QMapData
{
    struct Node
    {
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
	Node *backward;
#endif
	Node *forward[1];
    };
    enum { LastLevel = 11 };

#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    Node *backward;
#endif
    Node *forward[QMapData::LastLevel + 1];
    Node * volatile cachedNode;
    QAtomic ref;
    int topLevel;
    int size;
    uint randomBits;

    QMapData *createData();
    void freeData(int offset);
    Node *node_create(Node *update[], int offset);
    void node_delete(Node *update[], int offset, Node *node);

    static QMapData shared_null;
};

template <class Key, class T>
class QMap
{
    struct Node
    {
	Key key;
        T value;
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
        QMapData::Node *backward;
#endif
        QMapData::Node *forward[1];
    };
    struct NodeOffset
    {
	Key key;
        T value;
    };
    union {
	QMapData *d;
        QMapData::Node *e;
    };

    static inline int offset() { return sizeof(NodeOffset); }
    static inline Node *concrete(const QMapData::Node *node) {
	return (Node *)((char *)node - offset());
    }

public:
    inline QMap() : d(&QMapData::shared_null) { ++d->ref; }
    inline QMap(const QMap<Key, T> &other) : d(other.d) { ++d->ref; }
    inline ~QMap() { if (!--d->ref) freeData(d); }

    QMap &operator=(const QMap &other);
#ifndef QT_NO_STL
    QMap(const typename std::map<Key, T> &other);
    QMap<Key, T> &operator=(const typename std::map<Key,T> &other);
#endif

    class Iterator
    {
	QMapData::Node *i;
    public:
#ifndef QT_NO_STL
	typedef std::bidirectional_iterator_tag iterator_category;
#endif
	inline operator QMapData::Node *() const { return i; }
	inline Iterator() : i(0) { }
	inline Iterator(QMapData::Node *node) : i((QMapData::Node *)node) { }

	inline T &operator*() { return concrete(i)->value; }
	inline const T &operator*() const { return concrete(i)->value; }

	inline const Key &key() const { return concrete(i)->key; }
	inline const T &value() const { return concrete(i)->value; }
	inline T &value() { return concrete(i)->value; }
#ifndef QT_NO_COMPAT
	inline const T &data() const { return concrete(i)->value; }
	inline T &data() { return concrete(i)->value; }
#endif
	inline bool operator==(const Iterator &o) { return i == o.i; }
	inline bool operator!=(const Iterator &o) { return i != o.i; }

	inline Iterator &operator++() {
	    i = i->forward[0];
	    return *this;
	}
	inline Iterator operator++(int) {
	    Iterator r = *this;
	    i = i->forward[0];
	    return r;
	}
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
        inline Iterator &operator--() {
            i = i->backward;
            return *this;
        }
        inline Iterator operator--(int) {
	    Iterator r = *this;
            i = i->backward;
            return r;
        }
#endif
    };
    friend class Iterator;

    class ConstIterator
    {
	QMapData::Node *i;
    public:
	inline operator QMapData::Node *() const { return i; }
	inline ConstIterator() : i(0) { }
	inline ConstIterator(QMapData::Node *node) : i((QMapData::Node *)node) { }
	inline ConstIterator(const Iterator &o) { i = ((ConstIterator &)o).i; }

	inline const T &operator*() const { return concrete(i)->value; }

	inline const Key &key() const { return concrete(i)->key; }
	inline const T &value() const { return concrete(i)->value; }
#ifndef QT_NO_COMPAT
	inline const T &data() const { return concrete(i)->value; }
#endif
	inline bool operator==(const ConstIterator &o) { return i == o.i; }
	inline bool operator!=(const ConstIterator &o) { return i != o.i; }

	inline ConstIterator &operator++() {
	    i = i->forward[0];
	    return *this;
	}
	inline ConstIterator operator++(int) {
	    ConstIterator r = *this;
	    i = i->forward[0];
	    return r;
	}
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
        inline ConstIterator &operator--() {
            i = i->backward;
            return *this;
        }
        inline ConstIterator operator--(int) {
	    Iterator r = *this;
            i = i->backward;
            return r;
        }
#endif
    };
    friend class ConstIterator;

    typedef Key key_type;
    typedef T mapped_type;
    typedef QPair<const Key, T> value_type;
    typedef QPair<Iterator, bool> insert_pair;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef T ValueType;
#ifndef QT_NO_STL
    typedef ptrdiff_t difference_type;
#else
    typedef int difference_type;
#endif
    typedef size_t size_type;
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline Iterator begin() { detach(); return Iterator(e->forward[0]); }
    inline Iterator end() { detach(); return Iterator(e); }
    inline ConstIterator begin() const { return ConstIterator(e->forward[0]); }
    inline ConstIterator end() const { return ConstIterator(e); }
    inline ConstIterator constBegin() const { return ConstIterator(e->forward[0]); }
    inline ConstIterator constEnd() const { return ConstIterator(e); }

    inline int size() const { return d->size; }
#ifndef QT_NO_COMPAT
    inline int count() const { return d->size; }
#endif
    inline bool empty() const { return !d->size; }
    inline bool isEmpty() const { return !d->size; }
    size_type count(const key_type &key) const;

    void clear();

    QPair<Iterator, bool> insert(const value_type &x);
    Iterator insert(const Key &key, const T &value);
#ifndef QT_NO_COMPAT
    Iterator insert(const Key &key, const T &value, bool overwrite);
#endif

    size_type erase(const Key &key);
    Iterator erase(Iterator it);

#ifndef QT_NO_COMPAT
    inline size_type remove(const Key &key) { return erase(key); }
    inline Iterator remove(Iterator it) { return erase(it); }
#endif

#ifndef QT_NO_COMPAT
    inline Iterator replace(const Key &key, const T &value) { return insert(key, value); }
#endif

    bool contains(const Key &key) const;
    Iterator find(const Key &key);
    ConstIterator find(const Key &key) const;
    const T value(const Key &key) const;
    const T value(const Key &key, const T &defaultValue) const;
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;

    QList<Key> keys() const;
    QList<T> values() const;

    inline bool ensure_constructed()
    { if (!d) { d = &QMapData::shared_null; ++d->ref; return false; } return true; }

private:
    void detach_helper();
    void freeData(QMapData* d);
    QMapData::Node *findNode(const Key &key) const;
    QMapData::Node *mutableFindNode(QMapData::Node *update[], const Key &key);
    QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key,
				const T &value);
};

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::freeData(QMapData* d)
{
    QMapData::Node *cur = e->forward[0];
    while (cur != e) {
	concrete(cur)->key.~Key();
	concrete(cur)->value.~T();
	cur = cur->forward[0];
    }
    d->freeData(offset());
}

template <class Key, class T>
Q_INLINE_TEMPLATE QMap<Key, T> &QMap<Key, T>::operator=(const QMap<Key, T> &other)
{
    if (d != other.d) {
	QMapData *x = other.d;
	++x->ref;
	x = qAtomicSetPtr(&d, x);
	if (!--x->ref)
	    freeData(x);
    }
    return *this;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QMap<Key, T>::clear()
{
    *this = QMap<Key, T>();
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMapData::Node *
QMap<Key, T>::node_create(QMapData *d, QMapData::Node *update[], const Key &key, const T &value)
{
    QMapData::Node *abstractNode = d->node_create(update, offset());
    Node *concreteNode = concrete(abstractNode);
    new (&concreteNode->key) Key(key);
    new (&concreteNode->value) T(value);
    return abstractNode;
}

template <class Key, class T>
Q_INLINE_TEMPLATE QMapData::Node *QMap<Key, T>::findNode(const Key &key) const
{
    QMapData::Node *cur = e;
    QMapData::Node *next = e;

#if 0
    QMapData::Node *node = d->cachedNode;
    if (node != e && concrete(node)->key == key)
	return node;
#endif

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
    }

    if (next != e && concrete(next)->key == key) {
	d->cachedNode = next;
	return next;
    } else {
	return e;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QMap<Key, T>::value(const Key &key) const
{
    QMapData::Node *node = findNode(key);
    if (node == e) {
	T t;
	qInit(t);
	return t;
    } else {
	return concrete(node)->value;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QMap<Key, T>::value(const Key &key, const T &defaultValue) const
{
    QMapData::Node *node = findNode(key);
    if (node == e) {
	return defaultValue;
    } else {
	return concrete(node)->value;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QMap<Key, T>::operator[](const Key &key) const
{
    return value(key);
}

template <class Key, class T>
Q_INLINE_TEMPLATE T &QMap<Key, T>::operator[](const Key &key)
{
    detach();

    QMapData::Node *node = findNode(key);
    if (node == e) {
	QMapData::Node *update[QMapData::LastLevel + 1];
	mutableFindNode(update, key);
	return concrete(node_create(d, update, key, T()))->value;
    } else {
	return concrete(node)->value;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::size_type QMap<Key, T>::count(const key_type &key) const
{
    return findNode(key) != e ? 1 : 0;
}

template <class Key, class T>
Q_INLINE_TEMPLATE bool QMap<Key, T>::contains(const key_type &key) const
{
    return findNode(key) != e;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::insert(const Key &key,
								       const T &value)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *node = mutableFindNode(update, key);
    if (node == e) {
	node = node_create(d, update, key, value);
    } else {
	concrete(node)->value = value;
    }
    return Iterator(node);
}

#ifndef QT_NO_COMPAT
template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::insert(const Key &key,
								       const T &value,
                                                                       bool overwrite)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *node = mutableFindNode(update, key);
    if (node == e) {
	node = node_create(d, update, key, value);
    } else {
	if (overwrite)
	    concrete(node)->value = value;
    }
    return Iterator(node);
}
#endif

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QMap<Key, T>::size_type QMap<Key, T>::erase(const Key &key)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *cur = e;
    QMapData::Node *next = e;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
	update[i] = cur;
    }

    bool found = (next != e && concrete(next)->key == key);
    if (found) {
	concrete(next)->key.~Key();
	concrete(next)->value.~T();
	d->node_delete(update, offset(), next);
    }
    return found;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::erase(Iterator it)
{
    Iterator n = it;
    ++n;
    erase(it.key());
    return n;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::ConstIterator QMap<Key, T>::find(const Key &key) const
{
    return ConstIterator(findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::find(const Key &key)
{
    detach();
    return Iterator(findNode(key));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::detach_helper()
{
    union { QMapData *d; QMapData::Node *e; } x;
    x.d = d->createData();
    if (d->size) {
	QMapData::Node *update[QMapData::LastLevel + 1];
	QMapData::Node *cur = e->forward[0];
	update[0] = x.e;
	while (cur != e) {
	    node_create(x.d, update, concrete(cur)->key, concrete(cur)->value);
	    cur = cur->forward[0];
	}
    }
    x.d = qAtomicSetPtr(&d, x.d);
    if (!--x.d->ref)
 	freeData(x.d);
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QMapData::Node *
QMap<Key, T>::mutableFindNode(QMapData::Node *update[], const Key &key)
{
#if 0
    QMapData::Node *node = d->cachedNode;
    if (node != e && concrete(node)->key == key)
	return node;
#endif

    QMapData::Node *cur = e;
    QMapData::Node *next = e;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
	update[i] = cur;
    }
    if (next != e && !(key < concrete(next)->key)) {
	return next;
    } else {
	++d->size;
	return e;
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key,T>::keys() const
{
    QList<Key> res;
    ConstIterator i = begin();
    while (i != end()) {
	res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key,T>::values() const
{
    QList<Key> res;
    ConstIterator i = begin();
    while (i != end()) {
	res.append(i.value());
        ++i;
    }
    return res;
}

#ifndef QT_NO_STL
template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key, T>::QMap(const std::map<Key, T> &other)
    : d(&QMapData::shared_null)
{
    typename std::map<Key,T>::const_iterator it = other.end();
    while (it != other.begin()) {
	--it;
	insert((*it).first, (*it).second);
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key,T> &QMap<Key,T>::operator=(const std::map<Key,T> &other)
{
    clear();
    typename std::map<Key,T>::const_iterator it = other.end();
    while (it != other.begin()) {
	--it;
	insert((*it).first, (*it).second);
    }
    return *this;
}
#endif

#ifndef QT_NO_DATASTREAM
template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMap<Key, T> &map)
{
    map.clear();
    Q_UINT32 n;
    in >> n;
    for (Q_UINT32 i = 0; i < n; ++i) {
	Key key;
        T value;
	in >> key >> value;
	map.insert(key, value);
	if (in.atEnd())
	    break;
    }
    return in;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QMap<Key, T> &map)
{
    out << (Q_UINT32)map.size();
    typename QMap<Key, T>::ConstIterator it = map.begin();
    while (it != map.end()) {
	out << it.key() << it.value();
        ++it;
    }
    return out;
}
#endif

Q_DECLARE_ASSOCIATIVE_ITERATOR(QMap)

#endif // QMAP_H

#ifndef QMAP_H
#define QMAP_H

#ifndef QT_H
#include "qatomic.h"
#include "qiterator.h"
#include "qlist.h"
//#include "qpair.h" ###
#endif

#ifndef QT_NO_STL
#include <iterator>
#include <map>
#endif

#include <new>

struct Q_CORE_EXPORT QMapData
{
    struct Node {
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
    struct Node {
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
	typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

	inline operator QMapData::Node *() const { return i; }
	inline Iterator() : i(0) { }
	inline Iterator(QMapData::Node *node) : i((QMapData::Node *)node) { }

	inline const Key &key() const { return concrete(i)->key; }
	inline T &value() const { return concrete(i)->value; }
#ifdef QT_COMPAT
	inline QT_COMPAT T &data() const { return concrete(i)->value; }
#endif
	inline T &operator*() const { return concrete(i)->value; }
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
	typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

	inline operator QMapData::Node *() const { return i; }
	inline ConstIterator() : i(0) { }
	inline ConstIterator(QMapData::Node *node) : i((QMapData::Node *)node) { }
	inline ConstIterator(const Iterator &o) { i = ((ConstIterator &)o).i; }

	inline const Key &key() const { return concrete(i)->key; }
	inline const T &value() const { return concrete(i)->value; }
#ifdef QT_COMPAT
	inline QT_COMPAT const T &data() const { return concrete(i)->value; }
#endif
	inline const T &operator*() const { return concrete(i)->value; }
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

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline int size() const { return d->size; }
#ifdef QT_COMPAT
    inline QT_COMPAT int count() const { return d->size; }
#endif
    inline bool isEmpty() const { return !d->size; }
    int count(const Key &key) const;

    void clear();

//    QPair<Iterator, bool> insert(const value_type &x);
    Iterator insert(const Key &key, const T &value);
#ifdef QT_COMPAT
    QT_COMPAT Iterator insert(const Key &key, const T &value, bool overwrite);
#endif
    Iterator insertMulti(const Key &key, const T &value);

    int erase(const Key &key);
    Iterator erase(Iterator it);
#ifdef QT_COMPAT
    inline QT_COMPAT int remove(const Key &key) { return erase(key); }
    inline QT_COMPAT Iterator remove(Iterator it) { return erase(it); }
#endif

#ifdef QT_COMPAT
    inline QT_COMPAT Iterator replace(const Key &key, const T &value) { return insert(key, value); }
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
    QList<T> values(const Key &key) const;

    inline Iterator begin() { detach(); return Iterator(e->forward[0]); }
    inline ConstIterator begin() const { return ConstIterator(e->forward[0]); }
    inline ConstIterator constBegin() const { return ConstIterator(e->forward[0]); }
    inline Iterator end() {
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
	detach();
#endif
        return Iterator(e);
    }
    inline ConstIterator end() const { return ConstIterator(e); }
    inline ConstIterator constEnd() const { return ConstIterator(e); }

    inline bool ensure_constructed()
    { if (!d) { d = &QMapData::shared_null; ++d->ref; return false; } return true; }

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline bool empty() const { return isEmpty(); }

private:
    void detach_helper();
    void freeData(QMapData *d);
    QMapData::Node *findNode(const Key &key) const;
    QMapData::Node *mutableFindNode(QMapData::Node *update[], const Key &key);
    QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key,
				const T &value);
};

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

    QMapData::Node *node = d->cachedNode;
    if (node != e && concrete(node)->key == key)
	return node;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
    }

    if (next != e && concrete(next)->key == key) {
	(void) qAtomicSetPtr(&d->cachedNode, next);
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
Q_INLINE_TEMPLATE int QMap<Key, T>::count(const Key &key) const
{
    int cnt = 0;
    QMapData::Node *node = findNode(key);
    if (node != e) {
        do {
	    ++cnt;
            node = node->forward[0];
	} while (node != e && concrete(node)->key == key);
    }
    return cnt;
}

template <class Key, class T>
Q_INLINE_TEMPLATE bool QMap<Key, T>::contains(const Key &key) const
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

#ifdef QT_COMPAT
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
Q_INLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::insertMulti(const Key &key,
									    const T &value)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    mutableFindNode(update, key);
    return Iterator(node_create(d, update, key, value));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::freeData(QMapData *d)
{
    QMapData::Node *e = (QMapData::Node *)d;
    QMapData::Node *cur = e->forward[0];
    while (cur != e) {
	concrete(cur)->key.~Key();
	concrete(cur)->value.~T();
	cur = cur->forward[0];
    }
    d->freeData(offset());
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QMap<Key, T>::erase(const Key &key)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *cur = e;
    QMapData::Node *next = e;
    int oldSize = d->size;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
	update[i] = cur;
    }

    if (next != e && concrete(next)->key == key) {
	do {
	    cur = next;
            next = cur->forward[0];
	    concrete(cur)->key.~Key();
	    concrete(cur)->value.~T();
	    d->node_delete(update, offset(), cur);
	} while (next != e && concrete(next)->key == key);
    }
    return oldSize - d->size;
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
    QMapData::Node *node = d->cachedNode;
    if (node != e && concrete(node)->key == key)
	return node;

    QMapData::Node *cur = e;
    QMapData::Node *next = e;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
	update[i] = cur;
    }
    if (next != e && key == concrete(next)->key) {
	return next;
    } else {
	return e;
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, T>::keys() const
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
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T>::values() const
{
    QList<Key> res;
    ConstIterator i = begin();
    while (i != end()) {
	res.append(i.value());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T>::values(const Key &key) const
{
    QList<T> list;
    QMapData::Node *node = findNode(key);
    if (node != e) {
        do {
	    list.append(concrete(node)->value);
	    node = node->forward[0];
        } while (node != e && concrete(node)->key == key);
    }
    return list;
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

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

// #define QT_NO_QMAP_BACKWARD_ITERATORS

struct Q_CORE_EXPORT QMapData
{
    struct Node {
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
	Node *backward;
#endif
	Node *forward[1];
    };
    enum { LastLevel = 11, Sparseness = 3 };

#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    Node *backward;
#endif
    Node *forward[QMapData::LastLevel + 1];
    QAtomic ref;
    int topLevel;
    int size;
    uint randomBits;
    bool insertInOrder;

    static QMapData *createData();
    void continueFreeData(int offset);
    Node *node_create(Node *update[], int offset);
    void node_delete(Node *update[], int offset, Node *node);
#ifndef QT_NO_DEBUG
    uint adjust_ptr(Node *node);
    void dump();
#endif

    static QMapData shared_null;
};

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION)
class QDataStream;
template <class Key, class T> class QMap;
template <class Key, class T>
QDataStream &operator>>(QDataStream &out, QMap<Key, T> &map);
#endif

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
    struct Payload
    {
	Key key;
        T value;
    };
    union {
	QMapData *d;
        QMapData::Node *e;
    };

    static inline Node *concrete(QMapData::Node *node) {
	return reinterpret_cast<Node *>(reinterpret_cast<char *>(node) - sizeof(Payload));
    }
    struct BoolStruct { inline void QTrue() {} };
    typedef void (BoolStruct::*QSafeBool)();

public:
    inline QMap() : d(&QMapData::shared_null) { ++d->ref; }
    inline QMap(const QMap<Key, T> &other) : d(other.d) { ++d->ref; }
    inline ~QMap() { if (!--d->ref) freeData(d); }

    QMap<Key, T> &operator=(const QMap<Key, T> &other);
#ifndef QT_NO_STL
    QMap(const typename std::map<Key, T> &other);
    QMap<Key, T> &operator=(const typename std::map<Key,T> &other);
#endif

    bool operator==(const QMap<Key, T> &other) const;
    inline bool operator!=(const QMap<Key, T> &other) const { return !(*this == other); }

    inline int size() const { return d->size; }

    inline bool isEmpty() const { return d->size == 0; }
    inline bool operator!() const { return d->size == 0; }
    inline operator QSafeBool() const { return d->size == 0 ? 0 : &BoolStruct::QTrue; }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    void clear();

    int remove(const Key &key);
    T take(const Key &key);

    bool contains(const Key &key) const;
    const T value(const Key &key) const;
    const T value(const Key &key, const T &defaultValue) const;
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;

    QList<Key> keys() const;
    QList<T> values() const;
    QList<T> values(const Key &key) const;
    int count(const Key &key) const;

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
	inline Iterator(QMapData::Node *node) : i(node) { }

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
	inline ConstIterator(QMapData::Node *node) : i(node) { }
	inline ConstIterator(const Iterator &o)
        { i = reinterpret_cast<const ConstIterator &>(o).i; }

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

    // STL style
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
    Iterator erase(Iterator it);
#ifdef QT_COMPAT
    inline QT_COMPAT Iterator remove(Iterator it) { return erase(it); }
#endif
    // ### QPair<Iterator, bool> insert(const value_type &x);

    // more Qt
    inline int count() const { return d->size; }
    Iterator find(const Key &key);
    ConstIterator find(const Key &key) const;
    Iterator insert(const Key &key, const T &value);
#ifdef QT_COMPAT
    QT_COMPAT Iterator insert(const Key &key, const T &value, bool overwrite);
#endif
    Iterator insertMulti(const Key &key, const T &value);
#ifdef QT_COMPAT
    inline QT_COMPAT Iterator replace(const Key &key, const T &value) { return insert(key, value); }
#endif
    QMap<Key, T> &operator+=(const QMap<Key, T> &other);
    inline QMap<Key, T> operator+(const QMap<Key, T> &other) const
    { QMap<Key, T> result = *this; result += other; return result; }

    // STL compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline bool empty() const { return isEmpty(); }

    inline bool ensure_constructed()
    { if (!d) { d = &QMapData::shared_null; ++d->ref; return false; } return true; }

private:
    void detach_helper();
    void freeData(QMapData *d);
    QMapData::Node *findNode(const Key &key) const;
    QMapData::Node *mutableFindNode(QMapData::Node *update[], const Key &key);
    QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key,
				const T &value);

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION)
    friend QDataStream &operator>> <>(QDataStream &out, QMap<Key, T> &map);
#endif
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
    QMapData::Node *abstractNode = d->node_create(update, sizeof(Payload));
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

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < key)
	    cur = next;
    }

    if (next != e && !(key < concrete(next)->key)) {
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

    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *node = mutableFindNode(update, key);
    if (node == e)
	node = node_create(d, update, key, T());
    return concrete(node)->value;
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
	} while (node != e && !(key < concrete(node)->key));
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
Q_INLINE_TEMPLATE QMap<Key, T> &QMap<Key, T>::operator+=(const QMap<Key, T> &other)
{
    typename QMap<Key, T>::ConstIterator it = other.begin();
    while (it != other.end()) {
	insert(it.key(), it.value());
	++it;
    }
    return *this;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::freeData(QMapData *d)
{
    if (QTypeInfo<Key>::isComplex || QTypeInfo<T>::isComplex) {
        QMapData::Node *e = reinterpret_cast<QMapData::Node *>(d);
        QMapData::Node *cur = e;
        QMapData::Node *next = cur->forward[0];
        while (next != e) {
	    cur = next;
            next = cur->forward[0];
	    Node *concreteNode = concrete(cur);
	    concreteNode->key.~Key();
	    concreteNode->value.~T();
        }
    }
    d->continueFreeData(sizeof(Payload));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QMap<Key, T>::remove(const Key &key)
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

    if (next != e && !(key < concrete(next)->key)) {
	bool deleteNext = true;
	do {
	    cur = next;
	    next = cur->forward[0];
	    deleteNext = (next != e && !(concrete(cur)->key < concrete(next)->key));
	    concrete(cur)->key.~Key();
	    concrete(cur)->value.~T();
	    d->node_delete(update, sizeof(Payload), cur);
	} while (deleteNext);
    }
    return oldSize - d->size;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE T QMap<Key, T>::take(const Key &key)
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

    T t;
    if (next != e && !(key < concrete(next)->key)) {
	t = concrete(next)->key;
	concrete(next)->key.~Key();
	concrete(next)->value.~T();
	d->node_delete(update, sizeof(Payload), next);
    } else {
	qInit(t);
    }
    return t;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QMap<Key, T>::Iterator QMap<Key, T>::erase(Iterator it)
{
    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *cur = e;
    QMapData::Node *next = e;

    if (it == Iterator(e))
	return it;

    for (int i = d->topLevel; i >= 0; i--) {
	while ((next = cur->forward[i]) != e && concrete(next)->key < it.key())
	    cur = next;
	update[i] = cur;
    }

    while (next != e) {
	cur = next;
	next = cur->forward[0];
	if (cur == it) {
	    concrete(cur)->key.~Key();
	    concrete(cur)->value.~T();
	    d->node_delete(update, sizeof(Payload), cur);
            return Iterator(next);
	}

        for (int i = 0; i <= d->topLevel; ++i) {
	    if (update[i]->forward[i] != cur)
		break;
	    update[i] = cur;
        }
    }
    return end();
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::detach_helper()
{
    union { QMapData *d; QMapData::Node *e; } x;
    x.d = QMapData::createData();
    if (d->size) {
	x.d->insertInOrder = true;
	QMapData::Node *update[QMapData::LastLevel + 1];
	QMapData::Node *cur = e->forward[0];
	update[0] = x.e;
	while (cur != e) {
	    Node *concreteNode = concrete(cur);
	    node_create(x.d, update, concreteNode->key, concreteNode->value);
	    cur = cur->forward[0];
	}
	x.d->insertInOrder = false;
    }
    x.d = qAtomicSetPtr(&d, x.d);
    if (!--x.d->ref)
 	freeData(x.d);
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QMapData::Node *QMap<Key, T>::mutableFindNode(QMapData::Node *update[],
								   const Key &key)
{
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
    QList<T> res;
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
        } while (node != e && !(key < concrete(node)->key));
    }
    return list;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE bool QMap<Key, T>::operator==(const QMap<Key, T> &other) const
{
    if (size() != other.size())
	return false;
    if (d == other.d)
	return true;

    Iterator it1 = begin();
    Iterator it2 = other.begin();

    while (it1 != end()) {
	if (it1.key() != it2.key() || it1.value() != it2.value())
	    return false;
	++it2;
	++it1;
    }
    return true;
}

#ifndef QT_NO_STL
template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key, T>::QMap(const std::map<Key, T> &other)
{
    d = QMapData::createData();
    d->insertInOrder = true;
    typename std::map<Key,T>::const_iterator it = other.end();
    while (it != other.begin()) {
	--it;
	insert((*it).first, (*it).second);
    }
    d->insertInOrder = false;
}

template <class Key, class T>
Q_INLINE_TEMPLATE QMap<Key, T> &QMap<Key, T>::operator=(const typename std::map<Key,T> &other)
{
    *this = QMap(other);
    return *this;
}
#endif

Q_DECLARE_ASSOCIATIVE_ITERATOR(QMap)

#endif // QMAP_H

#ifndef QHASH_H
#define QHASH_H

#ifndef QT_H
#include "qatomic.h"
#include "qiterator.h"
#include "qlist.h"
//#include "qpair.h" ###
#endif // QT_H

#ifndef QT_NO_STL
#include <iterator>
#endif

class QByteArray;
class QString;

inline uint qHash(char key) { return (uint)key; }
inline uint qHash(signed char key) { return (uint)key; }
inline uint qHash(unsigned char key) { return (uint)key; }
inline uint qHash(signed short key) { return (uint)key; }
inline uint qHash(unsigned short key) { return (uint)key; }
inline uint qHash(signed int key) { return (uint)key; }
inline uint qHash(unsigned int key) { return (uint)key; }
inline uint qHash(signed long key) { return (uint)key; }
inline uint qHash(unsigned long key) { return (uint)key; }

Q_CORE_EXPORT uint qHash(const QByteArray &key);
Q_CORE_EXPORT uint qHash(const QString &key);

template <class T> inline uint qHash(const T *key)
{
    if (sizeof(const T *) > sizeof(uint))
	return (uint)((reinterpret_cast<Q_ULLONG>(key) >> 32) ^ reinterpret_cast<Q_ULLONG>(key));
    else
	return reinterpret_cast<uint>(key);
}

struct Q_CORE_EXPORT QHashData
{
    struct Node {
	Node *next;
	uint h;
    };

    Node *fakeNext;
    Node **buckets;
    QAtomic ref;
    int size;
    short userNumBits;
    short numBits;
    int numBuckets;
    void *autoDelete;

    QHashData *detach_helper(Node *(*node_duplicate)(Node *));
    void mightGrow();
    void hasShrunk();
    void rehash(int hint);
    Node *firstNode();
    static Node *nextNode(Node *node);
#ifndef QT_NO_QHASH_BACKWARD_ITERATORS
    static Node *prevNode(Node *node);
#endif

    static QHashData shared_null;
};

inline void QHashData::mightGrow()
{
    if (size >= numBuckets)
	rehash(numBits + 1);
}

inline void QHashData::hasShrunk()
{
    if (size <= (numBuckets >> 3) && numBits > userNumBits)
	rehash(qMax(numBits - 2, userNumBits));
}

inline QHashData::Node *QHashData::firstNode()
{
    Node *e = reinterpret_cast<Node *>(this);
    Node **bucket = buckets;
    int n = numBuckets;
    while (n--) {
	if (*bucket != e)
	    return *bucket;
	++bucket;
    }
    return e;
}

template <class Key, class T>
struct QHashNode
{
    QHashNode *next;
    uint h;
    Key key;
    T value;

    inline QHashNode(const Key &key0, const T &value0)
	: key(key0), value(value0) { }
    inline bool same_key(uint h0, const Key &key0) { return h0 == h && key0 == key; }
};

#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
#define Q_HASH_DECLARE_INT_NODE(key_type) \
    template <class T> \
    struct QHashNode<key_type, T> { \
	QHashNode *next; \
	union { uint h; key_type key; }; \
	T value; \
	inline QHashNode(key_type /* key0 */, const T &value0) : value(value0) { } \
	inline bool same_key(uint h0, key_type) { return h0 == h; } \
    };

#if defined(Q_BYTE_ORDER) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN
Q_HASH_DECLARE_INT_NODE(short)
Q_HASH_DECLARE_INT_NODE(ushort)
#endif
Q_HASH_DECLARE_INT_NODE(int)
Q_HASH_DECLARE_INT_NODE(uint)
#undef Q_HASH_DECLARE_INT_NODE
#endif // QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION

template <class Key, class T>
class QHash
{
    typedef QHashNode<Key, T> Node;

    union {
	QHashData *d;
	QHashNode<Key, T> *e;
    };

    static inline Node *concrete(QHashData::Node *node) {
	return reinterpret_cast<Node *>(node);
    }

public:
    inline QHash() : d(&QHashData::shared_null) { ++d->ref; }
    inline QHash(const QHash<Key, T> &other) : d(other.d) { ++d->ref; }
    inline ~QHash() { if (!--d->ref) freeData(d); }

    QHash<Key, T> &operator=(const QHash<Key, T> &other);

    bool operator==(const QHash<Key, T> &other) const;
    inline bool operator!=(const QHash<Key, T> &other) const { return !(*this == other); }

    inline int size() const { return d->size; }
    inline bool isEmpty() const { return d->size == 0; }
    inline bool operator!() const { return d->size == 0; }

    void reserve(int size);
    inline int capacity() const { return d->numBuckets; }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    void clear();

    int remove(const Key &key);
    T take(const Key &key); // ### on its way out

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
	QHashData::Node *i;
    public:
	typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

	inline operator Node *() const { return reinterpret_cast<Node *>(i); }
        inline Iterator() : i(0) { }
	explicit inline Iterator(void *node) : i(reinterpret_cast<QHashData::Node *>(node)) { }

	inline const Key &key() const { return concrete(i)->key; }
	inline T &value() const { return concrete(i)->value; }
#ifdef QT_COMPAT
	inline QT_COMPAT T &data() const { return concrete(i)->value; }
#endif
	inline T &operator*() const { return concrete(i)->value; }
	inline bool operator==(const Iterator &o) { return i == o.i; }
	inline bool operator!=(const Iterator &o) { return i != o.i; }

	inline Iterator &operator++() {
	    i = QHashData::nextNode(i);
	    return *this;
	}
	inline Iterator operator++(int) {
	    Iterator r = *this;
	    i = QHashData::nextNode(i);
	    return r;
	}
#ifndef QT_NO_QHASH_BACKWARD_ITERATORS
	inline Iterator &operator--() {
	    i = QHashData::prevNode(i);
	    return *this;
	}
	inline Iterator operator--(int) {
	    Iterator r = *this;
	    i = QHashData::prevNode(i);
	    return r;
	}
#endif
    };
    friend class Iterator;

    class ConstIterator
    {
	QHashData::Node *i;
    public:
	typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

	inline operator Node *() const { return reinterpret_cast<Node*>(i); }
        inline ConstIterator() : i(0) { }
	explicit inline ConstIterator(void *node)
	    : i(reinterpret_cast<QHashData::Node *>(node)) { }
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

	inline ConstIterator operator++() {
	    i = QHashData::nextNode(i);
	    return *this;
	}
	inline ConstIterator operator++(int) {
	    ConstIterator r = *this;
	    i = QHashData::nextNode(i);
	    return r;
	}
#ifndef QT_NO_QHASH_BACKWARD_ITERATORS
	inline ConstIterator &operator--() {
	    i = QHashData::prevNode(i);
	    return *this;
	}
	inline ConstIterator operator--(int) {
	    Iterator r = *this;
	    i = QHashData::prevNode(i);
	    return r;
	}
#endif
    };
    friend class ConstIterator;

    // STL style
    inline Iterator begin() { detach(); return Iterator(d->firstNode()); }
    inline ConstIterator begin() const { return ConstIterator(d->firstNode()); }
    inline ConstIterator constBegin() const { return ConstIterator(d->firstNode()); }
    inline Iterator end() {
#ifndef QT_NO_QHASH_BACKWARD_ITERATORS
	detach();
#endif
        return Iterator(e);
    }
    inline ConstIterator end() const { return ConstIterator(e); }
    inline ConstIterator constEnd() const { return ConstIterator(e); }
    Iterator erase(Iterator it);
    // ### QPair<Iterator, bool> insert(const value_type &x);

    // more Qt
    inline int count() const { return d->size; }
    Iterator find(const Key &key);
    ConstIterator find(const Key &key) const;
    Iterator insert(const Key &key, const T &value);
    Iterator insertMulti(const Key &key, const T &value);
#ifdef QT_COMPAT
    inline QT_COMPAT Iterator replace(const Key &key, const T &value) { return insert(key, value); }
#endif
    QHash<Key, T> &operator+=(const QHash<Key, T> &other);
    inline QHash<Key, T> operator+(const QHash<Key, T> &other) const
    { QHash<Key, T> result = *this; result += other; return result; }

    // should vanish soon
    bool autoDelete() const { return d->autoDelete == static_cast<void *>(this); }
    void setAutoDelete(bool enable);

    // STL compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline bool empty() const { return isEmpty(); }

    inline bool ensure_constructed()
    { if (!d) { d = &QHashData::shared_null; ++d->ref; return false; } return true; }

private:
    void detach_helper();
    void freeData(QHashData* d);
    Node **findNode(const Key &key, uint *hp = 0) const;
    Node *createNode(uint h, const Key &key, const T &value, Node **nextNode);
    static QHashData::Node *duplicateNode(QHashData::Node *node);
};

template <class Key, class T>
Q_INLINE_TEMPLATE QHashData::Node *QHash<Key, T>::duplicateNode(QHashData::Node *node)
{
    Node *concreteNode = concrete(node);
    return reinterpret_cast<QHashData::Node *>(new Node(concreteNode->key, concreteNode->value));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::Node *
QHash<Key, T>::createNode(uint h, const Key &key, const T &value, Node **nextNode)
{
    Node *node = new Node(key, value);
    node->h = h;
    node->next = *nextNode;
    *nextNode = node;
    ++d->size;
    return node;
}

template <class Key, class T>
Q_INLINE_TEMPLATE QHash<Key, T> &QHash<Key, T>::operator+=(const QHash<Key, T> &other)
{
    typename QHash<Key, T>::ConstIterator it = other.begin();
    while (it != other.end()) {
	insert(it.key(), it.value());
	++it;
    }
    return *this;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QHash<Key, T>::freeData(QHashData *x)
{
    Node *e_for_x = reinterpret_cast<Node *>(x);
    Node **bucket = reinterpret_cast<Node **>(x->buckets);
    int n = x->numBuckets;
    while (n--) {
	Node *cur = *bucket++;
	while (cur != e_for_x) {
	    Node *next = cur->next;
	    delete cur;
	    cur = next;
	}
    }
    delete [] x->buckets;
    delete x;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::clear()
{
    *this = QHash<Key,T>();
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QHash<Key, T>::detach_helper()
{
    QHashData *x = d->detach_helper(duplicateNode);
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
  	freeData(x);
}

template <class Key, class T>
Q_INLINE_TEMPLATE QHash<Key, T> &QHash<Key, T>::operator=(const QHash<Key, T> &other)
{
    if (d != other.d) {
	QHashData *x = other.d;
	++x->ref;
	x = qAtomicSetPtr(&d, x);
	if (!--x->ref)
	    freeData(x);
    }
    return *this;
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::value(const Key &key) const
{
    Node *node = *findNode(key);
    if (node == e) {
	T t;
	qInit(t);
	return t;
    } else {
	return node->value;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::value(const Key &key, const T &defaultValue) const
{
    Node *node = *findNode(key);
    if (node == e) {
	return defaultValue;
    } else {
	return node->value;
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QHash<Key, T>::values(const Key &key) const
{
    QList<T> list;
    Node *node = *findNode(key);
    if (node != e) {
	do {
	    list.append(node->value);
	} while ((node = node->next) != e && node->key == key);
    }
    return list;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QHash<Key, T>::count(const Key &key) const
{
    int cnt = 0;
    Node *node = *findNode(key);
    if (node != e) {
	do {
	    ++cnt;
	} while ((node = node->next) != e && node->key == key);
    }
    return cnt;
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::operator[](const Key &key) const
{
    return value(key);
}

template <class Key, class T>
Q_INLINE_TEMPLATE T &QHash<Key, T>::operator[](const Key &key)
{
    detach();
    d->mightGrow();

    uint h;
    Node **node = findNode(key, &h);
    if (*node == e)
	return createNode(h, key, T(), node)->value;
    return (*node)->value;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::Iterator QHash<Key, T>::insert(const Key &key,
									 const T &value)
{
    detach();
    d->mightGrow();

    uint h;
    Node **node = findNode(key, &h);
    if (*node == e)
	return Iterator(createNode(h, key, value, node));

    if (d->autoDelete == this)
	qDelete((*node)->value);
    (*node)->value = value;
    return Iterator(*node);
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::Iterator QHash<Key, T>::insertMulti(const Key &key,
									      const T &value)
{
    detach();
    d->mightGrow();

    uint h;
    Node **nextNode = findNode(key, &h);
    return Iterator(createNode(h, key, value, nextNode));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QHash<Key, T>::remove(const Key &key)
{
    detach();

    int oldSize = d->size;
    Node **node = findNode(key);
    if ((*node) != e) {
	do {
	    if (d->autoDelete == this)
	        qDelete((*node)->value);
	    Node *next = (*node)->next;
	    delete *node;
	    *node = next;
	    --d->size;
	} while ((*node) != e && (*node)->key == key);
        d->hasShrunk();
    }
    return oldSize - d->size;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE T QHash<Key, T>::take(const Key &key)
{
    detach();

    Node **node = findNode(key);
    T t;
    if ((*node) != e) {
	t = (*node)->value;
	Node *next = (*node)->next;
	delete *node;
	*node = next;
        --d->size;
	d->hasShrunk();
    } else {
	qInit(t);
    }
    return t;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QHash<Key, T>::Iterator QHash<Key, T>::erase(Iterator it)
{
    Iterator ret = it;
    ++ret;

    Node *node = it;
    Node **node_ptr = reinterpret_cast<Node **>(&d->buckets[node->h % d->numBuckets]);
    while (*node_ptr != node)
	node_ptr = &(*node_ptr)->next;
    *node_ptr = node->next;
    if (d->autoDelete == this)
	qDelete(node->value);
    delete node;
    --d->size;
    d->hasShrunk();
    return ret;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::setAutoDelete(bool enable)
{
    Q_ASSERT_X(QTypeInfo<T>::isPointer,
	       "QHash<Key,T>::setAutoDelete", "Cannot delete non-pointer types");
    detach();
    d->autoDelete = enable ? this : 0;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::reserve(int size)
{
    detach();
    d->rehash(-qMax(size, 1));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::ConstIterator QHash<Key, T>::find(const Key &key) const
{
    return ConstIterator(*findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::Iterator QHash<Key, T>::find(const Key &key)
{
    detach();
    return Iterator(*findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE bool QHash<Key, T>::contains(const Key &key) const
{
    return *findNode(key) != e;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QHash<Key, T>::Node **QHash<Key, T>::findNode(const Key &key,
									    uint *hp) const
{
    Node **node;
    uint h = qHash(key);

    if (d->numBuckets) {
	node = reinterpret_cast<Node **>(&d->buckets[h % d->numBuckets]);
	while (*node != e && !(*node)->same_key(h, key))
	    node = &(*node)->next;
    } else {
	node = const_cast<Node **>(reinterpret_cast<const Node * const *>(&e));
    }
    if (hp)
	*hp = h;
    return node;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE bool QHash<Key, T>::operator==(const QHash<Key, T> &other) const
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

Q_DECLARE_ASSOCIATIVE_ITERATOR(QHash)

#endif // QHASH_H

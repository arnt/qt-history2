#ifndef QHASH_H
#define QHASH_H

#ifndef QT_H
#include "qiterator.h"
#include "qatomic.h"
#endif // QT_H

class QByteArray;
class QString;

inline uint qHash(char key) { return (uint) key; }
inline uint qHash(signed char key) { return (uint) key; }
inline uint qHash(unsigned char key) { return (uint) key; }
inline uint qHash(signed short key) { return (uint) key; }
inline uint qHash(unsigned short key) { return (uint) key; }
inline uint qHash(signed int key) { return (uint) key; }
inline uint qHash(unsigned int key) { return (uint) key; }
inline uint qHash(signed long key) { return (uint) key; }
inline uint qHash(unsigned long key) { return (uint) key; }

uint qHash(const QByteArray &key);
uint qHash(const QString &key);

template <class T>
inline uint qHash(T *key) { return (uint) key; }
template <class T>
inline uint qHash(const T *key) { return (uint) key; }

struct QHashData
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

    QHashData *detach_helper(Node *(*node_duplicate)(Node *));
    void grow();
    void shrink();
    void rehash(int hint);
    Node *first_node();
    static Node *next_node(Node *node);

    static QHashData shared_null;
};

inline void QHashData::grow()
{
    ++size;
    if (size > numBuckets)
	rehash(numBits + 1);
}

inline void QHashData::shrink()
{
    --size;
    if (size == (numBuckets >> 3) && numBits > userNumBits)
	rehash(QMAX(numBits - 2, userNumBits));
}

inline QHashData::Node *QHashData::first_node()
{
    Node *e = (Node *) this;
    Node **bucket = buckets;
    int n = numBuckets;
    while (n--) {
	if (*bucket != e)
	    return *bucket;
	++bucket;
    }
    return e;
}

inline QHashData::Node *QHashData::next_node(Node *node)
{
    union {
	Node *next;
	Node *e;
	QHashData *d;
    };
    next = node->next;

    if (!next->next) {
	int start = (node->h % d->numBuckets) + 1;
	Node **bucket = d->buckets + start;
	int n = d->numBuckets - start;
	while (n--) {
	    if (*bucket != e)
		return *bucket;
	    ++bucket;
	}
    }
    return next;
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
    inline bool same_key(uint h0, const Key &key0)
    { return h0 == h && key0 == key; }
};

#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
#define Q_HASH_DECLARE_INT_NODE(key_type) \
    template <class T> \
    struct QHashNode<key_type, T> { \
	QHashNode *next; \
	union { uint h; key_type key; }; \
	T value; \
	inline QHashNode(key_type, const T &value0) : value(value0) { } \
	inline bool same_key(uint h0, key_type) { return h0 == h; } \
    };

Q_HASH_DECLARE_INT_NODE(signed int)
Q_HASH_DECLARE_INT_NODE(unsigned int)
#undef Q_HASH_DECLARE_INT_NODE
#endif // QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION

template <class Key, class T>
class QHash
{
    typedef QHashNode<Key, T> Node;

    union {
	QHashData *d;
	Node *e;
    };

public:
    inline QHash() : d(&QHashData::shared_null) { ++d->ref; }
    inline QHash(const QHash<Key, T> &h) : d(h.d) { ++d->ref; }
    inline ~QHash() { if (!--d->ref) free(d); }

    QHash &operator=(const QHash &h);

    void clear();

    const T value(const Key &key) const;
    const T value(const Key &key, const T &defaultValue) const;
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;
    void insert(const Key &key, const T &value);
    bool remove(const Key &key);
    void reserve(int size);
    inline int capacity() const { return d->numBuckets; }
    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    class Iterator
    {
	Node *i;
    public:
	inline operator Node *() const { return i; }
	explicit inline Iterator(void *node = 0) { i = (Node *) node; }
	inline const Key &key() const { return i->key; }
	inline T &value() const { return i->value; }
#ifndef QT_NO_COMPAT
	inline T &data() const { return i->value; }
#endif
	inline T &operator*() const { return i->value; }
	inline bool operator==(const Iterator &o) { return i == o.i; }
	inline bool operator!=(const Iterator &o) { return i != o.i; }
	inline Iterator operator++() {
	    i = (Node *) QHashData::next_node((QHashData::Node *) i);
	    return *this;
	}
	inline Iterator operator++(int) {
	    Iterator r = *this;
	    i = (Node *) QHashData::next_node((QHashData::Node *) i);
	    return r;
	}
    };
    friend class Iterator;

    class ConstIterator
    {
	Node *i;
    public:
	inline operator Node *() const { return i; }
	inline ConstIterator(const Iterator &o)
	{ i = ((ConstIterator&) o).i; }
	explicit inline ConstIterator(void *node = 0) { i = (Node *) node; }
	inline const Key &key() const { return i->key; }
	inline const T &value() const { return i->value; }
#ifndef QT_NO_COMPAT
	inline const T &data() const { return i->value; }
#endif
	inline const T &operator*() const { return i->value; }
	inline bool operator==(const ConstIterator &o) { return i == o.i; }
	inline bool operator!=(const ConstIterator &o) { return i != o.i; }
	inline ConstIterator operator++() {
	    i = (Node *) QHashData::next_node((QHashData::Node *) i);
	    return *this;
	}
	inline ConstIterator operator++(int) {
	    ConstIterator r = *this;
	    i = (Node *) QHashData::next_node((QHashData::Node *) i);
	    return r;
	}
    };
    friend class ConstIterator;

    Iterator find(const Key &key);
    ConstIterator find(const Key &key) const;
    bool contains(const Key &key) const;
    inline int size() const { return d->size; }
    inline bool isEmpty() const { return !d->size; }

    inline Iterator begin() { detach(); return Iterator(d->first_node()); }
    inline ConstIterator begin() const { return ConstIterator(d->first_node()); }
    inline ConstIterator constBegin() const { return ConstIterator(d->first_node()); }
    inline Iterator end() { return Iterator(e); }
    inline ConstIterator end() const { return ConstIterator(e); }
    inline ConstIterator constEnd() const { return ConstIterator(e); }
    Iterator erase(Iterator it);

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;

#ifndef QT_NO_COMPAT
    inline Iterator remove(Iterator it) { return erase(it); }
    inline int count() const { return d->size; }
#endif

    inline bool ensure_constructed()
    { if (!d) { d = &QHashData::shared_null; ++d->ref; return false; } return true; }

private:
    void detach_helper();
    void free(QHashData* d);
    Node * &node_find(const Key &key, uint *hp = 0) const;
    Node *node_create(uint h, const Key &key, const T &value);
    static QHashData::Node *node_duplicate(QHashData::Node *node);
};

template <class Key, class T>
QHashData::Node *QHash<Key, T>::node_duplicate(QHashData::Node *node)
{
    Node *c = (Node *) node;
    return (QHashData::Node *) new Node(c->key, c->value);
}

template <class Key, class T>
inline typename QHash<Key, T>::Node *QHash<Key, T>::node_create(uint h, const Key &key,
							const T &value)
{
    d->grow();
    Node *node = new Node(key, value);
    node->h = h;
    Node **bucket = (Node **) &d->buckets[h % d->numBuckets];
    node->next = *bucket;
    *bucket = node;
    return node;
}

template <class Key, class T>
void QHash<Key, T>::free(QHashData *d)
{
    Node **bucket = (Node **) d->buckets;
    int n = d->numBuckets;
    while (n--) {
	Node *cur = *bucket++;
	while (cur != e) {
	    Node *next = cur->next;
	    delete cur;
	    cur = next;
	}
    }
    delete [] d->buckets;
}

template <class Key, class T>
inline void QHash<Key, T>::clear()
{
    *this = QHash<Key,T>();
}

template <class Key, class T>
void QHash<Key, T>::detach_helper()
{
    QHashData *x = d->detach_helper(node_duplicate);
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
  	free(x);
}

template <class Key, class T>
inline QHash<Key, T> &QHash<Key, T>::operator=(const QHash &h)
{
    if (d != h.d) {
	QHashData *x = h.d;
	++x->ref;
	x = qAtomicSetPtr( &d, x );
	if (!--x->ref)
	    free(x);
    }
    return *this;
}

template <class Key, class T>
inline const T QHash<Key, T>::value(const Key &key) const
{
    Node *node = node_find(key);
    if (node == e) {
	T t;
	qInit(t);
	return t;
    } else {
	return node->value;
    }
}

template <class Key, class T>
inline const T QHash<Key, T>::value(const Key &key,
				     const T &defaultValue) const
{
    Node *node = node_find(key);
    if (node == e) {
	return defaultValue;
    } else {
	return node->value;
    }
}

template <class Key, class T>
inline const T QHash<Key, T>::operator[](const Key &key) const
{
    return value(key);
}

template <class Key, class T>
inline T &QHash<Key, T>::operator[](const Key &key)
{
    detach();

    uint h;
    Node *node = node_find(key, &h);
    if (node == e)
	node = node_create(h, key, T());
    return node->value;
}

template <class Key, class T>
inline void QHash<Key, T>::insert(const Key &key, const T &value)
{
    detach();

    uint h;
    Node *node = node_find(key, &h);
    if (node == e) {
	node_create(h, key, value);
    } else {
	node->value = value;
    }
}

template <class Key, class T>
bool QHash<Key, T>::remove(const Key &key)
{
    detach();

    Node * &node = node_find(key);
    bool found = (node != e);
    if (found) {
	Node *next = node->next;
	delete node;
	node = next;
	d->shrink();
    }
    return found;
}

template <class Key, class T>
typename QHash<Key, T>::Iterator QHash<Key, T>::erase(Iterator it)
{
    Iterator n = it;
    ++n;
    Node **node = &d->buckets[it.i->h % d->numBuckets];
    while (*node != it.i)
	node = &node->next;
    Node *next = node->next;
    delete node;
    node = next;
    d->shrink();
    return n;
}

template <class Key, class T>
inline void QHash<Key, T>::reserve(int size)
{
    detach();
    d->rehash(QMIN(-size, -1));
}

template <class Key, class T>
inline typename QHash<Key, T>::ConstIterator QHash<Key, T>::find(const Key &key) const
{
    return ConstIterator(node_find(key));
}

template <class Key, class T>
inline typename QHash<Key, T>::Iterator QHash<Key, T>::find(const Key &key)
{
    detach();
    return Iterator(node_find(key));
}

template <class Key, class T>
inline bool QHash<Key, T>::contains(const Key &key) const
{
    return node_find(key) != e;
}

template <class Key, class T>
typename QHash<Key, T>::Node * &QHash<Key, T>::node_find(const Key &key,
						 uint *hp) const
{
    Node **node;
    uint h = qHash(key);

    if (d->numBuckets) {
	node = (Node **) &d->buckets[h % d->numBuckets];
	while (*node != e && !(*node)->same_key(h, key))
	    node = &(*node)->next;
    } else {
	node = (Node **) &e;
    }
    if (hp)
	*hp = h;
    return *node;
}

Q_DECLARE_ASSOCIATIVE_ITERATOR(QHash)

#endif

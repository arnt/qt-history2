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

#ifndef QHASH_H
#define QHASH_H

#include "qatomic.h"
#include "qiterator.h"
#include "qlist.h"

#ifndef QT_NO_STL
#include <iterator>
#endif

class QByteArray;
class QString;

inline uint qHash(char key) { return uint(key); }
inline uint qHash(uchar key) { return uint(key); }
inline uint qHash(signed char key) { return uint(key); }
inline uint qHash(ushort key) { return uint(key); }
inline uint qHash(short key) { return uint(key); }
inline uint qHash(uint key) { return uint(key); }
inline uint qHash(int key) { return uint(key); }
inline uint qHash(ulong key)
{
    if (sizeof(ulong) > sizeof(uint)) {
        return uint((key >> (8 * sizeof(uint) - 1)) ^ key);
    } else {
        return uint(key);
    }
}
inline uint qHash(long key) { return qHash(ulong(key)); }
inline uint qHash(Q_ULONGLONG key)
{
    if (sizeof(Q_ULONGLONG) > sizeof(uint)) {
        return uint((key >> (8 * sizeof(uint) - 1)) ^ key);
    } else {
        return uint(key);
    }
}
inline uint qHash(Q_LONGLONG key) { return qHash(Q_ULONGLONG(key)); }
inline uint qHash(QChar key) { return qHash(key.unicode()); }
Q_CORE_EXPORT uint qHash(const QByteArray &key);
Q_CORE_EXPORT uint qHash(const QString &key);

template <class T> inline uint qHash(const T *key)
{
    if (sizeof(const T *) > sizeof(uint))
        return qHash(reinterpret_cast<Q_ULONGLONG>(key));
    else
        return uint(reinterpret_cast<ulong>(key));
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
    uint sharable : 1;

    QHashData *detach_helper(Node *(*node_duplicate)(Node *));
    void mightGrow();
    void hasShrunk();
    void rehash(int hint);
    void free();
    Node *firstNode();
    static Node *nextNode(Node *node);
    static Node *previousNode(Node *node);

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
    }

#if defined(Q_BYTE_ORDER) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN
Q_HASH_DECLARE_INT_NODE(short);
Q_HASH_DECLARE_INT_NODE(ushort);
#endif
Q_HASH_DECLARE_INT_NODE(int);
Q_HASH_DECLARE_INT_NODE(uint);
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
    inline QHash(const QHash<Key, T> &other) : d(other.d) { ++d->ref; if (!d->sharable) detach(); }
    inline ~QHash() { if (!--d->ref) freeData(d); }

    QHash<Key, T> &operator=(const QHash<Key, T> &other);

    bool operator==(const QHash<Key, T> &other) const;
    inline bool operator!=(const QHash<Key, T> &other) const { return !(*this == other); }

    inline int size() const { return d->size; }

    inline bool isEmpty() const { return d->size == 0; }

    inline int capacity() const { return d->numBuckets; }
    void reserve(int size);
    inline void squeeze() { reserve(1); }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }
    inline void setSharable(bool sharable) { if (!sharable) detach(); d->sharable = sharable; }

    static inline bool sameKey(const Key &key1, const Key &key2)
        { return key1 == key2; }

    void clear();

    int remove(const Key &key);
    T take(const Key &key);

    bool contains(const Key &key) const;
    const Key key(const T &value) const;
    const T value(const Key &key) const;
    const T value(const Key &key, const T &defaultValue) const;
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;

    QList<Key> keys() const;
    QList<Key> keys(const T &value) const;
    QList<T> values() const;
    QList<T> values(const Key &key) const;
    int count(const Key &key) const;

    class iterator
    {
        QHashData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline operator Node *() const { return concrete(i); }
        inline iterator() : i(0) { }
        explicit inline iterator(void *node) : i(reinterpret_cast<QHashData::Node *>(node)) { }

        inline const Key &key() const { return concrete(i)->key; }
        inline T &value() const { return concrete(i)->value; }
        inline T &operator*() const { return concrete(i)->value; }
        inline T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }

        inline iterator operator++() {
            i = QHashData::nextNode(i);
            return *this;
        }
        inline iterator operator++(int) {
            iterator r = *this;
            i = QHashData::nextNode(i);
            return r;
        }
        inline iterator operator--() {
            i = QHashData::previousNode(i);
            return *this;
        }
        inline iterator operator--(int) {
            iterator r = *this;
            i = QHashData::previousNode(i);
            return r;
        }
        inline iterator operator+(int j) const
        { iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r; }
        inline iterator operator-(int j) const { return operator+(-j); }
        inline iterator &operator+=(int j) { return *this = *this + j; }
        inline iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class iterator;

    class const_iterator
    {
        QHashData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline operator Node *() const { return concrete(i); }
        inline const_iterator() : i(0) { }
        explicit inline const_iterator(void *node)
            : i(reinterpret_cast<QHashData::Node *>(node)) { }
        inline const_iterator(const iterator &o)
        { i = reinterpret_cast<const const_iterator &>(o).i; }

        inline const Key &key() const { return concrete(i)->key; }
        inline const T &value() const { return concrete(i)->value; }
        inline const T &operator*() const { return concrete(i)->value; }
        inline const T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }

        inline const_iterator operator++() {
            i = QHashData::nextNode(i);
            return *this;
        }
        inline const_iterator operator++(int) {
            const_iterator r = *this;
            i = QHashData::nextNode(i);
            return r;
        }
        inline const_iterator operator--() {
            i = QHashData::previousNode(i);
            return *this;
        }
        inline const_iterator operator--(int) {
            iterator r = *this;
            i = QHashData::previousNode(i);
            return r;
        }
        inline const_iterator operator+(int j) const
        { const_iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r; }
        inline const_iterator operator-(int j) const { return operator+(-j); }
        inline const_iterator &operator+=(int j) { return *this = *this + j; }
        inline const_iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class const_iterator;

    // STL style
    inline iterator begin() { detach(); return iterator(d->firstNode()); }
    inline const_iterator begin() const { return const_iterator(d->firstNode()); }
    inline const_iterator constBegin() const { return const_iterator(d->firstNode()); }
    inline iterator end() { detach(); return iterator(e); }
    inline const_iterator end() const { return const_iterator(e); }
    inline const_iterator constEnd() const { return const_iterator(e); }
    iterator erase(iterator it);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    inline int count() const { return d->size; }
    iterator find(const Key &key);
    const_iterator find(const Key &key) const;
    iterator insert(const Key &key, const T &value);
    iterator insertMulti(const Key &key, const T &value);
    QHash<Key, T> &merge(const QHash<Key, T> &other);

    // STL compatibility
    inline bool empty() const { return isEmpty(); }

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
Q_INLINE_TEMPLATE QHash<Key, T> &QHash<Key, T>::merge(const QHash<Key, T> &other)
{
    QHash<Key, T> copy(other);
    const_iterator it = copy.constEnd();
    while (it != copy.constBegin()) {
        --it;
        insertMulti(it.key(), it.value());
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
    x->free();
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
        if (!d->sharable)
            detach_helper();
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
Q_OUTOFLINE_TEMPLATE QList<Key> QHash<Key, T>::keys() const
{
    QList<Key> res;
    const_iterator i = begin();
    while (i != end()) {
        res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QHash<Key, T>::keys(const T &value) const
{
    QList<Key> res;
    const_iterator i = begin();
    while (i != end()) {
        if (i.value() == value)
            res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE const Key QHash<Key, T>::key(const T &value) const
{
    const_iterator i = begin();
    while (i != end()) {
        if (i.value() == value)
            return i.key();
        ++i;
    }

    Key k;
    qInit(k);
    return k;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QHash<Key, T>::values() const
{
    QList<T> res;
    const_iterator i = begin();
    while (i != end()) {
        res.append(i.value());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QHash<Key, T>::values(const Key &key) const
{
    QList<T> res;
    Node *node = *findNode(key);
    if (node != e) {
        do {
            res.append(node->value);
        } while ((node = node->next) != e && node->key == key);
    }
    return res;
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
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::insert(const Key &key,
                                                                         const T &value)
{
    detach();
    d->mightGrow();

    uint h;
    Node **node = findNode(key, &h);
    if (*node == e)
        return iterator(createNode(h, key, value, node));

    (*node)->value = value;
    return iterator(*node);
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::insertMulti(const Key &key,
                                                                              const T &value)
{
    detach();
    d->mightGrow();

    uint h;
    Node **nextNode = findNode(key, &h);
    return iterator(createNode(h, key, value, nextNode));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QHash<Key, T>::remove(const Key &key)
{
    detach();

    int oldSize = d->size;
    Node **node = findNode(key);
    if (*node != e) {
        bool deleteNext = true;
        do {
            Node *next = (*node)->next;
            deleteNext = (next != e && next->key == (*node)->key);
            delete *node;
            *node = next;
            --d->size;
        } while (deleteNext);
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
    if (*node != e) {
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
Q_OUTOFLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::erase(iterator it)
{
    if (it == iterator(e))
        return it;

    iterator ret = it;
    ++ret;

    Node *node = it;
    Node **node_ptr = reinterpret_cast<Node **>(&d->buckets[node->h % d->numBuckets]);
    while (*node_ptr != node)
        node_ptr = &(*node_ptr)->next;
    *node_ptr = node->next;
    delete node;
    --d->size;
    return ret;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::reserve(int size)
{
    detach();
    d->rehash(-qMax(size, 1));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::const_iterator QHash<Key, T>::find(const Key &key) const
{
    return const_iterator(*findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::find(const Key &key)
{
    detach();
    return iterator(*findNode(key));
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

    const_iterator it1 = begin();
    const_iterator it2 = other.begin();

    while (it1 != end()) {
        if (!(it1.key() == it2.key()) || !(it1.value() == it2.value()))
            return false;
        ++it2;
        ++it1;
    }
    return true;
}

template <class Key, class T>
class QMultiHash : public QHash<Key, T>
{
public:
    QMultiHash() {}
    QMultiHash(const QHash<Key, T> &other) : QHash<Key, T>(other) {}

    inline typename QHash<Key, T>::iterator replace(const Key &key, const T &value)
    { return QHash<Key, T>::insert(key, value); }
    inline typename QHash<Key, T>::iterator insert(const Key &key, const T &value)
    { return QHash<Key, T>::insertMulti(key, value); }

    inline QMultiHash &operator+=(const QMultiHash &other)
    { merge(other); return *this; }
    inline QMultiHash operator+(const QMultiHash &other) const
    { QMultiHash result = *this; result += other; return result; }

private:
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;
};

Q_DECLARE_ASSOCIATIVE_ITERATOR(QHash)

#endif // QHASH_H

#ifndef QMAP_H
#define QMAP_H

#ifndef QT_H
#include "qatomic.h"
#include "qiterator.h"
#include "qlist.h"
#endif

#ifndef QT_NO_STL
#include <iterator>
#include <map>
#endif

#include <new>

struct Q_CORE_EXPORT QMapData
{
    struct Node {
        Node *backward;
        Node *forward[1];
    };
    enum { LastLevel = 11, Sparseness = 3 };

    Node *backward;
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
#ifdef QT_QMAP_DEBUG
    uint adjust_ptr(Node *node);
    void dump();
#endif

    static QMapData shared_null;
};

#if !defined(QT_NO_DATASTREAM)
class QDataStream;
template <class Key, class T> class QMap;
template <class aKey, class aT>
QDataStream &operator>>(QDataStream &in, QMap<aKey, aT> &map);
#endif

template <class Key, class T>
class QMap
{
    struct Node {
        Key key;
        T value;
        QMapData::Node *backward;
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
public:
    inline QMap() : d(&QMapData::shared_null) { ++d->ref; }
    inline QMap(const QMap<Key, T> &other) : d(other.d) { ++d->ref; }
    inline ~QMap() { if (!d) return; if (!--d->ref) freeData(d); }

    QMap<Key, T> &operator=(const QMap<Key, T> &other);
#ifndef QT_NO_STL
    QMap(const typename std::map<Key, T> &other);
    QMap<Key, T> &operator=(const typename std::map<Key,T> &other);
#endif

    bool operator==(const QMap<Key, T> &other) const;
    inline bool operator!=(const QMap<Key, T> &other) const { return !(*this == other); }

    inline int size() const { return d->size; }

    inline bool isEmpty() const { return d->size == 0; }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }
    static inline bool sameKey(const Key &key1, const Key &key2)
        { return !(key1 < key2 || key2 < key1); }

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
        QMapData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline operator QMapData::Node *() const { return i; }
        inline iterator() : i(0) { }
        inline iterator(QMapData::Node *node) : i(node) { }

        inline const Key &key() const { return concrete(i)->key; }
        inline T &value() const { return concrete(i)->value; }
#ifdef QT_COMPAT
        inline QT_COMPAT T &data() const { return concrete(i)->value; }
#endif
        inline T &operator*() const { return concrete(i)->value; }
        inline T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }

        inline iterator &operator++() {
            i = i->forward[0];
            return *this;
        }
        inline iterator operator++(int) {
            iterator r = *this;
            i = i->forward[0];
            return r;
        }
        inline iterator &operator--() {
            i = i->backward;
            return *this;
        }
        inline iterator operator--(int) {
            iterator r = *this;
            i = i->backward;
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
        QMapData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline operator QMapData::Node *() const { return i; }
        inline const_iterator() : i(0) { }
        inline const_iterator(QMapData::Node *node) : i(node) { }
        inline const_iterator(const iterator &o)
        { i = reinterpret_cast<const const_iterator &>(o).i; }

        inline const Key &key() const { return concrete(i)->key; }
        inline const T &value() const { return concrete(i)->value; }
#ifdef QT_COMPAT
        inline QT_COMPAT const T &data() const { return concrete(i)->value; }
#endif
        inline const T &operator*() const { return concrete(i)->value; }
        inline const T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }

        inline const_iterator &operator++() {
            i = i->forward[0];
            return *this;
        }
        inline const_iterator operator++(int) {
            const_iterator r = *this;
            i = i->forward[0];
            return r;
        }
        inline const_iterator &operator--() {
            i = i->backward;
            return *this;
        }
        inline const_iterator operator--(int) {
            iterator r = *this;
            i = i->backward;
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
    inline iterator begin() { detach(); return iterator(e->forward[0]); }
    inline const_iterator begin() const { return const_iterator(e->forward[0]); }
    inline const_iterator constBegin() const { return const_iterator(e->forward[0]); }
    inline iterator end() {
        detach();
        return iterator(e);
    }
    inline const_iterator end() const { return const_iterator(e); }
    inline const_iterator constEnd() const { return const_iterator(e); }
    iterator erase(iterator it);
#ifdef QT_COMPAT
    inline QT_COMPAT iterator remove(iterator it) { return erase(it); }
#endif

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    inline int count() const { return d->size; }
    iterator find(const Key &key);
    const_iterator find(const Key &key) const;
    iterator lowerBound(const Key &key);
    const_iterator lowerBound(const Key &key) const;
    iterator upperBound(const Key &key);
    const_iterator upperBound(const Key &key) const;
    iterator insert(const Key &key, const T &value);
#ifdef QT_COMPAT
    QT_COMPAT iterator insert(const Key &key, const T &value, bool overwrite);
#endif
    iterator insertMulti(const Key &key, const T &value);
#ifdef QT_COMPAT
    inline QT_COMPAT iterator replace(const Key &key, const T &value) { return insert(key, value); }
#endif
    QMap<Key, T> &merge(const QMap<Key, T> &other);

    // STL compatibility
    inline bool empty() const { return isEmpty(); }

private:
    void detach_helper();
    void freeData(QMapData *d);
    QMapData::Node *findNode(const Key &key) const;
    QMapData::Node *mutableFindNode(QMapData::Node *update[], const Key &key) const;
    QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key,
                                const T &value);

#if !defined(QT_NO_DATASTREAM)
#if defined Q_CC_MSVC && _MSC_VER < 1300
    friend QDataStream &operator>> (QDataStream &in, QMap &map);
#else
    template <class aKey, class aT> 
    friend QDataStream &operator>> (QDataStream &in, QMap<aKey, aT> &map);
#endif
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
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::insert(const Key &key,
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
    return iterator(node);
}

#ifdef QT_COMPAT
template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::insert(const Key &key,
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
    return iterator(node);
}
#endif

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::insertMulti(const Key &key,
                                                                            const T &value)
{
    detach();

    QMapData::Node *update[QMapData::LastLevel + 1];
    mutableFindNode(update, key);
    return iterator(node_create(d, update, key, value));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::const_iterator QMap<Key, T>::find(const Key &key) const
{
    return const_iterator(findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::find(const Key &key)
{
    detach();
    return iterator(findNode(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE QMap<Key, T> &QMap<Key, T>::merge(const QMap<Key, T> &other)
{
    QMap<Key, T> copy(other);
    const_iterator it = copy.constEnd();
    while (it != copy.constBegin()) {
        --it;
        insertMulti(it.key(), it.value());
    }
    return *this;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T>::freeData(QMapData *x)
{
    if (QTypeInfo<Key>::isComplex || QTypeInfo<T>::isComplex) {
        QMapData::Node *y = reinterpret_cast<QMapData::Node *>(x);
        QMapData::Node *cur = y;
        QMapData::Node *next = cur->forward[0];
        while (next != y) {
            cur = next;
            next = cur->forward[0];
            Node *concreteNode = concrete(cur);
            concreteNode->key.~Key();
            concreteNode->value.~T();
        }
    }
    x->continueFreeData(sizeof(Payload));
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
        t = concrete(next)->value;
        concrete(next)->key.~Key();
        concrete(next)->value.~T();
        d->node_delete(update, sizeof(Payload), next);
    } else {
        qInit(t);
    }
    return t;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::erase(iterator it)
{
    QMapData::Node *update[QMapData::LastLevel + 1];
    QMapData::Node *cur = e;
    QMapData::Node *next = e;

    if (it == iterator(e))
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
            return iterator(next);
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
                                                                   const Key &key) const
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
    const_iterator i = begin();
    while (i != end()) {
        res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, T>::keys(const T &value) const
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
Q_OUTOFLINE_TEMPLATE const Key QMap<Key, T>::key(const T &value) const
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
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T>::values() const
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
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T>::values(const Key &key) const
{
    QList<T> res;
    QMapData::Node *node = findNode(key);
    if (node != e) {
        do {
            res.append(concrete(node)->value);
            node = node->forward[0];
        } while (node != e && !(key < concrete(node)->key));
    }
    return res;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::const_iterator
QMap<Key, T>::lowerBound(const Key &key) const
{
    QMapData::Node *update[QMapData::LastLevel + 1];
    mutableFindNode(update, key);
    return const_iterator(update[0]->forward[0]);
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::lowerBound(const Key &key)
{
    detach();
    return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->lowerBound(key));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::const_iterator
QMap<Key, T>::upperBound(const Key &key) const
{
    QMapData::Node *update[QMapData::LastLevel + 1];
    mutableFindNode(update, key);
    QMapData::Node *node = update[0]->forward[0];
    while (node != e && !(key < concrete(node)->key))
        node = node->forward[0];
    return const_iterator(node);
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QMap<Key, T>::iterator QMap<Key, T>::upperBound(const Key &key)
{
    detach();
    return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->upperBound(key));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE bool QMap<Key, T>::operator==(const QMap<Key, T> &other) const
{
    if (size() != other.size())
        return false;
    if (d == other.d)
        return true;

    const_iterator it1 = begin();
    const_iterator it2 = other.begin();

    while (it1 != end()) {
        if (!(it1.value() == it2.value()) || it1.key() < it2.key() || it2.key() < it1.key())
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

template <class Key, class T>
class QMultiMap : public QMap<Key, T>
{
public:
    QMultiMap() {}
    QMultiMap(const QMap<Key, T> &other) : QMap<Key, T>(other) {}

    inline typename QMap<Key, T>::iterator replace(const Key &key, const T &value)
    { return QMap<Key, T>::insert(key, value); }
    inline typename QMap<Key, T>::iterator insert(const Key &key, const T &value)
    { return QMap<Key, T>::insertMulti(key, value); }

    inline QMultiMap &operator+=(const QMultiMap &other)
    { merge(other); return *this; }
    inline QMultiMap operator+(const QMultiMap &other) const
    { QMultiMap result = *this; result += other; return result; }

private:
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;
};

Q_DECLARE_ASSOCIATIVE_ITERATOR(QMap)

#endif // QMAP_H

/****************************************************************************
**
** Definition of QMap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMAP_H
#define QMAP_H

#ifndef QT_H
#include "qglobal.h"
#include "qdatastream.h"
#include "qpair.h"
#include "qlist.h"
#include "qatomic.h"
#endif // QT_H

#ifndef QT_NO_STL
#include <iterator>
#include <map>
#endif

#include <qiterator.h>

struct Q_CORE_EXPORT QMapData
{
    enum Color { Red, Black };
    struct Node {
	Node *left;
	Node *right;
	Node *parent;
	uint color : 1;
	uint root : 1;
    };
    QAtomic ref;
    int node_count;
    Node header;

    static Node* minimum(Node *n) {
	while (n->left)
	    n = n->left;
	return n;
    }

    static Node* maximum(Node *n) {
	while (n->right)
	    n = n->right;
	return n;
    }

    static inline Node* next(Node *n) {
	if (n->right) {
	    n = n->right;
	    while (n->left)
		n = n->left;
	} else {
	    register Node* y = n->parent;
	    while (n == y->right) {
		n = y;
		y = y->parent;
	    }
	    if (n->right != y)
		n = y;
	}
	return n;
    }

    static inline Node* prev(Node *n) {
	if (n->root) {
	    // n == header
	    n = n->right;
	} else if (n->left) {
	    n = n->left;
	    while (n->right)
		n = n->right;
	} else {
	    register Node* y = n->parent;
	    while (n == y->left) {
		n = y;
		y = y->parent;
	    }
	    n = y;
	}
	return n;
    }

    void rebalance(Node* x);
    void removeAndRebalance(Node* z);

    static inline QMapData *init(QMapData *d)
    {
	d->ref = 1;
	d->header.color = Red;
	d->header.root = 1; // Mark the header
	d->header.parent = 0;
	d->header.left = d->header.right = &d->header;
	d->node_count = 0;
	return d;
    }

    static QMapData shared_null;
};

template<class Key, class T>
class QMap
{
public:
    struct Node : QMapData::Node {
	Node() { root = 0; }
	Node(const Key &k) : key(k) { root = 0; }
	Node(const Node &o) : QMapData::Node(o), key(o.key), data(o.data) { root = 0; }
	Key key;
	T data;
    };

    class Iterator
    {
    public:
#ifndef QT_NO_STL
	typedef std::bidirectional_iterator_tag  iterator_category;
#endif
	Node* n;

	Iterator() : n(0) {}
	Iterator(Node* p) : n(p) {}
	Iterator(const Iterator& it) : n(it.n) {}

	inline bool atEnd() const { return n->root; }

	bool operator==(const Iterator& it) const { return n == it.n; }
	bool operator!=(const Iterator& it) const { return n != it.n; }

	T& operator*() { Q_ASSERT(!atEnd()); return n->data; }
	const T& operator*() const { Q_ASSERT(!atEnd()); return n->data; }

	const Key& key() const { Q_ASSERT(!atEnd()); return n->key; }
	const T& value() const { Q_ASSERT(!atEnd()); return n->data; }
	T& value() { Q_ASSERT(!atEnd()); return n->data; }
	const T& data() const { Q_ASSERT(!atEnd()); return n->data; }
	T& data() { Q_ASSERT(!atEnd()); return n->data; }

	Iterator& operator++() {
	    n = static_cast<Node *>(QMapData::next(n));
	    return *this;
	}

	Iterator operator++(int) {
	    Iterator tmp = *this;
	    n = static_cast<Node *>(QMapData::next(n));
	    return tmp;
	}

	Iterator& operator--() {
	    n = static_cast<Node *>(QMapData::prev(n));
	    return *this;
	}

	Iterator operator--(int) {
	    Iterator tmp = *this;
	    n = static_cast<Node *>(QMapData::prev(n));
	    return tmp;
	}
    };


    class ConstIterator
    {
    public:
#ifndef QT_NO_STL
	typedef std::bidirectional_iterator_tag  iterator_category;
#endif

	Node* n;

	/**
	     * Functions
	     */
	ConstIterator() : n(0) {}
	ConstIterator(Node* p) : n(p) {}
	ConstIterator(const ConstIterator& it) : n(it.n) {}
	ConstIterator(const Iterator& it) : n(it.n) {}

	inline bool atEnd() const { return n->root; }

	bool operator==(const ConstIterator& it) const { return n == it.n; }
	bool operator!=(const ConstIterator& it) const { return n != it.n; }

	const T& operator*()  const { Q_ASSERT(!atEnd()); return n->data; }

	const Key& key() const { Q_ASSERT(!atEnd()); return n->key; }
	const T& data() const { Q_ASSERT(!atEnd()); return n->data; }
	const T& value() const { Q_ASSERT(!atEnd()); return n->data; }

	ConstIterator& operator++() {
	    n = static_cast<Node *>(QMapData::next(n));
	    return *this;
	}

	ConstIterator operator++(int) {
	    ConstIterator tmp = *this;
	    n = static_cast<Node *>(QMapData::next(n));
	    return tmp;
	}

	ConstIterator& operator--() {
	    n = static_cast<Node *>(QMapData::prev(n));
	    return *this;
	}

	ConstIterator operator--(int) {
	    ConstIterator tmp = *this;
	    n = static_cast<Node *>(QMapData::prev(n));
	    return tmp;
	}
    };


    typedef Key key_type;
    typedef T mapped_type;
    typedef QPair<const Key, T> value_type;
    typedef QPair<Iterator, bool> insert_pair;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef T ValueType;
#ifndef QT_NO_STL
    typedef ptrdiff_t  difference_type;
#else
    typedef int difference_type;
#endif
    typedef size_t      size_type;
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;



    QMap()
    {
	d = &QMapData::shared_null;
	++d->ref;
    }
    QMap(const QMap<Key,T>& m)
    {
	d = m.d; ++d->ref;
    }
    ~QMap()
    {
	if (!--d->ref) {
	    free(d->header.parent);
	    delete d;
	}
    }
    QMap<Key,T>& operator= (const QMap<Key,T>& m);

#ifndef QT_NO_STL
    QMap(const Q_TYPENAME std::map<Key,T>& m);
    QMap<Key,T>& operator= (const Q_TYPENAME std::map<Key,T>& m);
#endif

    inline Iterator begin() { detach(); return static_cast<Node *>(d->header.left); }
    inline Iterator end() { detach(); return static_cast<Node *>(&d->header); }
    inline ConstIterator begin() const { return static_cast<Node *>(d->header.left); }
    inline ConstIterator end() const { return static_cast<Node *>(&d->header); }
    inline ConstIterator constBegin() const { return static_cast<Node *>(d->header.left); }
    inline ConstIterator constEnd() const { return static_cast<Node *>(&d->header); }

    inline size_type size() const { return d->node_count; }
    inline size_type count() const { return d->node_count; }
    inline bool empty() const { return d->node_count == 0; }
    inline bool isEmpty() const { return d->node_count == 0; }
    inline size_type count(const key_type& k) const { return find(k) != end() ? 1 : 0; }

    void clear();

    QPair<Iterator,bool> insert(const value_type& x);
    Iterator insert(const Key& key, const T& value);
    Iterator insert(const Key& key, const T& value, bool overwrite);

    Iterator erase(Iterator it);
    void erase(const Key& k);
    inline void remove(Iterator it) { erase(it); }
    inline void remove(const Key& k) { erase(k); }

    Iterator replace(const Key& k, const T& v) { return insert(k, v); }

    bool contains (const Key& k) const { return findNode(k) != &d->header; }
    Iterator find (const Key& k) { detach(); return findNode(k); }
    ConstIterator find (const Key& k) const { return findNode(k); }
    T value(const Key &key) const;
    T value(const Key &key, const T &defaultValue) const;
    T& operator[] (const Key& k);
    inline T operator[] (const Key& k) const { return value(k);}


    QList<Key> keys() const;
    QList<T> values() const;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==(const QMap<Key,T>&) const { return FALSE; }
#ifndef QT_NO_STL
    bool operator==(const Q_TYPENAME std::map<Key,T>&) const { return FALSE; }
#endif
#endif

    void detach() {  if (d->ref != 1) detachInternal(); }

    inline bool ensure_constructed()
    { if (!d) { d = &QMapData::shared_null; ++d->ref; } return true; }

#ifdef QT_QMAP_DEBUG
    void inorder(QMapData::Node* x = 0, int level = 0)
    {
	if (!x)
	    x = d->header.parent;
	if (!x) {
	    std::cout << "empty" << std::endl;
	    return;
	}
	if (x->left)
	    inorder(x->left, level + 1);
	std::cout << level << " this=" << x << " Key=" << key(x) << std::endl;
	if (x->right)
	    inorder(x->right, level + 1);
    }
    static void inorder(QMapData *d, QMapData::Node* x = 0, int level = 0){
	if (!x)
	    x = d->header.parent;
	if (x) {
	    if (x->left)
		inorder(d, x->left, level + 1);
	    std::cout << level << " this=" << x << std::endl;
	    if (x->right)
		inorder(d, x->right, level + 1);
	}
    }
#endif

private:
    QMapData* d;

    void detachInternal();
    Node *findNode(const Key& k) const;
    static inline const Key& key(const QMapData::Node* b)
	{ return static_cast<const Node *>(b)->key; }
    static inline const T& value(const QMapData::Node* b)
	{ return static_cast<const Node *>(b)->data; }
    Node *insertSingle(const Key& k);
    static void free(QMapData::Node *p);
};

template<class Key, class T>
Q_INLINE_TEMPLATE void QMap<Key,T>::clear()
{ *this = QMap<Key, T>(); }

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key,T>::detachInternal()
{
    QMapData *x = QMapData::init(new QMapData);

    if (d->node_count) {
        const QMapData::Node *o = d->header.parent;

        QMapData::Node *p = &x->header;

        bool left = true;
        while (o != &d->header) {
            QMapData::Node *n = new Node(*static_cast<const Node *>(o));
            n->left = n->right = 0;
            n->parent = p;
            if (left)
                p->left = n;
            else
                p->right = n;

            if (o->left) {
                left = true;
                p = n;
                n = n->left;
                o = o->left;
            } else {
                left = false;
                if (!o->right) {
                    n->right = 0;
                    while ((!o->parent->right || o->parent->right == o) &&
			    o->parent != &d->header) {
                        o = o->parent;
                        n = n->parent;
                    }
                    o = o->parent;
                    n = n->parent;
                }
                if (o->right && o != &d->header) {
                    p = n;
                    n = n->right;
                    o = o->right;
                }
            }
        }

        x->header.parent = x->header.left;
        x->header.left = QMapData::minimum(x->header.parent);
        x->header.right = QMapData::maximum(x->header.parent);
        x->node_count = d->node_count;
    }

    x = qAtomicSetPtr(&d, x);
    if (!--x->ref) {
        free(x->header.parent);
	delete x;
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<Key, T>::Node *QMap<Key, T>::insertSingle(const Key& k)
{
    // Search correct position in the tree
    QMapData::Node* y = &d->header;
    QMapData::Node* x = y->parent;
    QMapData::Node *left = 0;
    while (x) {
	y = x;
	if (k < key(x)) {
	    x = x->left;
	} else {
	    left = x;
	    x = x->right;
	}
    }
    if (left && !(key(left) < k)) {
	y = left;
    } else {
	// have to create a new node
	Node *z = new Node(k);
	if (y == &d->header || y != left
	   ) {
	    y->left = z;                // also makes leftmost = z when y == header
	    if (y == &d->header) {
		d->header.parent = z;
		d->header.right = z;
	    } else if (y == d->header.left)
		d->header.left = z;           // maintain leftmost pointing to min node
	} else {
	    y->right = z;
	    if (y == d->header.right)
		d->header.right = z;          // maintain rightmost pointing to max node
	}
	z->parent = y;
	z->left = 0;
	z->right = 0;
	d->rebalance(z);
	++d->node_count;
	y = z;
    }
    return static_cast<Node *>(y);
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<Key, T>::Node *QMap<Key, T>::findNode(const Key& k) const
{
    QMapData::Node* y = &d->header;        // Last node
    QMapData::Node* x = y->parent; // Root node.

    while (x != 0) {
	// If as k <= key(x) go left
	if (!(key(x) < k)) {
	    y = x;
	    x = x->left;
	} else {
	    x = x->right;
	}
    }

    // Was k bigger/smaller then the biggest/smallest
    // element of the tree ? Return end()
    if (y == &d->header || k < key(y))
	y = &d->header;
    return static_cast<Node *>(y);
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key,T>::free(QMapData::Node *p)
{
    while ( p != 0 ) {
	free(p->right);
	QMapData::Node *y = p->left;
	delete static_cast<Node *>(p);
	p = y;
    }
}

template<class K, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<K, T>::Iterator QMap<K, T>::erase(Iterator it)
{
    detach();
    Iterator n = it;
    ++n;
    d->removeAndRebalance(it.n);
    delete it.n;
    --d->node_count;
    return n;
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE void QMap<Key,T>::erase(const Key& k)
{
    detach();
    Node *n = findNode(k);
    if (n != &d->header) {
	d->removeAndRebalance(n);
        delete n;
	--d->node_count;
    }
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key,T>& QMap<Key,T>::operator= (const QMap<Key,T>& m)
{
    QMapData *x = m.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref) {
	free(x->header.parent);
	delete x;
    }
    return *this;
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<Key,T>::insert_pair QMap<Key,T>::insert(const Q_TYPENAME QMap<Key,T>::value_type& x)
{
    detach();
    size_type s = size();
    Node *n = insertSingle(x.first);
    bool inserted = FALSE;
    if (s < size()) {
	inserted = TRUE;
	n->data = x.second;
    }
    return QPair<Iterator,bool>(n, inserted);
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<Key,T>::Iterator QMap<Key,T>::insert(const Key& key, const T& value)
{
    detach();
    Node *n = insertSingle(key);
    n->data = value;
    return n;
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE Q_TYPENAME QMap<Key,T>::Iterator QMap<Key,T>::insert(const Key& key, const T& value, bool overwrite)
{
    detach();
    size_type s = size();
    Node *n = insertSingle(key);
    if (overwrite || s < size())
	n->data = value;
    return n;
}


template<class Key, class T>
Q_OUTOFLINE_TEMPLATE T & QMap<Key,T>::operator[] (const Key& k)
{
    detach();
    Node* p = findNode(k);
    if (p != &d->header)
	return p->data;
    return insert(k, T()).data();
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE T QMap<Key,T>::value(const Key& k) const
{
    Node *n = findNode(k);
    if ( n == &d->header) {
	T t;
	qInit(t);
	return t;
    }
    return n->data;
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE T QMap<Key,T>::value(const Key& k, const T& defaultValue) const
{
    Node *n = findNode(k);
    return ((n == &d->header) ? defaultValue : n->data);
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key,T>::keys() const {
    QList<Key> r;
    for (ConstIterator i=begin(); i!=end(); ++i)
	r.append(i.key());
    return r;
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key,T>::values() const {
    cerr << "called values" << endl;
    QList<T> r;
    for (ConstIterator i=begin(); i!=end(); ++i)
	r.append(*i);
    return r;
}


#ifndef QT_NO_STL
template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key,T>::QMap(const std::map<Key,T>& m)
{
    d = QMapData::init(new QMapData);
    Q_TYPENAME std::map<Key,T>::const_iterator it = m.begin();
    for (; it != m.end(); ++it) {
	value_type p((*it).first, (*it).second);
	insert(p);
    }
}

template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QMap<Key,T>& QMap<Key,T>::operator= (const std::map<Key,T>& m)
{
    clear();
    Q_TYPENAME std::map<Key,T>::const_iterator it = m.begin();
    for (; it != m.end(); ++it) {
	value_type p((*it).first, (*it).second);
	insert(p);
    }
    return *this;
}
#endif

#ifndef QT_NO_DATASTREAM
template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream& operator>>(QDataStream& s, QMap<Key,T>& m) {
    m.clear();
    Q_UINT32 c;
    s >> c;
    for(Q_UINT32 i = 0; i < c; ++i) {
	Key k; T t;
	s >> k >> t;
	m.insert(k, t);
	if (s.atEnd())
	    break;
    }
    return s;
}


template<class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream& operator<<(QDataStream& s, const QMap<Key,T>& m) {
    s << (Q_UINT32)m.size();
    Q_TYPENAME QMap<Key,T>::ConstIterator it = m.begin();
    for(; it != m.end(); ++it)
	s << it.key() << it.data();
    return s;
}
#endif

Q_DECLARE_ASSOCIATIVE_ITERATOR(QMap)

#endif // QMAP_H

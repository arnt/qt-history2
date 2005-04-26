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

#ifndef QCACHE_H
#define QCACHE_H

#include "QtCore/qhash.h"

template <class Key, class T>
class QCache
{
    struct Node {
        inline Node() {}
        inline Node(const Key &key, T *data, int cost)
            : k(key), t(data), c(cost), p(0), n(0) {}
        Key k; T *t; int c; Node *p,*n;
    };
    Node *f, *l;
    QHash<Key, Node> hash;
    QHash<int, Node> serial;
    int mx, total;
    inline void unlink(Node &n) {
        if (n.p) n.p->n = n.n;
        if (n.n) n.n->p = n.p;
        if (l == &n) l = n.p;
        if (f == &n) f = n.n;
        total -= n.c;
        delete n.t;
        hash.remove(Key(n.k));
    }
    inline T *&relink(const Key &key) {
        Node &n = hash[key];
        if (f != &n) {
            if (n.p) n.p->n = n.n;
            if (n.n) n.n->p = n.p;
            if (l == &n) l = n.p;
            n.p = 0;
            n.n = f;
            f->p = &n;
            f = &n;
        }
        return n.t;
    }

    Q_DISABLE_COPY(QCache)

public:
    inline explicit QCache(int maxCost = 100);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT_CONSTRUCTOR QCache(int maxCost, int /* dummy */)
        : f(0), l(0), mx(maxCost), total(0) {}
#endif
    inline ~QCache() { clear(); }

    inline int maxCost() const { return mx; }
    void setMaxCost(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return hash.size(); }
    inline int count() const { return hash.size(); }
    inline bool isEmpty() const { return hash.isEmpty(); }
    inline QList<Key> keys() const { return hash.keys(); }

    void clear();

    bool insert(const Key &key, T *object, int cost = 1);
    T *object(const Key &key) const;
    inline bool contains(const Key &key) const { return hash.contains(key); }
    T *operator[](const Key &key) const;

    bool remove(const Key &key);
    T *take(const Key &key);

private:
    void trim(int m);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT T *find(const Key &key) const { return object(key); }
#endif

};

template <class Key, class T>
inline QCache<Key, T>::QCache(int amaxCost)
    : f(0), l(0), mx(amaxCost), total(0) {}

template <class Key, class T>
inline void QCache<Key,T>::clear()
{ while (f) { delete f->t; f = f->n; }
 hash.clear(); l = 0; total = 0; }

template <class Key, class T>
inline void QCache<Key,T>::setMaxCost(int m)
{ mx = m; trim(mx); }

template <class Key, class T>
inline T *QCache<Key,T>::object(const Key &key) const
{ if (!hash.contains(key)) return 0;
 return const_cast<QCache<Key,T>*>(this)->relink(key); }

template <class Key, class T>
inline T *QCache<Key,T>::operator[](const Key &key) const
{ return object(key); }

template <class Key, class T>
inline bool QCache<Key,T>::remove(const Key &key)
{ if (hash.contains(key)) { unlink(hash[key]); return true; } return false; }

template <class Key, class T>
inline T *QCache<Key,T>::take(const Key &key)
{ if (!hash.contains(key)) return 0;
 Node &n = hash[key]; T *t = n.t; n.t = 0; unlink(n); return t; }

template <class Key, class T>
bool QCache<Key,T>::insert(const Key &akey, T *aobject, int acost)
{
    remove(akey);
    if (acost > mx) {
        delete aobject;
        return false;
    }
    trim(mx - acost);
    Node sn(akey, aobject, acost);
    hash.insert(akey, sn);
    total += acost;
    Node *n = &hash[akey];
    if (f) f->p = n;
    n->n = f;
    f = n;
    if (!l) l = f;
    return true;
}

template <class Key, class T>
void QCache<Key,T>::trim(int m)
{
    Node *n = l;
    while (n && total > m) {
        Node *u = n;
        n = n->p;
        if (qIsDetached(*u->t))
            unlink(*u);
    }
}

#endif

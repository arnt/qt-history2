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

#include "qhash.h"

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
    inline QCache(int maxCost = 100)
        : f(0), l(0), mx(maxCost), total(0) {}
#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QCache(int maxCost, int /* dummy */)
        : f(0), l(0), mx(maxCost), total(0) {}
#endif
    inline ~QCache() { clear(); }

    inline int maxCost() const { return mx; }
    void setMaxCost(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return hash.size(); }
    inline int count() const { return hash.size(); }
    inline bool isEmpty() const { return hash.isEmpty(); }

    void clear();

    void insert(const Key &key, T *data, int cost = 1);
    T *find(const Key &key) const;

    inline bool contains(const Key &key) const { return hash.contains(key); }
    T *operator[](const Key &key) const;

    bool remove(const Key &key);
    T *take(const Key &key);
};

template <class Key, class T>
inline void QCache<Key,T>::clear()
{ while (f) { delete f->t; f = f->n; }
 hash.clear(); l = 0; total = 0; }

template <class Key, class T>
inline void QCache<Key,T>::setMaxCost(int m)
{ mx = m; while (l && total > mx) unlink(*l); }

template <class Key, class T>
inline T *QCache<Key,T>::find(const Key &key) const
{ if (!hash.contains(key)) return 0;
 return ((QCache<Key,T>*)this)->relink(key); }

template <class Key, class T>
inline T *QCache<Key,T>::operator[](const Key &key) const
{ return find(key); }

template <class Key, class T>
inline bool QCache<Key,T>::remove(const Key &key)
{ if (hash.contains(key)) { unlink(hash[key]); return true; } return false; }

template <class Key, class T>
inline T *QCache<Key,T>::take(const Key &key)
{ if (!hash.contains(key)) return 0;
 Node &n = hash[key]; T *t = n.t; n.t = 0; unlink(n); return t; }

template <class Key, class T>
void QCache<Key,T>::insert(const Key &key, T *data, int cost)
{
    remove(key);
    if (cost > mx) {
        delete data;
        return;
    }
    Node *n = l;
    while (n && total > mx - cost) {
        Node *u = n;
        n = n->p;
        if (qIsDetached(*u->t))
            unlink(*u);
    }
    Node sn(key, data, cost);
    hash.insert(key, sn);
    total += cost;
    n = &hash[key];
    if (f) f->p = n;
    n->n = f;
    f = n;
    if (!l) l = f;
}

#endif

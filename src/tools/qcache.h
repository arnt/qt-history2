#ifndef QCACHE_H
#define QCACHE_H

#ifndef QT_H
#include "qmap.h"
#endif // QT_H

template <class Key, class T>
class QCache
{
    struct Node {
	inline Node(){}
	inline Node(const Key &key, const T &data, int cost)
	    :k(key),t(data),c(cost),p(0),n(0){}
	Key k; T t; int c; Node *p,*n;
    };
    Node *f, *l;
    QMap<Key, Node> map;
    int max, total;
    inline void unlink(Node &n) {
	if (n.p) n.p->n = n.n;
	if (n.n) n.n->p = n.p;
	if (l == &n) l = n.p;
	if (f == &n) f = n.n;
	total -= n.c;
	qDelete(n.t);
	map.remove(Key(n.k));
    }
    inline T &relink(const Key &key) {
	Node &n = map[key];
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

 public:
    inline QCache(int maxCost = 100, int /*size*/ = 17)
	:f(0),l(0),max(maxCost),total(0) {}
    inline ~QCache() { clear(); }

    inline int maxCost() const { return max; }
    void setMaxCost(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return map.size(); }
    inline int count() const { return map.size(); }
    inline bool isEmpty() const { return map.isEmpty(); }
    void clear();

    void insert(const Key &key, const T &data, int cost = 1);
    bool find(const Key &key, T &data) const;

    inline bool contains(const Key &key) const { return map.contains(key); }
    T operator[] (const Key &key) const;

    bool remove(const Key &key);
    T take(const Key &key);
    T &ref(const Key &key);

    inline bool ensure_constructed()
    { if (!map.ensure_constructed()) { max = 100; return false; } return true; }
};

template <class Key, class T>
inline void QCache<Key,T>::clear()
{ if (QTypeInfo<T>::isPointer) while (f) { qDelete(f->t); f = f->n; }
 map.clear(); }

template <class Key, class T>
inline void QCache<Key,T>::setMaxCost(int m)
{ max = m; while (l && total > max) unlink(*l); }

template <class Key, class T>
inline bool QCache<Key,T>::find(const Key &key, T &data) const
{ if (!map.contains(key)) return false;
 data = ((QCache<Key,T>*)this)->relink(key); return true; }

template <class Key, class T>
inline T QCache<Key,T>::operator[](const Key &key) const
{ if (map.contains(key)) return  ((QCache<Key,T>*)this)->relink(key);
  T t; qInit(t); return t; }

template <class Key, class T>
inline T &QCache<Key,T>::ref(const Key &key)
{ Q_ASSERT(contains(key)) return relink(key); }

template <class Key, class T>
inline bool QCache<Key,T>::remove(const Key &key)
{ if (map.contains(key)) { unlink(map[key]); return true; } return false; }

template <class Key, class T>
inline T QCache<Key,T>::take(const Key &key)
{ if (!map.contains(key)) { T t; qInit(t); return t; }
 Node &n = map[key]; T t = n.t; qInit(n.t);  unlink(n);  return t; }

template <class Key, class T>
void QCache<Key,T>::insert(const Key &key, const T &data, int cost)
{
    remove(key);
    Node *n = l;
    while (n && total > max - cost) {
	Node *u = n;
	n = n->p;
	if (qIsDetached(u->t))
	    unlink(*u);
    }
    Node sn(key, data, cost);
    map.insert(key, sn);
    total += cost;
    n = &map[key];
    if (f) f->p = n;
    n->n = f;
    f = n;
    if (!l) l = f;
}

#endif

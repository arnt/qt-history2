#ifndef QITERATOR_H
#define QITERATOR_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

namespace std { struct bidirectional_iterator_tag; struct random_access_iterator_tag; }

#define Q_DECLARE_ITERATOR(C) \
\
template <class T> \
class C##Iterator \
{ \
    typedef typename C<T>::const_iterator const_iterator; \
    C<T> c; \
    const_iterator i; \
public: \
    inline C##Iterator(const C<T> &container) \
    :c(container), i(c.constBegin()) {} \
    inline void operator=(const C<T> &container) \
    { c = container.c; i = c.constBegin(); } \
    inline void toFront() { i = c.constBegin(); } \
    inline void toBack() { i = c.constEnd(); } \
    inline bool hasNext() const { return i != c.constEnd(); } \
    inline const T &next() { return *i++; } \
    inline const T &peekNext() const { return *i; } \
    inline bool hasPrev() const { return i != c.constBegin(); } \
    inline const T &prev() { return *--i; } \
    inline const T &peekPrev() const { const_iterator p = i; return *--p; } \
    inline bool findNext(const T &t) \
    { while (c && i != c->constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrev(const T &t) \
    { while (c && i!=c->constBegin()) if (*(--i) == t) return true; \
      return false;  } \
}; \
\
template <class T> \
class C##MutableIterator \
{ \
    typedef typename C<T>::iterator iterator; \
    C<T> *c; \
    iterator i, n; \
    inline bool item_exists() const { return n != c->constEnd(); } \
public: \
    inline C##MutableIterator(C<T> &container) \
    :c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline void toBack() { i = c->end(); n = i; } \
    inline bool hasNext() const { return c && c->constEnd() != i; } \
    inline T &next() { n = i++; return *n; } \
    inline T &peekNext() const { return *i; } \
    inline bool hasPrev() const { return c && c->constBegin() != i; } \
    inline T &prev() { n = --i; return *n; } \
    inline T &peekPrev() const { iterator p = i; return *--p; } \
    inline void remove() \
    { if (c->constEnd() != n) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) const { if (c->constEnd() != n) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline void insert(const T &t) { n = i = c->insert(i, t); ++i; } \
    inline bool findNext(const T &t) \
    { while (c && c->constEnd() != (n=i)) if (*i++ == t) return true; return false; } \
    inline bool findPrev(const T &t) \
    { while (c && c->constBegin() != i) if (*(n=--i) == t) return true; \
      n = c->end(); return false;  } \
};

#define Q_DECLARE_ASSOCIATIVE_ITERATOR(C) \
\
template <class Key, class T> \
class C##Iterator \
{ \
    typedef typename C<Key,T>::const_iterator const_iterator; \
    C<Key,T> *c; \
    const_iterator i, n; \
    inline bool item_exists() const { return n != c->constEnd(); } \
public: \
    inline C##Iterator(C<Key,T> &container) \
    : c(&container), i(c->constBegin()), n(c->constEnd()) {} \
    inline void operator=(C<Key,T> &container) \
    { c = &container; i = c->constBegin(); n = c->constEnd(); } \
    inline void toFront() { i = c->constBegin(); n = c->constEnd(); } \
    inline void toBack() { i = c->constEnd(); n = c->constEnd(); } \
    inline bool hasNext() const { return c && i != c->constEnd(); } \
    inline const_iterator next() { n = i++; return n; } \
    inline const_iterator peekNext() const { return i; } \
    inline bool hasPrev() const { return c && i != c->constBegin(); } \
    inline const_iterator prev() { n = --i; return n; } \
    inline const_iterator peekPrev() const { const_iterator p = i; return --p; } \
    inline void remove() \
    { if (n != c->constEnd()) { i = c->erase(n); n = c->constEnd(); } } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findNext(const T &t) \
    { while (c && (n=i) != c->constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrev(const T &t) \
    { while (c && i != c->constBegin()) if (*(n = --i) == t) return true; \
      n = c->constEnd(); return false; } \
    inline bool findNextKey(const Key &key) \
    {  \
	if (i == c->constEnd() || key < i.key()) { \
	    n = i = c->constEnd(); \
            return false; \
	} else if (i.key() < key) { \
	    i = c->find(key); \
            if ((n = i) != c->constEnd()) { \
		++i; \
                return true; \
            } \
            return false; \
        } else { \
	    n = i++; \
            return true; \
        } \
    } \
    inline bool findPrevKey(const Key &key) \
    { \
	if (i == c->constBegin() || (--i).key() < key) { \
	    n = c->constEnd(); \
            i = c->constBegin(); \
            return false; \
        } else if (key < i.key()) { \
	    n = i = c->find(key); \
            if (i == c->constEnd()) { \
		i = c->constBegin(); \
                return false; \
            } \
            while (++i != c->constEnd() && !(key < i.key())) \
		; \
	    n = --i; \
            return true; \
        } else { \
	    n = i; \
            return true; \
        } \
    } \
}; \
\
template <class Key, class T> \
class C##MutableIterator \
{ \
    typedef typename C<Key,T>::iterator iterator; \
    C<Key,T> *c; \
    iterator i, n; \
    inline bool item_exists() const { return n != c->constEnd(); } \
public: \
    inline C##MutableIterator(C<Key,T> &container)\
    : c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<Key,T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline void toBack() { i = c->end(); n = c->end(); } \
    inline bool hasNext() const { return c && i != c->constEnd(); } \
    inline iterator next() { n = i++; return n; } \
    inline iterator peekNext() const { return i; } \
    inline bool hasPrev() const { return c && i != c->constBegin(); } \
    inline iterator prev() const { n = --i; return n; } \
    inline iterator peekPrev() const { iterator p = i; return --p; } \
    inline void remove() \
    { if (n != c->constEnd()) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) { if (n != c->constEnd()) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findKey(const Key &key) \
    { if ((n = i = c->find(key)) != c->constEnd()) { ++i; return true; } return false; } \
    inline bool findNext(const T &t) \
    { while (c && (n=i) != c->constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrev(const T &t) \
    { while (c && i != c->constBegin()) if (*(n = --i) == t) return true; \
      n = c->end(); return false; } \
    inline bool findNextKey(const Key &key) \
    { return reinterpret_cast<C##Iterator<Key, T> *>(this)->findNextKey(key); } \
    inline bool findPrevKey(const Key &key) \
    { return reinterpret_cast<C##Iterator<Key, T> *>(this)->findPrevKey(key); } \
};

#endif // QITERATOR_H

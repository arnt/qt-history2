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
    typedef typename C<T>::ConstIterator ConstIterator; \
    C<T> c; \
    ConstIterator i; \
public: \
    inline C##Iterator(const C<T> &container) \
    :c(container), i(c.constBegin()) {} \
    inline void operator=(const C<T> &container) \
    { c = container.c; i = c.constBegin(); } \
    inline void toFront() { i = c.constBegin(); } \
    inline void toBack() { i = c.constEnd(); } \
    inline bool hasNext() const { return i != c.end(); } \
    inline const T &next() { return *i++; } \
    inline const T &peekNext() const { return *i; } \
    inline bool hasPrev() const { return i != c.constBegin(); } \
    inline const T &prev() { return *--i; } \
    inline const T &peekPrev() const { ConstIterator p = i; return *--p; } \
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
    typedef typename C<T>::Iterator Iterator; \
    C<T> *c; \
    Iterator i, n; \
    inline bool item_exists() const { return n!=c->end(); } \
public: \
    inline C##MutableIterator(C<T> &container) \
    :c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline void toBack() { i = c->end(); n = i; } \
    inline bool hasNext() const { return c && i != c->end(); } \
    inline T &next() { n = i++; return *n; } \
    inline T &peekNext()  { return *i; } \
    inline bool hasPrev() const { return c && i != c->begin(); } \
    inline T &prev() { n = --i; return *n; } \
    inline T &peekPrev() { Iterator p = i; return *--p; } \
    inline void remove() \
    { if (n != c->end()) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) { if (n!= c->end()) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists); return *n; } \
    inline void insert(const T &t) { n = i = c->insert(i, t); ++i; } \
    inline bool findNext(const T &t) \
    { while (c && (n=i) != c->end()) if (*i++ == t) return true; return false; } \
    inline bool findPrev(const T &t) \
    { while (c && i != c->begin()) if (*(n=--i) == t) return true; \
      n = c->end(); return false;  } \
};


#define Q_DECLARE_ASSOCIATIVE_ITERATOR(C) \
\
template <class Key, class T> \
class C##Iterator \
{ \
    typedef typename C<Key,T>::Iterator ConstIterator; \
    C<Key,T> *c; \
    ConstIterator i, n; \
    inline bool item_exists() const { return n != c->end(); } \
public: \
    inline C##Iterator(C<Key,T> &container) \
    :c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<Key,T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline bool hasNext() const { return c && i != c->end(); } \
    inline ConstIterator next() { n=i++; return n; } \
    inline ConstIterator peekNext()  { return i; } \
    inline void remove() \
    { if (n != c->end()) { i = c->erase(n); n = c->end(); } } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findNext(const T &t) \
    { while (c && (n=i) != c->end()) if (*i++ == t) return true; return false; } \
}; \
\
template <class Key, class T> \
class C##MutableIterator \
{ \
    typedef typename C<Key,T>::Iterator Iterator; \
    C<Key,T> *c; \
    Iterator i, n; \
    inline bool item_exists() const { return n!=c->end(); } \
public: \
    inline C##MutableIterator(C<Key,T> &container)\
    :c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<Key,T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline bool hasNext() const { return c && i != c->end(); } \
    inline Iterator next() { n = i++; return n; } \
    inline Iterator peekNext()  { return i; } \
    inline void remove() \
    { if (n != c->end()) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) { if (n != c->end()) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findNext(const T &t) \
    { while (c && (n=i) != c->end()) if (*i++ == t) return true; return false; } \
};

#endif // QITERATOR_H

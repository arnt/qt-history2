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

#ifndef QITERATOR_H
#define QITERATOR_H

#include "qglobal.h"

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
    : c(container), i(c.constBegin()) {} \
    inline void operator=(const C<T> &container) \
    { c = container.c; i = c.constBegin(); } \
    inline void toFront() { i = c.constBegin(); } \
    inline void toBack() { i = c.constEnd(); } \
    inline bool hasNext() const { return i != c.constEnd(); } \
    inline const T &next() { return *i++; } \
    inline const T &peekNext() const { return *i; } \
    inline bool hasPrevious() const { return i != c.constBegin(); } \
    inline const T &previous() { return *--i; } \
    inline const T &peekPrevious() const { const_iterator p = i; return *--p; } \
    inline bool findNext(const T &t) \
    { while (i != c->constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrevious(const T &t) \
    { while (i != c->constBegin()) if (*(--i) == t) return true; \
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
    : c(&container), i(c->begin()), n(c->end()) {} \
    inline void operator=(C<T> &container) \
    { c = &container; i = c->begin(); n = c->end(); } \
    inline void toFront() { i = c->begin(); n = c->end(); } \
    inline void toBack() { i = c->end(); n = i; } \
    inline bool hasNext() const { return c->constEnd() != i; } \
    inline T &next() { n = i++; return *n; } \
    inline T &peekNext() const { return *i; } \
    inline bool hasPrevious() const { return c->constBegin() != i; } \
    inline T &previous() { n = --i; return *n; } \
    inline T &peekPrevious() const { iterator p = i; return *--p; } \
    inline void remove() \
    { if (c->constEnd() != n) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) const { if (c->constEnd() != n) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline void insert(const T &t) { n = i = c->insert(i, t); ++i; } \
    inline bool findNext(const T &t) \
    { while (c->constEnd() != (n = i)) if (*i++ == t) return true; return false; } \
    inline bool findPrevious(const T &t) \
    { while (c->constBegin() != i) if (*(n = --i) == t) return true; \
      n = c->end(); return false;  } \
};

#define Q_DECLARE_ASSOCIATIVE_ITERATOR(C) \
\
template <class Key, class T> \
class C##Iterator \
{ \
    typedef typename C<Key,T>::const_iterator const_iterator; \
    typedef const_iterator Item; \
    C<Key,T> c; \
    const_iterator i, n; \
    inline bool item_exists() const { return n != c.constEnd(); } \
public: \
    inline C##Iterator(const C<Key,T> &container) \
    : c(container), i(c.constBegin()), n(c.constEnd()) {} \
    inline void operator=(const C<Key,T> &container) \
    { c = container; i = c.constBegin(); n = c.constEnd(); } \
    inline void toFront() { i = c.constBegin(); n = c.constEnd(); } \
    inline void toBack() { i = c.constEnd(); n = c.constEnd(); } \
    inline bool hasNext() const { return i != c.constEnd(); } \
    inline Item next() { n = i++; return n; } \
    inline Item peekNext() const { return i; } \
    inline bool hasPrevious() const { return i != c.constBegin(); } \
    inline Item previous() { n = --i; return n; } \
    inline Item peekPrevious() const { const_iterator p = i; return --p; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findNext(const T &t) \
    { while ((n = i) != c.constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrevious(const T &t) \
    { while (i != c.constBegin()) if (*(n = --i) == t) return true; \
      n = c.constEnd(); return false; } \
    inline bool findNextKey(const Key &key) \
    {  \
        if (i == c.constBegin()) { \
            i = const_cast<const C<Key,T> &>(c).find(key); \
            if ((n = i) != c.constEnd()) { \
                ++i; \
                return true; \
            } \
            return false; \
        } else if (i == c.constEnd() || !C<Key, T>::sameKey(i.key(), key)) { \
            n = i = c.constEnd(); \
            return false; \
        } else { \
            n = i++; \
            return true; \
        } \
    } \
    inline bool findPreviousKey(const Key &key) \
    { \
        if (i == c.constEnd()) { \
            n = i = const_cast<const C<Key,T> &>(c).find(key); \
            if (i == c.constEnd()) { \
                i = c.constBegin(); \
                return false; \
            } \
            while (++i != c.constEnd() && C<Key, T>::sameKey(i.key(), key)) \
                ; \
            n = --i; \
            return true; \
        } else if (i == c.constBegin() || !C<Key, T>::sameKey((--i).key(), key)) { \
            n = c.constEnd(); \
            i = c.constBegin(); \
            return false; \
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
    typedef iterator Item; \
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
    inline bool hasNext() const { return i != c->constEnd(); } \
    inline Item next() { n = i++; return n; } \
    inline Item peekNext() const { return i; } \
    inline bool hasPrevious() const { return i != c->constBegin(); } \
    inline Item previous() const { n = --i; return n; } \
    inline Item peekPrevious() const { iterator p = i; return --p; } \
    inline void remove() \
    { if (n != c->constEnd()) { i = c->erase(n); n = c->end(); } } \
    inline void setValue(const T &t) { if (n != c->constEnd()) *n = t; } \
    inline const T &value() const { Q_ASSERT(item_exists()); return *n; } \
    inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); } \
    inline bool findNext(const T &t) \
    { while ((n = i) != c->constEnd()) if (*i++ == t) return true; return false; } \
    inline bool findPrevious(const T &t) \
    { while (i != c->constBegin()) if (*(n = --i) == t) return true; \
      n = c->end(); return false; } \
    inline bool findNextKey(const Key &key) \
    { return reinterpret_cast<C##Iterator<Key, T> *>(this)->findNextKey(key); } \
    inline bool findPreviousKey(const Key &key) \
    { return reinterpret_cast<C##Iterator<Key, T> *>(this)->findPreviousKey(key); } \
};

#endif // QITERATOR_H

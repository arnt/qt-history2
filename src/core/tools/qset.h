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

#ifndef QSET_H
#define QSET_H

#include "QtCore/qhash.h"

template <class T>
class QSet
{
    typedef QHash<T, QHashDummyValue> Hash;

public:
    inline QSet() {}
    inline QSet(const QSet<T> &other) : q_hash(other.q_hash) {}

    inline QSet<T> &operator=(const QSet<T> &other)
        { q_hash = other.q_hash; return *this; }

    inline bool operator==(const QSet<T> &other) const
        { return q_hash == other.q_hash; }
    inline bool operator!=(const QSet<T> &other) const
        { return q_hash != other.q_hash; }

    inline int size() const { return q_hash.size(); }

    inline bool isEmpty() const { return q_hash.isEmpty(); }

    inline int capacity() const { return q_hash.capacity(); }
    inline void reserve(int size) { q_hash.reserve(size); }
    inline void squeeze() { q_hash.squeeze(); }

    inline void detach() { q_hash.detach(); }
    inline bool isDetached() const { return q_hash.isDetached(); }

    inline void clear() { q_hash.clear(); }

    inline bool remove(const T &value) { return q_hash.remove(value) != 0; }

    bool contains(const T &value) const { return q_hash.contains(value); }

/*
    fromList
    toList
*/

    class const_iterator
    {
        typename Hash::const_iterator i;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline const_iterator() {}
        inline const_iterator(typename Hash::const_iterator i) : i(i) {}
        inline const_iterator(const const_iterator &o): i(o.i){}
        inline const T &operator*() const { return i.key(); }
        inline const T *operator->() const { return &i.key(); }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }
        inline const_iterator operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const_iterator r = *this; ++i; return r; }
        inline const_iterator operator--() { --i; return *this; }
        inline const_iterator operator--(int) { const_iterator r = *this; --i; return r; }
        inline const_iterator operator+(int j) const { return i + j; }
        inline const_iterator operator-(int j) const {return i - j; }
        inline const_iterator &operator+=(int j) { i += j; return *this; }
        inline const_iterator &operator-=(int j) { i -= j; return *this; }
    };

    // STL style
    inline const_iterator begin() const { return q_hash.begin(); }
    inline const_iterator constBegin() const { return q_hash.constBegin(); }
    inline const_iterator end() const { return q_hash.end(); }
    inline const_iterator constEnd() const { return q_hash.constEnd(); }

    // more Qt
    typedef const_iterator ConstIterator;
    inline int count() const { return q_hash.count(); }
    inline const_iterator insert(const T &value)
        { return static_cast<typename Hash::const_iterator>(q_hash.insert(value,
                                                                          QHashDummyValue())); }
    QSet<T> &merge(const QSet<T> &other);
    QSet<T> &intersect(const QSet<T> &other);
    QSet<T> &subtract(const QSet<T> &other);

    // STL compatibility
    inline bool empty() const { return isEmpty(); }

    // comfort
    inline QSet<T> &operator<<(const T &value) { insert(value); return *this; }
    inline QSet<T> &operator|=(const QSet<T> &other) { merge(other); return *this; }
    inline QSet<T> &operator&=(const QSet<T> &other) { intersect(other); return *this; }
    inline QSet<T> &operator+=(const QSet<T> &other) { merge(other); return *this; }
    inline QSet<T> &operator-=(const QSet<T> &other) { subtract(other); return *this; }
    inline QSet<T> operator|(const QSet<T> &other)
        { QSet<T> result = *this; result |= other; return result; }
    inline QSet<T> operator&(const QSet<T> &other)
        { QSet<T> result = *this; result &= other; return result; }
    inline QSet<T> operator+(const QSet<T> &other)
        { QSet<T> result = *this; result += other; return result; }
    inline QSet<T> operator-(const QSet<T> &other)
        { QSet<T> result = *this; result -= other; return result; }

private:
    Hash q_hash;
};

template <class T>
Q_INLINE_TEMPLATE QSet<T> &QSet<T>::merge(const QSet<T> &other)
{
    QSet<T> copy(other);
    const_iterator i = copy.constEnd();
    while (i != copy.constBegin()) {
        --i;
        insert(*i);
    }
    return *this;
}

template <class T>
Q_INLINE_TEMPLATE QSet<T> &QSet<T>::intersect(const QSet<T> &other)
{
    QSet<T> copy1(*this);
    QSet<T> copy2(other);
    const_iterator i = copy1.constEnd();
    while (i != copy1.constBegin()) {
        --i;
        if (!copy2.contains(*i))
            remove(*i);
    }
    return *this;
}

template <class T>
Q_INLINE_TEMPLATE QSet<T> &QSet<T>::subtract(const QSet<T> &other)
{
    QSet<T> copy1(*this);
    QSet<T> copy2(other);
    const_iterator i = copy1.constEnd();
    while (i != copy1.constBegin()) {
        --i;
        if (copy2.contains(*i))
            remove(*i);
    }
    return *this;
}

Q_DECLARE_SEQUENTIAL_ITERATOR(Set)

#endif

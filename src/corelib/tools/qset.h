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

QT_MODULE(Core)

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
    inline void reserve(int size);
    inline void squeeze() { q_hash.squeeze(); }

    inline void detach() { q_hash.detach(); }
    inline bool isDetached() const { return q_hash.isDetached(); }

    inline void clear() { q_hash.clear(); }

    inline bool remove(const T &value) { return q_hash.remove(value) != 0; }

    inline bool contains(const T &value) const { return q_hash.contains(value); }

    class const_iterator
    {
        typedef QHash<T, QHashDummyValue> Hash;
        typename Hash::const_iterator i;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline const_iterator() {}
        inline const_iterator(typename Hash::const_iterator o) : i(o) {}
        inline const_iterator(const const_iterator &o) : i(o.i) {}
        inline const_iterator &operator=(const const_iterator &o) { i = o.i; return *this; }
        inline const T &operator*() const { return i.key(); }
        inline const T *operator->() const { return &i.key(); }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }
        inline const_iterator &operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const_iterator r = *this; ++i; return r; }
        inline const_iterator &operator--() { --i; return *this; }
        inline const_iterator operator--(int) { const_iterator r = *this; --i; return r; }
        inline const_iterator operator+(int j) const { return i + j; }
        inline const_iterator operator-(int j) const { return i - j; }
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
    QSet<T> &unite(const QSet<T> &other);
    QSet<T> &intersect(const QSet<T> &other);
    QSet<T> &subtract(const QSet<T> &other);

    // STL compatibility
    inline bool empty() const { return isEmpty(); }

    // comfort
    inline QSet<T> &operator<<(const T &value) { insert(value); return *this; }
    inline QSet<T> &operator|=(const QSet<T> &other) { unite(other); return *this; }
    inline QSet<T> &operator|=(const T &value) { insert(value); return *this; }
    inline QSet<T> &operator&=(const QSet<T> &other) { intersect(other); return *this; }
    inline QSet<T> &operator&=(const T &value)
        { QSet<T> result; if (contains(value)) result.insert(value); return (*this = result); }
    inline QSet<T> &operator+=(const QSet<T> &other) { unite(other); return *this; }
    inline QSet<T> &operator+=(const T &value) { insert(value); return *this; }
    inline QSet<T> &operator-=(const QSet<T> &other) { subtract(other); return *this; }
    inline QSet<T> &operator-=(const T &value) { subtract(value); return *this; }
    inline QSet<T> operator|(const QSet<T> &other)
        { QSet<T> result = *this; result |= other; return result; }
    inline QSet<T> operator&(const QSet<T> &other)
        { QSet<T> result = *this; result &= other; return result; }
    inline QSet<T> operator+(const QSet<T> &other)
        { QSet<T> result = *this; result += other; return result; }
    inline QSet<T> operator-(const QSet<T> &other)
        { QSet<T> result = *this; result -= other; return result; }

    QList<T> toList() const;
    inline QList<T> values() const { return toList(); }

    static QSet<T> fromList(const QList<T> &list);

private:
    Hash q_hash;
};

template <class T>
Q_INLINE_TEMPLATE void QSet<T>::reserve(int asize) { q_hash.reserve(asize); }

template <class T>
Q_INLINE_TEMPLATE QSet<T> &QSet<T>::unite(const QSet<T> &other)
{
    QSet<T> copy(other);
    typename QSet<T>::const_iterator i = copy.constEnd();
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
    typename QSet<T>::const_iterator i = copy1.constEnd();
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
    typename QSet<T>::const_iterator i = copy1.constEnd();
    while (i != copy1.constBegin()) {
        --i;
        if (copy2.contains(*i))
            remove(*i);
    }
    return *this;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QList<T> QSet<T>::toList() const
{
    QList<T> result;
    typename QSet<T>::const_iterator i = constBegin();
    while (i != constEnd()) {
        result.append(*i);
        ++i;
    }
    return result;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QSet<T> QList<T>::toSet() const
{
    QSet<T> result;
    result.reserve(size());
    for (int i = 0; i < size(); ++i)
        result.insert(at(i));
    return result;
}

template <typename T>
QSet<T> QSet<T>::fromList(const QList<T> &list)
{
    return list.toSet();
}

template <typename T>
QList<T> QList<T>::fromSet(const QSet<T> &set)
{
    return set.toList();
}

Q_DECLARE_SEQUENTIAL_ITERATOR(Set)

#endif

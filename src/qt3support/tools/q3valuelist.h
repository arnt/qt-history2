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

#ifndef Q3VALUELIST_H
#define Q3VALUELIST_H

#include "QtCore/qalgorithms.h"
#include "QtCore/qdatastream.h"
#include "QtCore/qlinkedlist.h"
#include "QtCore/qlist.h"

#ifndef QT_NO_STL
#include <iterator>
#include <list>
#endif

template <typename T>
class Q3ValueListIterator : public QLinkedList<T>::iterator
{
public:
    inline Q3ValueListIterator() :
        QLinkedList<T>::iterator() {}
    inline Q3ValueListIterator(const Q3ValueListIterator &o) :
        QLinkedList<T>::iterator(o) {}
    inline Q3ValueListIterator(const typename QLinkedList<T>::iterator &o) :
        QLinkedList<T>::iterator(o) {}
};

template <typename T>
class Q3ValueListConstIterator : public QLinkedList<T>::const_iterator
{
public:
    inline Q3ValueListConstIterator() {}
    inline Q3ValueListConstIterator(const Q3ValueListConstIterator &o) :
        QLinkedList<T>::const_iterator(o) {}
    inline Q3ValueListConstIterator(const typename QLinkedList<T>::const_iterator &o) :
        QLinkedList<T>::const_iterator(o) {}
    inline Q3ValueListConstIterator(const typename QLinkedList<T>::iterator &o) :
        QLinkedList<T>::const_iterator(o) {}
};

template <typename T>
class Q3ValueList : public QLinkedList<T>
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
#ifndef QT_NO_STL
    typedef ptrdiff_t  difference_type;
#else
    typedef int difference_type;
#endif

    typedef Q3ValueListIterator<T> Iterator;
    typedef Q3ValueListConstIterator<T> ConstIterator;
    typedef Q3ValueListIterator<T> iterator;
    typedef Q3ValueListConstIterator<T> const_iterator;
    typedef typename QLinkedList<T>::size_type size_type;

    /**
     * API
     */
    Q3ValueList() {}
    Q3ValueList(const Q3ValueList<T>& l) : QLinkedList<T>(l) {}
    Q3ValueList(const QLinkedList<T>& l) : QLinkedList<T>(l) {}
    Q3ValueList(const QList<T>& l)
    {
        for (int i = 0; i < l.size(); ++i) append(l.at(i));
    }
#ifndef QT_NO_STL
    Q3ValueList(const std::list<T>& l)
    {
        qCopy(l.begin(), l.end(), std::back_inserter(*this));
    }
#endif
    ~Q3ValueList() {}

    Q3ValueList<T>& operator= (const Q3ValueList<T>& l)
    {
        QLinkedList<T>::operator=(l);
        return *this;
    }
    Q3ValueList<T>& operator= (const QList<T>& l)
    {
        this->clear();
        for (int i = 0; i < l.size(); ++i) append(l.at(i));
        return *this;
    }
#ifndef QT_NO_STL
    Q3ValueList<T>& operator= (const std::list<T>& l)
    {
        this->detach();
        qCopy(l.begin(), l.end(), std::back_inserter(*this));
        return *this;
    }
    bool operator== (const std::list<T>& l) const
    {
        if (this->size() != l.size())
            return false;
        typename Q3ValueList<T>::const_iterator it2 = this->begin();
#if !defined(Q_CC_MIPS)
        typename
#endif
            std::list<T>::const_iterator it = l.begin();
        for (; it2 != this->end(); ++it2, ++it)
        if (!((*it2) == (*it)))
            return false;
        return true;
    }
#endif
    bool operator== (const Q3ValueList<T>& l) const { return QLinkedList<T>::operator==(l); }
    bool operator!= (const Q3ValueList<T>& l) const { return QLinkedList<T>::operator!=(l); }

    operator QList<T>() const {
        QList<T> list;
        for (typename Q3ValueList<T>::const_iterator it = QLinkedList<T>::constBegin();
             it != QLinkedList<T>::constEnd(); ++it)
            list.append(*it);
        return list;
    }

    inline Q3ValueList<T>& operator<< (const T& x) { append(x); return *this; }

    void insert(typename Q3ValueList<T>::Iterator pos,
                 typename Q3ValueList<T>::size_type n,
                 const T& x);

    typename Q3ValueList<T>::Iterator insert(typename Q3ValueList<T>::Iterator pos,
                                             const T& x)
        { return QLinkedList<T>::insert(pos, x); }
    typename Q3ValueList<T>::Iterator remove(typename Q3ValueList<T>::Iterator pos)
        { return QLinkedList<T>::erase(pos); }
    int remove(const T &value)
        { return QLinkedList<T>::removeAll(value); }

    inline Q3ValueList<T> operator+ (const Q3ValueList<T>& l) const
        { return static_cast<Q3ValueList<T> >(QLinkedList<T>::operator+(l)); }
    inline Q3ValueList<T>& operator+= (const Q3ValueList<T>& l)
        { QLinkedList<T>::operator+=(l); return *this; }

    typename Q3ValueList<T>::Iterator fromLast()
        { return (this->isEmpty() ? this->end() : --this->end()); }
    typename Q3ValueList<T>::ConstIterator fromLast() const
        { return (this->isEmpty() ? this->end() : --this->end()); }

    typename Q3ValueList<T>::Iterator append(const T& x)
        { QLinkedList<T>::append(x); return this->begin(); }
    typename Q3ValueList<T>::Iterator prepend(const T& x)
        { QLinkedList<T>::prepend(x); return --this->end(); }

    typename Q3ValueList<T>::Iterator at(typename Q3ValueList<T>::size_type i)
        { Q_ASSERT(i < this->size()); this->detach(); return this->begin()+i; }
    typename Q3ValueList<T>::ConstIterator at(typename Q3ValueList<T>::size_type i) const
        { Q_ASSERT(i < this->size()); return this->begin()+i; }
    typename Q3ValueList<T>::size_type contains(const T& x) const
        { return QLinkedList<T>::count(x); }

    Q3ValueList<T>& operator+= (const T& x) { append(x); return *this; }

    T& operator[] (typename Q3ValueList<T>::size_type i) { return *at(i); }
    const T& operator[] (typename Q3ValueList<T>::size_type i) const { return *at(i); }

};

template <typename T>
Q_OUTOFLINE_TEMPLATE void Q3ValueList<T>::insert(typename Q3ValueList<T>::Iterator pos,
                           typename Q3ValueList<T>::size_type n, const T& x)
{
    for (; n > 0; --n)
        this->insert(pos, x);
}

#ifndef QT_NO_DATASTREAM
template <typename T>
Q_OUTOFLINE_TEMPLATE QDataStream& operator>>(QDataStream& s, Q3ValueList<T>& l)
{
    return operator>>(s, static_cast<QLinkedList<T> &>(l));
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QDataStream& operator<<(QDataStream& s, const Q3ValueList<T>& l)
{
    return operator<<(s, static_cast<const QLinkedList<T> &>(l));
}
#endif // QT_NO_DATASTREAM

#endif // Q3VALUELIST_H

/****************************************************************************
**
** Definition of QValueList class.
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

#ifndef QVALUELIST_H
#define QVALUELIST_H

#ifndef QT_H
#include "qtl.h"
#include "qdatastream.h"
#include "qlist.h"
#endif // QT_H

#ifndef QT_NO_STL
#include <iterator>
#include <list>
#endif

#ifndef QT_NO_COMPAT
template <typename T> class QValueListIterator : public QList<T>::Iterator
{
public:
    inline QValueListIterator<T>() :
	QList<T>::Iterator() {}
    inline QValueListIterator<T>(const QValueListIterator &o) :
	QList<T>::Iterator(o) {}
    inline QValueListIterator<T>(const typename QList<T>::Iterator &o) :
	QList<T>::Iterator(o) {}
};

template <typename T> class QValueListConstIterator : public QList<T>::ConstIterator
{
public:
    inline QValueListConstIterator<T>() :
	QList<T>::ConstIterator() {}
    inline QValueListConstIterator<T>(const QValueListConstIterator &o) :
	QList<T>::ConstIterator(o) {}
    inline QValueListConstIterator<T>(const typename QList<T>::ConstIterator &o) :
	QList<T>::ConstIterator(o) {}
    inline QValueListConstIterator<T>(const typename QList<T>::Iterator &o) :
	QList<T>::ConstIterator(o) {}
};
#endif

template <typename T>
class QValueList : public QList<T>
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

    /**
     * API
     */
    QValueList() {}
    QValueList(const QValueList<T>& l) : QList<T>(l) {}
    QValueList(const QList<T>& l) : QList<T>(l) {}
#ifndef QT_NO_STL
    QValueList(const std::list<T>& l)
    {
	qCopy(l.begin(), l.end(), std::back_inserter(*this));
    }
#endif
    ~QValueList() {}

    QValueList<T>& operator= (const QValueList<T>& l)
    {
	QList<T>::operator=(l);
	return *this;
    }
#ifndef QT_NO_STL
    QValueList<T>& operator= (const std::list<T>& l)
    {
	detach();
	qCopy(l.begin(), l.end(), std::back_inserter(*this));
	return *this;
    }
    bool operator== (const std::list<T>& l) const
    {
	if (size() != l.size())
	    return FALSE;
	typename QValueList<T>::const_iterator it2 = begin();
#if !defined(Q_CC_MIPS)
	typename
#endif
	    std::list<T>::const_iterator it = l.begin();
	for (; it2 != end(); ++it2, ++it)
	if (!((*it2) == (*it)))
	    return FALSE;
	return TRUE;
    }
#endif
    bool operator== (const QValueList<T>& l) const { return QList<T>::operator==(l); }
    bool operator!= (const QValueList<T>& l) const { return QList<T>::operator!=(l); }

    inline QValueList<T>& operator<< (const T& x) { append(x); return *this; }

    void insert(typename QValueList<T>::Iterator pos,
		 typename QValueList<T>::size_type n,
		 const T& x);

    typename QValueList<T>::Iterator insert(typename QValueList<T>::Iterator pos,
					     const T& x)
        { return QList<T>::insert(pos, x); }

    inline QValueList<T> operator+ (const QValueList<T>& l) const
	{ return static_cast<QValueList<T> >(QList<T>::operator+(l)); }
    inline QValueList<T>& operator+= (const QValueList<T>& l)
        { QList<T>::operator+=(l); return *this; }

    typename QValueList<T>::Iterator fromLast()
        { return (isEmpty() ? end() : --end()); }
    typename QValueList<T>::ConstIterator fromLast() const
        { return (isEmpty() ? end() : --end()); }

    typename QValueList<T>::Iterator append(const T& x)
        { QList<T>::append(x); return begin(); }
    typename QValueList<T>::Iterator prepend(const T& x)
        { QList<T>::prepend(x); return --end(); }

    typename QValueList<T>::Iterator at(typename QValueList<T>::size_type i )
        { Q_ASSERT(i < size()); detach(); return begin()+i; }
    typename QValueList<T>::ConstIterator at(typename QValueList<T>::size_type i) const
        { Q_ASSERT(i < size()); return begin()+i; }
    typename QValueList<T>::size_type contains(const T& x) const
        { return QList<T>::count(x); }

    QValueList<T>& operator+= (const T& x) { append(x); return *this; }
};

template <typename T>
void QValueList<T>::insert(typename QValueList<T>::Iterator pos,
			   typename QValueList<T>::size_type n, const T& x)
{
    for (; n > 0; --n)
	insert(pos, x);
}

#ifndef QT_NO_DATASTREAM
template <typename T>
QDataStream& operator>>(QDataStream& s, QValueList<T>& l)
{
    return operator>>(s, (QList<T>&)l);
}

template <typename T>
QDataStream& operator<<(QDataStream& s, const QValueList<T>& l)
{
    return operator<<(s, (QList<T>&)l);
}
#endif // QT_NO_DATASTREAM

#define Q_DEFINED_QVALUELIST
#define Q_DEFINED_QVALUELIST
#include "qwinexport.h"
#endif // QVALUELIST_H

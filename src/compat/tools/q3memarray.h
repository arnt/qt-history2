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

#ifndef Q3MEMARRAY_H
#define Q3MEMARRAY_H

#include "q3garray.h"
#include "qvector.h"

template<class type>
class Q3MemArray : public Q3GArray
{
public:
    typedef type* Iterator;
    typedef const type* ConstIterator;
    typedef type ValueType;

protected:
    Q3MemArray(int, int) : Q3GArray(0, 0) {}

public:
    Q3MemArray() {}
    Q3MemArray(int size) : Q3GArray(size*sizeof(type)) {}
    Q3MemArray(const Q3MemArray<type> &a) : Q3GArray(a) {}
    Q3MemArray(const QVector<type> &vector);
   ~Q3MemArray() {}
    Q3MemArray<type> &operator=(const Q3MemArray<type> &a)
				{ return (Q3MemArray<type>&)Q3GArray::assign(a); }
    type *data()    const	{ return (type *)Q3GArray::data(); }
    uint  nrefs()   const	{ return Q3GArray::nrefs(); }
    uint  size()    const	{ return Q3GArray::size()/sizeof(type); }
    uint  count()   const	{ return size(); }
    bool  isEmpty() const	{ return Q3GArray::size() == 0; }
    bool  isNull()  const	{ return Q3GArray::data() == 0; }
    bool  resize(uint size)	{ return Q3GArray::resize(size*sizeof(type)); }
    bool  resize(uint size, Optimization optim) { return Q3GArray::resize(size*sizeof(type), optim); }
    bool  truncate(uint pos)	{ return Q3GArray::resize(pos*sizeof(type)); }
    bool  fill(const type &d, int size = -1)
	{ return Q3GArray::fill((char*)&d,size,sizeof(type)); }
    void  detach()		{ Q3GArray::detach(); }
    Q3MemArray<type>   copy() const
	{ Q3MemArray<type> tmp; return tmp.duplicate(*this); }
    Q3MemArray<type>& assign(const Q3MemArray<type>& a)
	{ return (Q3MemArray<type>&)Q3GArray::assign(a); }
    Q3MemArray<type>& assign(const type *a, uint n)
	{ return (Q3MemArray<type>&)Q3GArray::assign((char*)a,n*sizeof(type)); }
    Q3MemArray<type>& duplicate(const Q3MemArray<type>& a)
	{ return (Q3MemArray<type>&)Q3GArray::duplicate(a); }
    Q3MemArray<type>& duplicate(const type *a, uint n)
	{ return (Q3MemArray<type>&)Q3GArray::duplicate((char*)a,n*sizeof(type)); }
    Q3MemArray<type>& setRawData(const type *a, uint n)
	{ return (Q3MemArray<type>&)Q3GArray::setRawData((char*)a,
						     n*sizeof(type)); }
    void resetRawData(const type *a, uint n)
	{ Q3GArray::resetRawData((char*)a,n*sizeof(type)); }
    int	 find(const type &d, uint i=0) const
	{ return Q3GArray::find((char*)&d,i,sizeof(type)); }
    int	 contains(const type &d) const
	{ return Q3GArray::contains((char*)&d,sizeof(type)); }
    void sort() { Q3GArray::sort(sizeof(type)); }
    int  bsearch(const type &d) const
	{ return Q3GArray::bsearch((const char*)&d,sizeof(type)); }
    // ### Qt 4.0: maybe provide uint overload as work-around for MSVC bug
    type& operator[](int i) const
	{ return (type &)(*(type *)Q3GArray::at(i*sizeof(type))); }
    type& at(uint i) const
	{ return (type &)(*(type *)Q3GArray::at(i*sizeof(type))); }
	 operator const type*() const { return (const type *)Q3GArray::data(); }
    bool operator==(const Q3MemArray<type> &a) const { return isEqual(a); }
    bool operator!=(const Q3MemArray<type> &a) const { return !isEqual(a); }
    Iterator begin() { return data(); }
    Iterator end() { return data() + size(); }
    ConstIterator begin() const { return data(); }
    ConstIterator end() const { return data() + size(); }

    operator QVector<type>() const;
};

template<class type>
Q_OUTOFLINE_TEMPLATE Q3MemArray<type>::Q3MemArray(const QVector<type> &vector)
    : Q3GArray(vector.size()*sizeof(type))
{
    for (int i = 0; i < vector.size(); ++i)
        at(i) = vector.at(i);
}

template<class type>
Q_OUTOFLINE_TEMPLATE Q3MemArray<type>::operator QVector<type>() const
{
    QVector<type> vector;
    for (int i = 0; i < size(); ++i)
        vector.append(at(i));
    return vector;
}

#endif // Q3MEMARRAY_H

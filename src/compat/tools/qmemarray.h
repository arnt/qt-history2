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

#ifndef QMEMARRAY_H
#define QMEMARRAY_H

#ifndef QT_H
#include "qgarray.h"
#endif // QT_H


template<class type>
class QMemArray : public QGArray
{
public:
    typedef type* Iterator;
    typedef const type* ConstIterator;
    typedef type ValueType;

protected:
    QMemArray(int, int) : QGArray(0, 0) {}

public:
    QMemArray() {}
    QMemArray(int size) : QGArray(size*sizeof(type)) {}
    QMemArray(const QMemArray<type> &a) : QGArray(a) {}
    ~QMemArray() {}
    QMemArray<type> &operator=(const QMemArray<type> &a)
                                { return static_cast<QMemArray<type> &>(QGArray::assign(a)); }
    type *data()    const        { return static_cast<type *>(QGArray::data()); }
    uint  nrefs()   const        { return QGArray::nrefs(); }
    uint  size()    const        { return QGArray::size()/sizeof(type); }
    uint  count()   const        { return size(); }
    bool  isEmpty() const        { return QGArray::size() == 0; }
    bool  isNull()  const        { return QGArray::data() == 0; }
    bool  resize(uint size)        { return QGArray::resize(size*sizeof(type)); }
    bool  resize(uint size, Optimization optim) { return QGArray::resize(size*sizeof(type), optim); }
    bool  truncate(uint pos)        { return QGArray::resize(pos*sizeof(type)); }
    bool  fill(const type &d, int size = -1)
        { return QGArray::fill(reinterpret_cast<char*>(&d),size,sizeof(type)); }
    void  detach()                { QGArray::detach(); }
    QMemArray<type>   copy() const
        { QMemArray<type> tmp; return tmp.duplicate(*this); }
    QMemArray<type>& assign(const QMemArray<type>& a)
        { return static_cast<QMemArray<type> &>(QGArray::assign(a)); }
    QMemArray<type>& assign(const type *a, uint n)
        { return static_cast<QMemArray<type>&>(QGArray::assign(reinterpret_cast<char*>(a),n*sizeof(type))); }
    QMemArray<type>& duplicate(const QMemArray<type>& a)
        { return static_cast<QMemArray<type> &>(QGArray::duplicate(a)); }
    QMemArray<type>& duplicate(const type *a, uint n)
        { return static_cast<QMemArray<type> &>(QGArray::duplicate(reinterpret_cast<char*>(a),n*sizeof(type))); }
    QMemArray<type>& setRawData(const type *a, uint n)
        { return static_cast<QMemArray<type> &>(QGArray::setRawData(reinterpret_cast<char*>(a), n*sizeof(type))); }
    void resetRawData(const type *a, uint n)
        { QGArray::resetRawData(reinterpret_cast<char*>(a),n*sizeof(type)); }
    int         find(const type &d, uint i=0) const
        { return QGArray::find(reinterpret_cast<char*>(&d),i,sizeof(type)); }
    int         contains(const type &d) const
        { return QGArray::contains(reinterpret_cast<char*>(&d),sizeof(type)); }
    void sort() { QGArray::sort(sizeof(type)); }
    int  bsearch(const type &d) const
        { return QGArray::bsearch(reinterpret_cast<const char*>(&d),sizeof(type)); }
    type& operator[](int i) const
        { return *static_cast<type *>(QGArray::at(i*sizeof(type))); }
    type& at(uint i) const
        { return *static_cast<type *>(QGArray::at(i*sizeof(type))); }
    operator const type*() const { return static_cast<const type *>(QGArray::data()); }
    bool operator==(const QMemArray<type> &a) const { return isEqual(a); }
    bool operator!=(const QMemArray<type> &a) const { return !isEqual(a); }
    Iterator begin() { return data(); }
    Iterator end() { return data() + size(); }
    ConstIterator begin() const { return data(); }
    ConstIterator end() const { return data() + size(); }
};

#endif // QARRAY_H

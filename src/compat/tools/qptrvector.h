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

#ifndef QPTRVECTOR_H
#define QPTRVECTOR_H

#ifndef QT_H
#include "qgvector.h"
#endif // QT_H

template<class type>
class QPtrVector
#ifdef Q_QDOC
        : public QPtrCollection
#else
        : public QGVector
#endif
{
public:
    QPtrVector()                                { }
    QPtrVector(uint size) : QGVector(size) { }
    QPtrVector(const QPtrVector<type> &v) : QGVector(v) { }
    ~QPtrVector()                                { clear(); }
    QPtrVector<type> &operator=(const QPtrVector<type> &v)
                        { return (QPtrVector<type>&)QGVector::operator=(v); }
    bool operator==(const QPtrVector<type> &v) const { return QGVector::operator==(v); }
    type **data()   const                { return (type **)QGVector::data(); }
    uint  size()    const                { return QGVector::size(); }
    uint  count()   const                { return QGVector::count(); }
    bool  isEmpty() const                { return QGVector::count() == 0; }
    bool  isNull()  const                { return QGVector::size() == 0; }
    bool  resize(uint size)                { return QGVector::resize(size); }
    bool  insert(uint i, const type *d){ return QGVector::insert(i,(Item)d); }
    bool  remove(uint i)                { return QGVector::remove(i); }
    type *take(uint i)                { return (type *)QGVector::take(i); }
    void  clear()                        { QGVector::clear(); }
    bool  fill(const type *d, int size=-1)
                                        { return QGVector::fill((Item)d,size);}
    void  sort()                        { QGVector::sort(); }
    int          bsearch(const type *d) const{ return QGVector::bsearch((Item)d); }
    int          findRef(const type *d, uint i=0) const
                                        { return QGVector::findRef((Item)d,i);}
    int          find(const type *d, uint i= 0) const
                                        { return QGVector::find((Item)d,i); }
    uint  containsRef(const type *d) const
                                { return QGVector::containsRef((Item)d); }
    uint  contains(const type *d) const
                                        { return QGVector::contains((Item)d); }
    type *operator[](int i) const        { return (type *)QGVector::at(i); }
    type *at(uint i) const                { return (type *)QGVector::at(i); }
    void  toList(QGList *list) const        { QGVector::toList(list); }

#ifdef Q_QDOC
protected:
    virtual int compareItems(QPtrCollection::Item d1, QPtrCollection::Item d2);
    virtual QDataStream& read(QDataStream &s, QPtrCollection::Item &d);
    virtual QDataStream& write(QDataStream &s, QPtrCollection::Item d) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrVector<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QPtrVector<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

#endif // QVECTOR_H

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

#ifndef QINTDICT_H
#define QINTDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H

template<class type>
class QIntDict
#ifdef qdoc
        : public QPtrCollection
#else
        : public QGDict
#endif
{
public:
    QIntDict(int size=17) : QGDict(size,IntKey,0,0) {}
    QIntDict(const QIntDict<type> &d) : QGDict(d) {}
   ~QIntDict()                                { clear(); }
    QIntDict<type> &operator=(const QIntDict<type> &d)
                        { return static_cast<QIntDict<type>&>(QGDict::operator=(d)); }
    uint  count()   const                { return QGDict::count(); }
    uint  size()    const                { return QGDict::size(); }
    bool  isEmpty() const                { return QGDict::count() == 0; }
    void  insert(long k, const type *d)
                                        { QGDict::look_int(k,Item(d),1); }
    void  replace(long k, const type *d)
                                        { QGDict::look_int(k,Item(d),2); }
    bool  remove(long k)                { return QGDict::remove_int(k); }
    type *take(long k)                { return static_cast<type*>(QGDict::take_int(k)); }
    type *find(long k) const
                { return static_cast<type *>(const_cast<QIntDict *>(this)->QGDict::look_int(k,0,0)); }
    type *operator[](long k) const
                { return static_cast<type *>(const_cast<QIntDict *>(this)->QGDict::look_int(k,0,0)); }
    void  clear()                        { QGDict::clear(); }
    void  resize(uint n)                { QGDict::resize(n); }
    void  statistics() const                { QGDict::statistics(); }

#ifdef qdoc
protected:
    virtual QDataStream& read(QDataStream &, QPtrCollection::Item &);
    virtual QDataStream& write(QDataStream &, QPtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QIntDict<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QIntDict<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type*>(d);
}

template<class type>
class QIntDictIterator : public QGDictIterator
{
public:
    QIntDictIterator(const QIntDict<type> &d) :QGDictIterator(d) {}
   ~QIntDictIterator()              {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()              { return static_cast<type *>(QGDictIterator::toFirst()); }
    operator type *()  const  { return static_cast<type *>(QGDictIterator::get()); }
    type *current()    const  { return static_cast<type *>(QGDictIterator::get()); }
    long  currentKey() const  { return QGDictIterator::getKeyInt(); }
    type *operator()()              { return static_cast<type *>(QGDictIterator::operator()()); }
    type *operator++()              { return static_cast<type *>(QGDictIterator::operator++()); }
    type *operator+=(uint j)  { return static_cast<type *>(QGDictIterator::operator+=(j));}
};

#endif // QINTDICT_H

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

#ifndef QPTRDICT_H
#define QPTRDICT_H

#include "qgdict.h"

template<class type>
class QPtrDict
#ifdef qdoc
        : public QPtrCollection
#else
        : public QGDict
#endif
{
public:
    QPtrDict(int size=17) : QGDict(size,PtrKey,0,0) {}
    QPtrDict(const QPtrDict<type> &d) : QGDict(d) {}
   ~QPtrDict()                                { clear(); }
    QPtrDict<type> &operator=(const QPtrDict<type> &d)
                        { return static_cast<QPtrDict<type> &>(QGDict::operator=(d)); }
    uint  count()   const                { return QGDict::count(); }
    uint  size()    const                { return QGDict::size(); }
    bool  isEmpty() const                { return QGDict::count() == 0; }
    void  insert(void *k, const type *d)
                                        { QGDict::look_ptr(k,Item(d),1); }
    void  replace(void *k, const type *d)
                                        { QGDict::look_ptr(k,Item(d),2); }
    bool  remove(void *k)                { return QGDict::remove_ptr(k); }
    type *take(void *k)                { return static_cast<type*>(QGDict::take_ptr(k)); }
    type *find(void *k) const
                { return static_cast<type *>(const_cast<QPtrDict *>(this)->QGDict::look_ptr(k,0,0)); }
    type *operator[](void *k) const
                { return static_cast<type *>(const_cast<QPtrDict *>(this)->QGDict::look_ptr(k,0,0)); }
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
template<> inline void QPtrDict<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type>
inline void QPtrDict<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type *>(d);
}

template<class type>
class QPtrDictIterator : public QGDictIterator
{
public:
    QPtrDictIterator(const QPtrDict<type> &d) :QGDictIterator(d) {}
   ~QPtrDictIterator()              {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()              { return static_cast<type *>(QGDictIterator::toFirst()); }
    operator type *()  const  { return static_cast<type *>(QGDictIterator::get()); }
    type *current()    const  { return static_cast<type *>(QGDictIterator::get()); }
    void *currentKey() const  { return QGDictIterator::getKeyPtr(); }
    type *operator()()              { return static_cast<type *>(QGDictIterator::operator()()); }
    type *operator++()              { return static_cast<type *>(QGDictIterator::operator++()); }
    type *operator+=(uint j)  { return static_cast<type *>(QGDictIterator::operator+=(j));}
};

#endif // QPTRDICT_H

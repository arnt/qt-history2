/****************************************************************************
**
** Definition of QPtrDict template class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPTRDICT_H
#define QPTRDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H

template<class type>
class QPtrDict
#ifdef Q_QDOC
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
                        { return (QPtrDict<type>&)QGDict::operator=(d); }
    uint  count()   const                { return QGDict::count(); }
    uint  size()    const                { return QGDict::size(); }
    bool  isEmpty() const                { return QGDict::count() == 0; }
    void  insert(void *k, const type *d)
                                        { QGDict::look_ptr(k,(Item)d,1); }
    void  replace(void *k, const type *d)
                                        { QGDict::look_ptr(k,(Item)d,2); }
    bool  remove(void *k)                { return QGDict::remove_ptr(k); }
    type *take(void *k)                { return (type*)QGDict::take_ptr(k); }
    type *find(void *k) const
                { return (type *)((QGDict*)this)->QGDict::look_ptr(k,0,0); }
    type *operator[](void *k) const
                { return (type *)((QGDict*)this)->QGDict::look_ptr(k,0,0); }
    void  clear()                        { QGDict::clear(); }
    void  resize(uint n)                { QGDict::resize(n); }
    void  statistics() const                { QGDict::statistics(); }

#ifdef Q_QDOC
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
    if (del_item) delete (type *)d;
}

template<class type>
class QPtrDictIterator : public QGDictIterator
{
public:
    QPtrDictIterator(const QPtrDict<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QPtrDictIterator()              {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()              { return (type *)QGDictIterator::toFirst(); }
    operator type *()  const  { return (type *)QGDictIterator::get(); }
    type *current()    const  { return (type *)QGDictIterator::get(); }
    void *currentKey() const  { return QGDictIterator::getKeyPtr(); }
    type *operator()()              { return (type *)QGDictIterator::operator()(); }
    type *operator++()              { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#endif // QPTRDICT_H

/****************************************************************************
**
** Definition of QPtrList template/macro class.
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

#ifndef QPTRLIST_H
#define QPTRLIST_H

#ifndef QT_H
#include "qglist.h"
#include "qlist.h"
#endif // QT_H

template<class type>
class QPtrListStdIterator : public QGListStdIterator
{
public:
    inline QPtrListStdIterator(QLNode* n): QGListStdIterator(n) {}
    type *operator*() { return node ? (type *)node->getData() : 0; }
    inline QPtrListStdIterator<type> operator++()
    { node = next(); return *this; }
    inline QPtrListStdIterator<type> operator++(int)
    { QLNode* n = node; node = next(); return QPtrListStdIterator<type>(n); }
    inline bool operator==(const QPtrListStdIterator<type>& it) const { return node == it.node; }
    inline bool operator!=(const QPtrListStdIterator<type>& it) const { return node != it.node; }
};


template<class type>
class QPtrList
#ifdef Q_QDOC
        : public QPtrCollection
#else
        : public QGList
#endif
{
public:

    QPtrList()                                {}
    QPtrList(const QPtrList<type> &l) : QGList(l) {}
    ~QPtrList()                                { clear(); }
    QPtrList<type> &operator=(const QPtrList<type> &l)
                        { return (QPtrList<type>&)QGList::operator=(l); }

    QPtrList(const QList<type *> &l);
    QPtrList<type> &operator=(const QList<type *> &l);
    operator QList<type *>();

    bool operator==(const QPtrList<type> &list) const
    { return QGList::operator==(list); }
    bool operator!=(const QPtrList<type> &list) const
    { return !QGList::operator==(list); }
    uint  count()   const                { return QGList::count(); }
    bool  isEmpty() const                { return QGList::count() == 0; }
    bool  insert(uint i, const type *d){ return QGList::insertAt(i,(QPtrCollection::Item)d); }
    void  inSort(const type *d)        { QGList::inSort((QPtrCollection::Item)d); }
    void  prepend(const type *d)        { QGList::insertAt(0,(QPtrCollection::Item)d); }
    void  append(const type *d)        { QGList::append((QPtrCollection::Item)d); }
    bool  remove(uint i)                { return QGList::removeAt(i); }
    bool  remove()                        { return QGList::remove((QPtrCollection::Item)0); }
    bool  remove(const type *d)        { return QGList::remove((QPtrCollection::Item)d); }
    bool  removeRef(const type *d)        { return QGList::removeRef((QPtrCollection::Item)d); }
    void  removeNode(QLNode *n)        { QGList::removeNode(n); }
    bool  removeFirst()                        { return QGList::removeFirst(); }
    bool  removeLast()                        { return QGList::removeLast(); }
    type *take(uint i)                { return (type *)QGList::takeAt(i); }
    type *take()                        { return (type *)QGList::take(); }
    type *takeNode(QLNode *n)                { return (type *)QGList::takeNode(n); }
    void  clear()                        { QGList::clear(); }
    void  sort()                        { QGList::sort(); }
    int          find(const type *d)                { return QGList::find((QPtrCollection::Item)d); }
    int          findNext(const type *d)        { return QGList::find((QPtrCollection::Item)d,false); }
    int          findRef(const type *d)        { return QGList::findRef((QPtrCollection::Item)d); }
    int          findNextRef(const type *d){ return QGList::findRef((QPtrCollection::Item)d,false);}
    uint  contains(const type *d) const { return QGList::contains((QPtrCollection::Item)d); }
    uint  containsRef(const type *d) const
                                        { return QGList::containsRef((QPtrCollection::Item)d); }
    bool replace(uint i, const type *d) { return QGList::replaceAt(i, (QPtrCollection::Item)d); }
    type *at(uint i)                        { return (type *)QGList::at(i); }
    int          at() const                        { return QGList::at(); }
    type *current()  const                { return (type *)QGList::get(); }
    QLNode *currentNode()  const        { return QGList::currentNode(); }
    type *getFirst() const                { return (type *)QGList::cfirst(); }
    type *getLast()  const                { return (type *)QGList::clast(); }
    type *first()                        { return (type *)QGList::first(); }
    type *last()                        { return (type *)QGList::last(); }
    type *next()                        { return (type *)QGList::next(); }
    type *prev()                        { return (type *)QGList::prev(); }
    void  toVector(QGVector *vec)const{ QGList::toVector(vec); }


    // standard iterators
    typedef QPtrListStdIterator<type> Iterator;
    typedef QPtrListStdIterator<type> ConstIterator;
    inline Iterator begin() { return QGList::begin(); }
    inline ConstIterator begin() const { return QGList::begin(); }
    inline ConstIterator constBegin() const { return QGList::begin(); }
    inline Iterator end() { return QGList::end(); }
    inline ConstIterator end() const { return QGList::end(); }
    inline ConstIterator constEnd() const { return QGList::end(); }
    inline Iterator erase(Iterator it) { return QGList::erase(it); }
    // stl syntax compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;


#ifdef Q_QDOC
protected:
    virtual int compareItems(QPtrCollection::Item, QPtrCollection::Item);
    virtual QDataStream& read(QDataStream&, QPtrCollection::Item&);
    virtual QDataStream& write(QDataStream&, QPtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrList<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QPtrList<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

template<class type>
QPtrList<type>::QPtrList(const QList<type *> &l)
    : QGList()
{
    for (int i = 0; i < l.size(); ++i)
        append(l.at(i));
}

template<class type>
QPtrList<type> &QPtrList<type>::operator=(const QList<type *> &l)
{
    QPtrList<type> pl(l);
    *this = pl;
    return *this;
}

template<class type>
QPtrList<type>::operator QList<type *>()
{
    QList<type *> l;
    for (Iterator it = begin(); it != end(); ++it)
        l.append(*it);
    return l;
}


template<class type>
class QPtrListIterator : public QGListIterator
{
public:
    QPtrListIterator(const QPtrList<type> &l) :QGListIterator((QGList &)l) {}
   ~QPtrListIterator()              {}
    uint  count()   const     { return list->count(); }
    bool  isEmpty() const     { return list->count() == 0; }
    bool  atFirst() const     { return QGListIterator::atFirst(); }
    bool  atLast()  const     { return QGListIterator::atLast(); }
    type *toFirst()              { return (type *)QGListIterator::toFirst(); }
    type *toLast()              { return (type *)QGListIterator::toLast(); }
    operator type *() const   { return (type *)QGListIterator::get(); }
    type *operator*()         { return (type *)QGListIterator::get(); }

    // No good, since QPtrList<char> (ie. QStrList fails...
    //
    // MSVC++ gives warning
    // Sunpro C++ 4.1 gives error
    //    type *operator->()        { return (type *)QGListIterator::get(); }

    type *current()   const   { return (type *)QGListIterator::get(); }
    type *operator()()              { return (type *)QGListIterator::operator()();}
    type *operator++()              { return (type *)QGListIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}
    type *operator--()              { return (type *)QGListIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}
    QPtrListIterator<type>& operator=(const QPtrListIterator<type>&it)
                              { QGListIterator::operator=(it); return *this; }
};

#endif // QPTRLIST_H

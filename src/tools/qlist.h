/****************************************************************************
** $Id: //depot/qt/main/src/tools/qlist.h#2 $
**
** Definition of QList template/macro class
**
** Author  : Haavard Nord
** Created : 920701
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QLIST_H
#define QLIST_H

#include "qglist.h"


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QListdeclare QListMdeclare
#define QList QListM
#endif
#define QListM(type) name2(QListM_,type)

#define QListMdeclare(type)						      \
class QListM(type) : public QGList					      \
{									      \
public:									      \
    QListM(type)()			{}				      \
    QListM(type)( const QListM(type) &l ) : QGList(l) {}		      \
   ~QListM(type)()			{ clear(); }			      \
    QListM(type)& operator=(const QListM(type) &l)			      \
			{ return (QListM(type)&)QGList::operator=(l); }	      \
    uint  count()   const		{ return QGList::count(); }	      \
    bool  isEmpty() const		{ return QGList::count() == 0; }      \
    bool  insert( const type *d )	{ return QGList::insert(GCI(d)); }    \
    bool  inSort( const type *d )	{ return QGList::inSort(GCI(d)); }    \
    bool  append( const type *d )	{ return QGList::append(GCI(d)); }    \
    bool  insertAt( const type *d, uint i )				      \
					{ return QGList::insertAt((GCI)d,i);} \
    bool  remove( const type *d=0 )	{ return QGList::remove(GCI(d)); }    \
    bool  removeFirst()			{ return QGList::removeFirst(); }     \
    bool  removeLast()			{ return QGList::removeLast(); }      \
    type *take()			{ return (type *)QGList::take(); }    \
    void  clear()			{ QGList::clear(); }		      \
    int	  find( const type *d )		{ return QGList::find(GCI(d)); }      \
    int	  findNext( const type *d )	{ return QGList::find((GCI)d,FALSE);} \
    int	  findRef( const type *d )	{ return QGList::findRef(GCI(d)); }   \
    int	  findNextRef( const type *d ){ return QGList::findRef((GCI)d,FALSE);}\
    uint  contains( const type *d )	{ return QGList::contains(GCI(d)); }  \
    uint  containsRef( const type *d )	{ return QGList::containsRef(GCI(d));}\
    type *at( uint i )			{ return (type *)QGList::at(i); }     \
    uint  at() const			{ return QGList::at(); }	      \
	  operator type *() const	{ return (type *)QGList::get(); }     \
    type *current() const		{ return (type *)QGList::get(); }     \
    type *first()			{ return (type *)QGList::first(); }   \
    type *last()			{ return (type *)QGList::last(); }    \
    type *next()			{ return (type *)QGList::next(); }    \
    type *prev()			{ return (type *)QGList::prev(); }    \
    void  asVector( QGVector &vec )const{ QGList::asVector(vec); }	      \
    int	  apply( int (*applyFunc)(type*,void*), void *x ) const		      \
				{ return QGList::apply( (GCF)applyFunc, x ); }\
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QListIteratordeclare QListIteratorMdeclare
#define QListIterator QListIteratorM
#endif
#define QListIteratorM(type) name2(QListIteratorM_,type)

#define QListIteratorMdeclare(type)					      \
class QListIteratorM(type) : public QGListIterator			      \
{									      \
public:									      \
    QListIteratorM(type)(const QListM(type) &l) :QGListIterator((QGList &)l){}\
   ~QListIteratorM(type)()    {}					      \
    int	  count()   const     { return list->count(); }			      \
    bool  isEmpty() const     { return list->count() == 0; }		      \
    bool  atFirst() const     { return QGListIterator::atFirst(); }	      \
    bool  atLast()  const     { return QGListIterator::atLast(); }	      \
    type *toFirst()	      { return (type *)QGListIterator::toFirst(); }   \
    type *toLast()	      { return (type *)QGListIterator::toLast(); }    \
    operator type *() const   { return (type *)QGListIterator::get(); }	      \
    type *current()   const   { return (type *)QGListIterator::get(); }	      \
    type *operator()()	      { return (type *)QGListIterator::operator()();} \
    type *operator++()	      { return (type *)QGListIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}\
    type *operator--()	      { return (type *)QGListIterator::operator--(); }\
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QList
#define QList QListT
#endif

template<class type> class QListT : public QGList
{
public:
    QListT()				{}
    QListT( const QListT<type> &l ) : QGList(l) {}
   ~QListT()				{ clear(); }
    QListT<type>& operator=(const QListT<type> &l)
			{ return (QListT<type>&)QGList::operator=(l); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    bool  insert( const type *d )	{ return QGList::insert(GCI(d)); }
    bool  inSort( const type *d )	{ return QGList::inSort(GCI(d)); }
    bool  append( const type *d )	{ return QGList::append(GCI(d)); }
    bool  insertAt( const type *d, uint i )
					{ return QGList::insertAt((GCI)d,i); }
    bool  remove( const type *d=0 )	{ return QGList::remove(GCI(d)); }
    bool  removeFirst()			{ return QGList::removeFirst(); }
    bool  removeLast()			{ return QGList::removeLast(); }
    type *take()			{ return (type *)QGList::take(); }
    void  clear()			{ QGList::clear(); }
    int	  find( const type *d )		{ return QGList::find(GCI(d)); }
    int	  findNext( const type *d )	{ return QGList::find((GCI)d,FALSE); }
    int	  findRef( const type *d )	{ return QGList::findRef(GCI(d)); }
    int	  findNextRef( const type *d ){ return QGList::findRef((GCI)d,FALSE); }
    uint  contains( const type *d )	{ return QGList::contains(GCI(d)); }
    uint  containsRef( const type *d )	{ return QGList::containsRef(GCI(d)); }
    type *at( uint i )			{ return (type *)QGList::at(i); }
    uint  at() const			{ return QGList::at(); }
	  operator type *() const	{ return (type *)QGList::get(); }
    type *current() const		{ return (type *)QGList::get(); }
    type *first()			{ return (type *)QGList::first(); }
    type *last()			{ return (type *)QGList::last(); }
    type *next()			{ return (type *)QGList::next(); }
    type *prev()			{ return (type *)QGList::prev(); }
    void  asVector( QGVector &vec )const{ QGList::asVector(vec); }
    int	  apply( int (*applyFunc)(type*,void*), void *x ) const
				{ return QGList::apply( (GCF)applyFunc, x ); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QListIterator
#define QListIterator QListIteratorT
#endif

template<class type> class QListIteratorT : public QGListIterator
{
public:
    QListIteratorT(const QListT<type> &l) :QGListIterator((QGList &)l) {}
   ~QListIteratorT()	      {}
    int	  count()   const     { return list->count(); }
    bool  isEmpty() const     { return list->count() == 0; }
    bool  atFirst() const     { return QGListIterator::atFirst(); }
    bool  atLast()  const     { return QGListIterator::atLast(); }
    type *toFirst()	      { return (type *)QGListIterator::toFirst(); }
    type *toLast()	      { return (type *)QGListIterator::toLast(); }
    operator type *() const   { return (type *)QGListIterator::get(); }
    type *current()   const   { return (type *)QGListIterator::get(); }
    type *operator()()	      { return (type *)QGListIterator::operator()();}
    type *operator++()	      { return (type *)QGListIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGListIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QLIST_H

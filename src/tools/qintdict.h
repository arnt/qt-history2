/****************************************************************************
** $Id: //depot/qt/main/src/tools/qintdict.h#1 $
**
** Definition of QIntDict template/macro class
**
** Author  : Haavard Nord
** Created : 940624
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#ifndef QINTDICT_H
#define QINTDICT_H

#include "qgdict.h"


#if defined(USE_MACROCLASS)

#include <generic.h>

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QIntDictdeclare QIntDictMdeclare
#define QIntDict QIntDictM
#endif
#define QIntDictM(type) name2(QIntDictM_,type)

#define QIntDictMdeclare(type)						      \
class QIntDictM(type) : public QGDict					      \
{									      \
public:									      \
    QIntDictM(type)(int size=15):QGDict(size,0,0,TRUE) {}		      \
   ~QIntDictM(type)()		{ clear(); }				      \
    uint  count()   const	{ return QGDict::count(); }		      \
    bool  isEmpty() const	{ return QGDict::count() == 0; }	      \
    bool  insert( long k, const type *d )				      \
			{ return QGDict::look((const char*)k,(GCI)d,1)!=0;}   \
    bool  remove( long k )	{ return QGDict::remove((const char*)k); }    \
    void  clear()		{ QGDict::clear(); }			      \
    type *find( long k )	const					      \
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0);}  \
    type *operator[]( long k ) const					      \
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0);}  \
    void  statistics() const	{ QGDict::statistics(); }		      \
private:								      \
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QIntDictIteratordeclare QIntDictIteratorMdeclare
#define QIntDictIterator QIntDictIteratorM
#endif
#define QIntDictIteratorM(type) name2(QIntDictIteratorM_,type)

#define QIntDictIteratorMdeclare(type)					      \
class QIntDictIteratorM(type) : public QGDictIterator			      \
{									      \
public:									      \
    QIntDictIteratorM(type)(const QIntDictM(type) &d) :			      \
	QGDictIterator((QGDict &)d){}					      \
   ~QIntDictIteratorM(type)()	 {}					      \
    int	  count()   const     { return dict->count(); }			      \
    bool  isEmpty() const     { return dict->count() == 0; }		      \
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }   \
    operator type *() const   { return (type *)QGDictIterator::get(); }	      \
    type *current()   const   { return (type *)QGDictIterator::get(); }	      \
    type *operator()()	      { return (type *)QGDictIterator::operator()();} \
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QIntDict
#define QIntDict QIntDictT
#endif

template<class type> class QIntDictT : public QGDict
{
public:
    QIntDictT(int size=15) : QGDict(size,0,0,TRUE) {}
   ~QIntDictT()			{ clear(); }
    uint  count()   const	{ return QGDict::count(); }
    bool  isEmpty() const	{ return QGDict::count() == 0; }
    bool  insert( long k, const type *d )
			{ return QGDict::look((const char*)k,(GCI)d,1)!=0; }
    bool  remove( long k )	{ return QGDict::remove((const char*)k); }
    void  clear()		{ QGDict::clear(); }
    type *find( long k )	const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    type *operator[]( long k ) const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    void  statistics() const	{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QIntDictIterator
#define QIntDictIterator QIntDictIteratorT
#endif

template<class type> class QIntDictIteratorT : public QGDictIterator
{
public:
    QIntDictIteratorT(const QIntDictT<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QIntDictIteratorT()	      {}
    int	  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type *current()   const   { return (type *)QGDictIterator::get(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()();}
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QINTDICT_H

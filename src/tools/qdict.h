/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdict.h#18 $
**
** Definition of QDict template/macro class
**
** Created : 920821
**
** Copyright (C) 1992-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDICT_H
#define QDICT_H

#include "qgdict.h"


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QDictdeclare QDictMdeclare
#define QDict QDictM
#endif
#define QDictM(type) name2(QDictM_,type)

#define QDictMdeclare(type)						      \
class QDictM(type) : public QGDict					      \
{									      \
public:									      \
    QDictM(type)(int size=17,bool cs=TRUE,bool ck=TRUE):QGDict(size,cs,ck,0){}\
    QDictM(type)( const QDictM(type) &d ) : QGDict(d) {}		      \
   ~QDictM(type)()			{ clear(); }			      \
    QDictM(type) &operator=(const QDictM(type) &d)			      \
			{ return (QDictM(type)&)QGDict::operator=(d); }	      \
    uint  count()   const		{ return QGDict::count(); }	      \
    uint  size()    const		{ return QGDict::size(); }	      \
    bool  isEmpty() const		{ return QGDict::count() == 0; }      \
    void  insert( const char *k, const type *d )			      \
					{ QGDict::look(k,(GCI)d,1); }	      \
    void  replace( const char *k, const type *d )			      \
					{ QGDict::look(k,(GCI)d,2); }	      \
    bool  remove( const char *k )	{ return QGDict::remove(k); }	      \
    type *take( const char *k )		{ return (type *)QGDict::take(k); }   \
    void  clear()			{ QGDict::clear(); }		      \
    type *find( const char *k ) const					      \
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0);}   \
    type *operator[]( const char *k ) const				      \
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0);}   \
    void  statistics() const		{ QGDict::statistics(); }	      \
private:								      \
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QDictIteratordeclare QDictIteratorMdeclare
#define QDictIterator QDictIteratorM
#endif
#define QDictIteratorM(type) name2(QDictIteratorM_,type)

#define QDictIteratorMdeclare(type)					      \
class QDictIteratorM(type) : public QGDictIterator			      \
{									      \
public:									      \
    QDictIteratorM(type)(const QDictM(type) &d) :QGDictIterator((QGDict &)d){}\
   ~QDictIteratorM(type)()    {}					      \
    uint  count()   const     { return dict->count(); }			      \
    bool  isEmpty() const     { return dict->count() == 0; }		      \
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }   \
    operator type *() const   { return (type *)QGDictIterator::get(); }	      \
    type *current()   const   { return (type *)QGDictIterator::get(); }	      \
    const char *currentKey() const					      \
			      { return QGDictIterator::getKey(); }	      \
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }\
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QDict
#define QDict QDictT
#endif

template<class type> class QDictT : public QGDict
{
public:
    QDictT(int size=17,bool cs=TRUE,bool ck=TRUE) : QGDict(size,cs,ck,0) {}
    QDictT( const QDictT<type> &d ) : QGDict(d) {}
   ~QDictT()				{ clear(); }
    QDictT<type> &operator=(const QDictT<type> &d)
			{ return (QDictT<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }
    void  insert( const char *k, const type *d )
					{ QGDict::look(k,(GCI)d,1); }
    void  replace( const char *k, const type *d )
					{ QGDict::look(k,(GCI)d,2); }
    bool  remove( const char *k )	{ return QGDict::remove(k); }
    type *take( const char *k )		{ return (type *)QGDict::take(k); }
    void  clear()			{ QGDict::clear(); }
    type *find( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    type *operator[]( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QDictIterator
#define QDictIterator QDictIteratorT
#endif

template<class type> class QDictIteratorT : public QGDictIterator
{
public:
    QDictIteratorT(const QDictT<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QDictIteratorT()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type *current()   const   { return (type *)QGDictIterator::get(); }
    const char *currentKey() const
			      { return QGDictIterator::getKey(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QDICT_H

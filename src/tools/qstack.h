/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstack.h#1 $
**
** Definition of QStack template/macro class
**
** Author  : Haavard Nord
** Created : 920917
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTACK_H
#define QSTACK_H

#include "qglist.h"


#if defined(USE_MACROCLASS)

#include <generic.h>

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QStackdeclare QStackMdeclare
#define QStack QStackM
#endif
#define QStackM(type) name2(QStackM_,type)

#define QStackMdeclare(type)						      \
class QStackM(type) : private QGList					      \
{									      \
public:									      \
    QStackM(type)()			{}				      \
    QStackM(type)( const QStackM(type) &s ) : QGList(s) {}		      \
   ~QStackM(type)()			{ clear(); }			      \
    QStackM(type) &operator=(const QStackM(type) &s)			      \
			{ return (QStackM(type)&)QGList::operator=(s); }      \
    void  autoDelete( bool del )	{ QCollection::autoDelete(del); }     \
    uint  count()   const		{ return QGList::count(); }	      \
    bool  isEmpty() const		{ return QGList::count() == 0; }      \
    bool  push( const type *d )		{ return QGList::insert(GCI(d)); }    \
    type *pop()				{ return (type *)QGList::takeFirst();}\
    bool  remove()			{ return QGList::removeFirst(); }     \
    void  clear()			{ QGList::clear(); }		      \
    type *top()	    const		{ return (type *)QGList::cfirst(); }  \
	  operator type *() const	{ return (type *)QGList::cfirst(); }  \
    type *current() const		{ return (type *)QGList::cfirst(); }  \
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QStack
#define QStack QStackT
#endif

template<class type> class QStackT : private QGList
{
public:
    QStackT()				{}
    QStackT( const QStackT<type> &s ) : QGList(s) {}
   ~QStackT()				{ clear(); }
    QStackT<type> &operator=(const QStackT<type> &s)
			{ return (QStackT<type>&)QGList::operator=(s); }
    void  autoDelete( bool del )	{ QCollection::autoDelete(del); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    bool  push( const type *d )		{ return QGList::insert(GCI(d)); }
    type *pop()				{ return (type *)QGList::takeFirst(); }
    bool  remove()			{ return QGList::removeFirst(); }
    void  clear()			{ QGList::clear(); }
    type *top()	    const		{ return (type *)QGList::cfirst(); }
	  operator type *() const	{ return (type *)QGList::cfirst(); }
    type *current() const		{ return (type *)QGList::cfirst(); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS


#endif // QSTACK_H

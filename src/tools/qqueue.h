/****************************************************************************
** $Id: //depot/qt/main/src/tools/qqueue.h#3 $
**
** Definition of QQueue template/macro class
**
** Author  : Haavard Nord
** Created : 920917
**
** Copyright (C) 1992,1993 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QQUEUE_H
#define QQUEUE_H

#include "qglist.h"


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QQueuedeclare QQueueMdeclare
#define QQueue QQueueM
#endif
#define QQueueM(type) name2(QQueueM_,type)

#define QQueueMdeclare(type)						      \
class QQueueM(type) : private QGList					      \
{									      \
public:									      \
    QQueueM(type)()			{}				      \
    QQueueM(type)( const QQueueM(type) &q ) : QGList(q) {}		      \
   ~QQueueM(type)()			{ clear(); }			      \
    QQueueM(type)& operator=(const QQueueM(type) &q)			      \
			{ return (QQueueM(type)&)QGList::operator=(q); }      \
    bool  autoDelete() const		{ return QCollection::autoDelete(); } \
    void  setAutoDelete( bool del )	{ QCollection::setAutoDelete(del); }  \
    uint  count()   const		{ return QGList::count(); }	      \
    bool  isEmpty() const		{ return QGList::count() == 0; }      \
    bool  enqueue( const type *d )	{ return QGList::append(GCI(d)); }    \
    type *dequeue()			{ return (type *)QGList::takeFirst();}\
    bool  remove()			{ return QGList::removeFirst(); }     \
    void  clear()			{ QGList::clear(); }		      \
    type *head()    const		{ return (type *)QGList::cfirst(); }  \
	  operator type *() const	{ return (type *)QGList::cfirst(); }  \
    type *current() const		{ return (type *)QGList::cfirst(); }  \
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QQueue
#define QQueue QQueueT
#endif

template<class type> class QQueueT : private QGList
{
public:
    QQueueT()				{}
    QQueueT( const QQueueT<type> &q ) : QGList(q) {}
   ~QQueueT()				{ clear(); }
    QQueueT<type>& operator=(const QQueueT<type> &q)
			{ return (QQueueT<type>&)QGList::operator=(q); }
    bool  autoDelete() const		{ return QCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ QCollection::setAutoDelete(del); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    bool  enqueue( const type *d )	{ return QGList::append(GCI(d)); }
    type *dequeue()			{ return (type *)QGList::takeFirst();}
    bool  remove()			{ return QGList::removeFirst(); }
    void  clear()			{ QGList::clear(); }
    type *head()    const		{ return (type *)QGList::cfirst(); }
	  operator type *() const	{ return (type *)QGList::cfirst(); }
    type *current() const		{ return (type *)QGList::cfirst(); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS


#endif // QQUEUE_H

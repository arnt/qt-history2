/****************************************************************************
** $Id: //depot/qt/main/src/tools/qintcach.h#1 $
**
** Definition of QIntCache template/macro class
**
** Author  : Eirik Eng
** Created : 950209
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QINTCACH_H
#define QINTCACH_H

#include "qgcache.h"


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QIntCachedeclare QIntCacheMdeclare
#define QIntCache QIntCacheM
#endif
#define QIntCacheM(type) name2(QIntCacheM_,type)

#define QIntCacheMdeclare(type)						      \
class QIntCacheM(type) : public QGCache					      \
{									      \
public:									      \
    QIntCacheM(type)(long maxCost, int size=17)                               \
        : QGCache(maxCost,size,0,0,TRUE){}                                    \
   ~QIntCacheM(type)()		      { clear(); }			      \
    uint  count()   const	      { return QGCache::count(); }	      \
    uint  size()    const	      { return QGCache::size(); }	      \
    bool  isEmpty() const	      { return QGCache::count() == 0; }       \
    bool  insert( long k, const type *d , long c=1, int p=0 )                 \
    		        { return QGCache::insert((const char*)k,(GCI)d,c,p);} \
    bool  remove( long k )   { return QGCache::remove((const char*)k); }      \
    type *take( long k )     { return (type *)QGCache::take((const char*)k); }\
    void  clear()		      { QGCache::clear(); }		      \
    type *find( long k ) const{ return (type *)QGCache::find((const char*)k);}\
    type *operator[]( long k ) const				              \
                             { return (type *)QGCache::find((const char*)k);} \
    void  statistics() const	      { QGCache::statistics(); }	      \
private:								      \
    void  deleteItem( GCI d )         { if ( del_item ) delete (type *)d; }   \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QIntCache
#define QIntCache QIntCacheT
#endif

template<class type> class QIntCacheT : public QGCache
{
public:
    QIntCacheT(int size=17):QGCache(size,0,0,TRUE) {}
   ~QIntCacheT()	     { clear(); }
    uint  count()   const    { return QGCache::count(); }
    uint  size()    const    { return QGCache::size(); }
    bool  isEmpty() const    { return QGCache::count() == 0; }
    bool  insert( long k, const type *d , long c, short p )
			 { return QGCache::insert((const char *)k,(GCI)d,c,p);}
    bool  remove( long k )   { return QGCache::remove((const char *)k); }
    type *take( long k )     { return (type *)QGCache::take((const char *)k); }
    void  clear()	     { QGCache::clear(); }
    type *find( long k ) const 
                             { return (type *)QGCache::find((const char *)k);}
    type *operator[]( long k ) const
                             { return (type *)QGCache::find((const char *)k);}
    void  statistics() const { QGCache::statistics(); }
private:
    void  deleteItem( GCI d ){ if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS

#endif // QINTCACH_H

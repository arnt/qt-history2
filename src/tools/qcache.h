/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcache.h#1 $
**
** Definition of QCache template/macro class
**
** Author  : Eirik Eng
** Created : 950209
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCACHE_H
#define QCACHE_H

#include "qgcache.h"


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QCachedeclare QCacheMdeclare
#define QCache QCacheM
#endif
#define QCacheM(type) name2(QCacheM_,type)

#define QCacheMdeclare(type)						      \
class QCacheM(type) : public QGCache					      \
{									      \
public:									      \
    QCacheM(type)(long maxCost, int size=17,bool cs=TRUE,bool ck=TRUE)        \
        :QGCache(maxCost,size,cs,ck,0){}                                      \
   ~QCacheM(type)()		      { clear(); }			      \
    uint  count()   const	      { return QGCache::count(); }	      \
    uint  size()    const	      { return QGCache::size(); }	      \
    bool  isEmpty() const	      { return QGCache::count() == 0; }       \
    void  insert( const char *k, const type *d , long c=1, int p=0 )          \
			              { QGCache::insert(k,(GCI)d,c,p);}       \
    bool  remove( const char *k )     { return QGCache::remove(k); }	      \
    type *take( const char *k )	      { return (type *)QGCache::take(k); }    \
    void  clear()		      { QGCache::clear(); }		      \
    type *find( const char *k ) const { return (type *)QGCache::find(k);}     \
    type *operator[]( const char *k ) const				      \
                                      { return (type *)QGCache::find(k);}     \
    void  statistics() const	      { QGCache::statistics(); }	      \
private:								      \
    void  deleteItem( GCI d )         { if ( del_item ) delete (type *)d; }   \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QCache
#define QCache QCacheT
#endif

template<class type> class QCacheT : public QGCache
{
public:
    QCacheT(int size=17,bool cs=TRUE,bool ck=TRUE) : QGCache(size,cs,ck,0) {}
   ~QCacheT()			      { clear(); }
    uint  count()   const	      { return QGCache::count(); }
    uint  size()    const	      { return QGCache::size(); }
    bool  isEmpty() const	      { return QGCache::count() == 0; }
    void  insert( const char *k, const type *d , long c, short p )
			              { QGCache::insert(k,(GCI)d,c,p);}
    bool  remove( const char *k )     { return QGCache::remove(k); }
    type *take( const char *k )	      { return (type *)QGCache::take(k); }
    void  clear()		      { QGCache::clear(); }
    type *find( const char *k ) const { return (type *)QGCache::find(k);}
    type *operator[]( const char *k ) const
                                      { return (type *)QGCache::find(k);}
    void  statistics() const	      { QGCache::statistics(); }
private:
    void  deleteItem( GCI d )         { if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS

#endif // QCACHE_H

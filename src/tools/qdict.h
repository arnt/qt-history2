/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdict.h#27 $
**
** Definition of QDict template/macro class
**
** Created : 920821
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QDICT_H
#define QDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


template<class type> class Q_EXPORT QDict : public QGDict
{
public:
    QDict(int size=17,bool cs=TRUE,bool ck=TRUE) : QGDict(size,cs,ck,0) {}
    QDict( const QDict<type> &d ) : QGDict(d) {}
   ~QDict()				{ clear(); }
    QDict<type> &operator=(const QDict<type> &d)
			{ return (QDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }

    void  insert( const char *k, const type *d )
					{ QGDict::look(k,(Item)d,1); }
    void  replace( const char *k, const type *d )
					{ QGDict::look(k,(Item)d,2); }
    bool  remove( const char *k )	{ return QGDict::remove(k); }
    type *take( const char *k )		{ return (type *)QGDict::take(k); }
    type *find( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    type *operator[]( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }

    void  insert( const QString& k, const type *d )
					{ QGDict::look(k,(Item)d,1); }
    void  replace( const QString& k, const type *d )
					{ QGDict::look(k,(Item)d,2); }
    bool  remove( const QString& k )	{ return QGDict::remove(k); }
    type *take( const QString& k )		{ return (type *)QGDict::take(k); }
    type *find( const QString& k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    type *operator[]( const QString& k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }

    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( Item d )	{ if ( del_item ) delete (type *)d; }
};


template<class type> class Q_EXPORT QDictIterator : public QGDictIterator
{
public:
    QDictIterator(const QDict<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QDictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type *current()   const   { return (type *)QGDictIterator::get(); }
    long currentKeyLong() const
			      { return QGDictIterator::getKeyLong(); }
    QString currentKey() const
			      { return QGDictIterator::getKey(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QDICT_H

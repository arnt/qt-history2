/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdict.h#29 $
**
** Definition of QDict template class
**
** Created : 920821
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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
    QDict(int size=17, bool caseSensitive=TRUE)
	: QGDict(size,StringKey,caseSensitive,FALSE) {}
    QDict( const QDict<type> &d ) : QGDict(d) {}
   ~QDict()				{ clear(); }
    QDict<type> &operator=(const QDict<type> &d)
			{ return (QDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }

    void  insert( const QString &k, const type *d )
					{ QGDict::look_string(k,(Item)d,1); }
    void  replace( const QString &k, const type *d )
					{ QGDict::look_string(k,(Item)d,2); }
    bool  remove( const QString &k )	{ return QGDict::remove_string(k); }
    type *take( const QString &k )	{ return (type *)QGDict::take_string(k); }
    type *find( const QString &k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_string(k,0,0); }
    type *operator[]( const QString &k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_string(k,0,0); }

    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( Item d )		{ if ( del_item ) delete (type *)d; }
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
    type   *current() const   { return (type *)QGDictIterator::get(); }
    QString currentKey() const{ return QGDictIterator::getKeyString(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QDICT_H

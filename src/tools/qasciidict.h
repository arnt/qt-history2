/****************************************************************************
** $Id: //depot/qt/main/src/tools/qasciidict.h#1 $
**
** Definition of QAsciiDict template class
**
** Created : 920821
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QASCIIDICT_H
#define QASCIIDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


template<class type> class Q_EXPORT QAsciiDict : public QGDict
{
public:
    QAsciiDict(int size=17, bool caseSensitive=TRUE, bool copyKeys=TRUE )
	: QGDict(size,AsciiKey,caseSensitive,copyKeys) {}
    QAsciiDict( const QAsciiDict<type> &d ) : QGDict(d) {}
   ~QAsciiDict()			{ clear(); }
    QAsciiDict<type> &operator=(const QAsciiDict<type> &d)
			{ return (QAsciiDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }

    void  insert( const char *k, const type *d )
					{ QGDict::look_ascii(k,(Item)d,1); }
    void  replace( const char *k, const type *d )
					{ QGDict::look_ascii(k,(Item)d,2); }
    bool  remove( const char *k )	{ return QGDict::remove_ascii(k); }
    type *take( const char *k )		{ return (type *)QGDict::take_ascii(k); }
    type *find( const char *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ascii(k,0,0); }
    type *operator[]( const char *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ascii(k,0,0); }

    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( Item d )		{ if ( del_item ) delete (type *)d; }
};


template<class type> class Q_EXPORT QAsciiDictIterator : public QGDictIterator
{
public:
    QAsciiDictIterator(const QAsciiDict<type> &d)
	: QGDictIterator((QGDict &)d) {}
   ~QAsciiDictIterator()      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type   *current() const   { return (type *)QGDictIterator::get(); }
    const char *currentKey() const { return QGDictIterator::getKeyAscii(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QASCIIDICT_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qarray.h#23 $
**
** Definition of QArray template/macro class
**
** Created : 930906
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

#ifndef QARRAY_H
#define QARRAY_H

#ifndef QT_H
#include "qgarray.h"
#endif // QT_H


template<class type> class QArray : public QGArray
{
protected:
    QArray( int, int ) : QGArray( 0, 0 ) {}
public:
    QArray()			{}
    QArray( int size ) : QGArray(size*sizeof(type)) {}
    QArray( const QArray<type> &a ) : QGArray(a) {}
   ~QArray()			{}
    QArray<type> &operator=(const QArray<type> &a)
				{ return (QArray<type>&)QGArray::assign(a); }
    type *data()    const	{ return (type *)QGArray::data(); }
    uint  nrefs()   const	{ return QGArray::nrefs(); }
    uint  size()    const	{ return QGArray::size()/sizeof(type); }
    bool  isEmpty() const	{ return QGArray::size() == 0; }
    bool  isNull()  const	{ return QGArray::data() == 0; }
    // ### bug bug bug.  this does not call constructors/destructors
    bool  resize( uint size )	{ return QGArray::resize(size*sizeof(type)); }
    bool  truncate( uint pos )	{ return QGArray::resize(pos*sizeof(type)); }
    // end if bug bug bug.  but the bug is in the constructur too.
    bool  fill( const type &d, int size=-1 )
	{ return QGArray::fill((char*)&d,size,sizeof(type) ); }
    void  detach()		{ QGArray::detach(); }
    QArray<type>   copy() const
	{ QArray<type> tmp; return tmp.duplicate(*this); }
    QArray<type>& assign( const QArray<type>& a )
	{ return (QArray<type>&)QGArray::assign(a); }
    QArray<type>& assign( const type *a, uint n )
	{ return (QArray<type>&)QGArray::assign((char*)a,n*sizeof(type)); }
    QArray<type>& duplicate( const QArray<type>& a )
	{ return (QArray<type>&)QGArray::duplicate(a); }
    QArray<type>& duplicate( const type *a, uint n )
	{ return (QArray<type>&)QGArray::duplicate((char*)a,n*sizeof(type)); }
    QArray<type>& setRawData( const type *a, uint n )
	{ return (QArray<type>&)QGArray::setRawData((char*)a,
						     n*sizeof(type)); }
    void resetRawData( const type *a, uint n )
	{ QGArray::resetRawData((char*)a,n*sizeof(type)); }
    int	 find( const type &d, uint i=0 ) const
	{ return QGArray::find((char*)&d,i,sizeof(type)); }
    int	 contains( const type &d ) const
	{ return QGArray::contains((char*)&d,sizeof(type)); }
    type& operator[]( int i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
    type& at( uint i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
	 operator const type*() const { return (const type *)QGArray::data(); }
    bool operator==( const QArray<type> &a ) const { return isEqual(a); }
    bool operator!=( const QArray<type> &a ) const { return !isEqual(a); }
};


#endif // QARRAY_H

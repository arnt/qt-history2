/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3INTDICT_H
#define Q3INTDICT_H

#include "Qt3Support/q3gdict.h"

template<class type>
class Q3IntDict
#ifdef qdoc
	: public Q3PtrCollection
#else
	: public Q3GDict
#endif
{
public:
    Q3IntDict(int size=17) : Q3GDict(size,IntKey,0,0) {}
    Q3IntDict( const Q3IntDict<type> &d ) : Q3GDict(d) {}
   ~Q3IntDict()				{ clear(); }
    Q3IntDict<type> &operator=(const Q3IntDict<type> &d)
			{ return (Q3IntDict<type>&)Q3GDict::operator=(d); }
    uint  count()   const		{ return Q3GDict::count(); }
    uint  size()    const		{ return Q3GDict::size(); }
    bool  isEmpty() const		{ return Q3GDict::count() == 0; }
    void  insert( long k, const type *d )
					{ Q3GDict::look_int(k,(Item)d,1); }
    void  replace( long k, const type *d )
					{ Q3GDict::look_int(k,(Item)d,2); }
    bool  remove( long k )		{ return Q3GDict::remove_int(k); }
    type *take( long k )		{ return (type*)Q3GDict::take_int(k); }
    type *find( long k ) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_int(k,0,0); }
    type *operator[]( long k ) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_int(k,0,0); }
    void  clear()			{ Q3GDict::clear(); }
    void  resize( uint n )		{ Q3GDict::resize(n); }
    void  statistics() const		{ Q3GDict::statistics(); }

#ifdef qdoc
protected:
    virtual QDataStream& read( QDataStream &, Q3PtrCollection::Item & );
    virtual QDataStream& write( QDataStream &, Q3PtrCollection::Item ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3IntDict<void>::deleteItem( Q3PtrCollection::Item )
{
}
#endif

template<class type> inline void Q3IntDict<type>::deleteItem( Q3PtrCollection::Item d )
{
    if ( del_item ) delete (type*)d;
}

template<class type>
class Q3IntDictIterator : public Q3GDictIterator
{
public:
    Q3IntDictIterator(const Q3IntDict<type> &d) :Q3GDictIterator((Q3GDict &)d) {}
   ~Q3IntDictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)Q3GDictIterator::toFirst(); }
    operator type *()  const  { return (type *)Q3GDictIterator::get(); }
    type *current()    const  { return (type *)Q3GDictIterator::get(); }
    long  currentKey() const  { return Q3GDictIterator::getKeyInt(); }
    type *operator()()	      { return (type *)Q3GDictIterator::operator()(); }
    type *operator++()	      { return (type *)Q3GDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GDictIterator::operator+=(j);}
};

#endif // QINTDICT_H

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

#ifndef Q3PTRDICT_H
#define Q3PTRDICT_H

#include "Qt3Support/q3gdict.h"

template<class type>
class Q3PtrDict
#ifdef qdoc
	: public Q3PtrCollection
#else
	: public Q3GDict
#endif
{
public:
    Q3PtrDict(int size=17) : Q3GDict(size,PtrKey,0,0) {}
    Q3PtrDict( const Q3PtrDict<type> &d ) : Q3GDict(d) {}
   ~Q3PtrDict()				{ clear(); }
    Q3PtrDict<type> &operator=(const Q3PtrDict<type> &d)
			{ return (Q3PtrDict<type>&)Q3GDict::operator=(d); }
    uint  count()   const		{ return Q3GDict::count(); }
    uint  size()    const		{ return Q3GDict::size(); }
    bool  isEmpty() const		{ return Q3GDict::count() == 0; }
    void  insert( void *k, const type *d )
					{ Q3GDict::look_ptr(k,(Item)d,1); }
    void  replace( void *k, const type *d )
					{ Q3GDict::look_ptr(k,(Item)d,2); }
    bool  remove( void *k )		{ return Q3GDict::remove_ptr(k); }
    type *take( void *k )		{ return (type*)Q3GDict::take_ptr(k); }
    type *find( void *k ) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_ptr(k,0,0); }
    type *operator[]( void *k ) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_ptr(k,0,0); }
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
template<> inline void Q3PtrDict<void>::deleteItem( Q3PtrCollection::Item )
{
}
#endif

template<class type>
inline void Q3PtrDict<type>::deleteItem( Q3PtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

template<class type>
class Q3PtrDictIterator : public Q3GDictIterator
{
public:
    Q3PtrDictIterator(const Q3PtrDict<type> &d) :Q3GDictIterator((Q3GDict &)d) {}
   ~Q3PtrDictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)Q3GDictIterator::toFirst(); }
    operator type *()  const  { return (type *)Q3GDictIterator::get(); }
    type *current()    const  { return (type *)Q3GDictIterator::get(); }
    void *currentKey() const  { return Q3GDictIterator::getKeyPtr(); }
    type *operator()()	      { return (type *)Q3GDictIterator::operator()(); }
    type *operator++()	      { return (type *)Q3GDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GDictIterator::operator+=(j);}
};

#endif // Q3PTRDICT_H

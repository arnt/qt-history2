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

#ifndef Q3DICT_H
#define Q3DICT_H

#include "Qt3Support/q3gdict.h"

template<class type>
class Q3Dict
#ifdef Q_QDOC
	: public Q3PtrCollection
#else
	: public Q3GDict
#endif
{
public:
    Q3Dict(int size = 17, bool caseSensitive = true)
	: Q3GDict(size, StringKey, caseSensitive, false) { }
    Q3Dict(const Q3Dict<type> &d) : Q3GDict(d) { }
    ~Q3Dict()				{ clear(); }
    Q3Dict<type> &operator=(const Q3Dict<type> &d)
			{ return (Q3Dict<type>&)Q3GDict::operator=(d); }
    uint  count()   const		{ return Q3GDict::count(); }
    uint  size()    const		{ return Q3GDict::size(); }
    bool  isEmpty() const		{ return Q3GDict::count() == 0; }

    void  insert(const QString &k, const type *d)
					{ Q3GDict::look_string(k,(Item)d,1); }
    void  replace(const QString &k, const type *d)
					{ Q3GDict::look_string(k,(Item)d,2); }
    bool  remove(const QString &k)	{ return Q3GDict::remove_string(k); }
    type *take(const QString &k)	{ return (type *)Q3GDict::take_string(k); }
    type *find(const QString &k) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_string(k,0,0); }
    type *operator[](const QString &k) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_string(k,0,0); }

    void  clear()			{ Q3GDict::clear(); }
    void  resize(uint n)		{ Q3GDict::resize(n); }
    void  statistics() const		{ Q3GDict::statistics(); }

#ifdef Q_QDOC
protected:
    virtual QDataStream& read(QDataStream &, Q3PtrCollection::Item &);
    virtual QDataStream& write(QDataStream &, Q3PtrCollection::Item) const;
#endif

private:
	void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3Dict<void>::deleteItem(Item)
{
}
#endif

template<class type> inline void Q3Dict<type>::deleteItem(Q3PtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

template<class type>
class Q3DictIterator : public Q3GDictIterator
{
public:
    Q3DictIterator(const Q3Dict<type> &d) : Q3GDictIterator((Q3GDict &)d) { }
    ~Q3DictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)Q3GDictIterator::toFirst(); }
    operator type *() const   { return (type *)Q3GDictIterator::get(); }
    type *operator*()         { return (type *)Q3GDictIterator::get(); }
    type   *current() const   { return (type *)Q3GDictIterator::get(); }
    QString currentKey() const{ return Q3GDictIterator::getKeyString(); }
    type *operator()()	      { return (type *)Q3GDictIterator::operator()(); }
    type *operator++()	      { return (type *)Q3GDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GDictIterator::operator+=(j); }
};

#endif // Q3DICT_H

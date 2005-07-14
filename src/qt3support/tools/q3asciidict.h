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

#ifndef Q3ASCIIDICT_H
#define Q3ASCIIDICT_H

#include "Qt3Support/q3gdict.h"

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3AsciiDict
#ifdef qdoc
	: public Q3PtrCollection
#else
	: public Q3GDict
#endif
{
public:
    Q3AsciiDict(int size=17, bool caseSensitive=true, bool copyKeys=true)
	: Q3GDict(size,AsciiKey,caseSensitive,copyKeys) {}
    Q3AsciiDict(const Q3AsciiDict<type> &d) : Q3GDict(d) {}
   ~Q3AsciiDict()			{ clear(); }
    Q3AsciiDict<type> &operator=(const Q3AsciiDict<type> &d)
			{ return (Q3AsciiDict<type>&)Q3GDict::operator=(d); }
    uint  count()   const		{ return Q3GDict::count(); }
    uint  size()    const		{ return Q3GDict::size(); }
    bool  isEmpty() const		{ return Q3GDict::count() == 0; }

    void  insert(const char *k, const type *d)
					{ Q3GDict::look_ascii(k,(Item)d,1); }
    void  replace(const char *k, const type *d)
					{ Q3GDict::look_ascii(k,(Item)d,2); }
    bool  remove(const char *k)	{ return Q3GDict::remove_ascii(k); }
    type *take(const char *k)		{ return (type *)Q3GDict::take_ascii(k); }
    type *find(const char *k) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_ascii(k,0,0); }
    type *operator[](const char *k) const
		{ return (type *)((Q3GDict*)this)->Q3GDict::look_ascii(k,0,0); }

    void  clear()			{ Q3GDict::clear(); }
    void  resize(uint n)		{ Q3GDict::resize(n); }
    void  statistics() const		{ Q3GDict::statistics(); }

#ifdef qdoc
protected:
    virtual QDataStream& read(QDataStream &, Q3PtrCollection::Item &);
    virtual QDataStream& write(QDataStream &, Q3PtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3AsciiDict<void>::deleteItem(Q3PtrCollection::Item)
{
}
#endif

template<class type> inline void Q3AsciiDict<type>::deleteItem(Q3PtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

template<class type>
class Q3AsciiDictIterator : public Q3GDictIterator
{
public:
    Q3AsciiDictIterator(const Q3AsciiDict<type> &d)
	: Q3GDictIterator((Q3GDict &)d) {}
   ~Q3AsciiDictIterator()      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)Q3GDictIterator::toFirst(); }
    operator type *() const   { return (type *)Q3GDictIterator::get(); }
    type   *current() const   { return (type *)Q3GDictIterator::get(); }
    const char *currentKey() const { return Q3GDictIterator::getKeyAscii(); }
    type *operator()()	      { return (type *)Q3GDictIterator::operator()(); }
    type *operator++()	      { return (type *)Q3GDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GDictIterator::operator+=(j);}
};

#endif // Q3ASCIIDICT_H

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

#ifndef QASCIIDICT_H
#define QASCIIDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H

template<class type>
class QAsciiDict
#ifdef qdoc
        : public QPtrCollection
#else
        : public QGDict
#endif
{
public:
    QAsciiDict(int size=17, bool caseSensitive=true, bool copyKeys=true)
        : QGDict(size,AsciiKey,caseSensitive,copyKeys) {}
    QAsciiDict(const QAsciiDict<type> &d) : QGDict(d) {}
   ~QAsciiDict()                        { clear(); }
    QAsciiDict<type> &operator=(const QAsciiDict<type> &d)
                        { return static_cast<QAsciiDict<type> &>(QGDict::operator=(d)); }
    uint  count()   const                { return QGDict::count(); }
    uint  size()    const                { return QGDict::size(); }
    bool  isEmpty() const                { return QGDict::count() == 0; }

    void  insert(const char *k, const type *d)
                                        { QGDict::look_ascii(k,Item(d),1); }
    void  replace(const char *k, const type *d)
                                        { QGDict::look_ascii(k,Item(d),2); }
    bool  remove(const char *k)        { return QGDict::remove_ascii(k); }
    type *take(const char *k)                { return static_cast<type *>(QGDict::take_ascii(k)); }
    type *find(const char *k) const
        { return static_cast<type *>(const_cast<QAsciiDict *>(this)->QGDict::look_ascii(k,0,0)); }
    type *operator[](const char *k) const
                { return static_cast<type *>(const_cast<QAsciiDict *>(this)->QGDict::look_ascii(k,0,0)); }

    void  clear()                        { QGDict::clear(); }
    void  resize(uint n)                { QGDict::resize(n); }
    void  statistics() const                { QGDict::statistics(); }

#ifdef qdoc
protected:
    virtual QDataStream& read(QDataStream &, QPtrCollection::Item &);
    virtual QDataStream& write(QDataStream &, QPtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QAsciiDict<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QAsciiDict<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type *>(d);
}

template<class type>
class QAsciiDictIterator : public QGDictIterator
{
public:
    QAsciiDictIterator(const QAsciiDict<type> &d)
        : QGDictIterator(d) {}
   ~QAsciiDictIterator()      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()              { return static_cast<type *>(QGDictIterator::toFirst()); }
    operator type *() const   { return static_cast<type *>(QGDictIterator::get()); }
    type   *current() const   { return static_cast<type *>(QGDictIterator::get()); }
    const char *currentKey() const { return QGDictIterator::getKeyAscii(); }
    type *operator()()              { return static_cast<type *>(QGDictIterator::operator()()); }
    type *operator++()              { return static_cast<type *>(QGDictIterator::operator++()); }
    type *operator+=(uint j)  { return static_cast<type *>(QGDictIterator::operator+=(j));}
};

#endif // QASCIIDICT_H

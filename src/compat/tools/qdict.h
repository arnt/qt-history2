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

#ifndef QDICT_H
#define QDICT_H

#ifdef QT_COMPAT
#ifndef QT_H
#include "qgdict.h"
#endif // QT_H

template<class type>
class QDict
#ifdef qdoc
        : public QPtrCollection
#else
        : public QGDict
#endif
{
public:
    QDict(int size = 17, bool caseSensitive = true)
        : QGDict(size, StringKey, caseSensitive, false) { }
    QDict(const QDict<type> &d) : QGDict(d) { }
    ~QDict()                                { clear(); }
    QDict<type> &operator=(const QDict<type> &d)
                        { return static_cast<QDict<type> &>(QGDict::operator=(d)); }
    uint  count()   const                { return QGDict::count(); }
    uint  size()    const                { return QGDict::size(); }
    bool  isEmpty() const                { return QGDict::count() == 0; }

    void  insert(const QString &k, const type *d)
                                        { QGDict::look_string(k,Item(d),1); }
    void  replace(const QString &k, const type *d)
                                        { QGDict::look_string(k,Item(d),2); }
    bool  remove(const QString &k)        { return QGDict::remove_string(k); }
    type *take(const QString &k)        { return static_cast<type *>(QGDict::take_string(k)); }
    type *find(const QString &k) const
                { return static_cast<type *>(QGDict::look_string(k,0,0)); }
    type *operator[](const QString &k) const
                { return static_cast<type *>(QGDict::look_string(k,0,0)); }

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
template<> inline void QDict<void>::deleteItem(Item)
{
}
#endif

template<class type> inline void QDict<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type *>(d);
}

template<class type>
class QDictIterator : public QGDictIterator
{
public:
    QDictIterator(const QDict<type> &d) : QGDictIterator(d) { }
    ~QDictIterator()              {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()              { return static_cast<type *>(QGDictIterator::toFirst()); }
    operator type *() const   { return static_cast<type *>(QGDictIterator::get()); }
    type *operator*()         { return static_cast<type *>(QGDictIterator::get()); }
    type   *current() const   { return static_cast<type *>(QGDictIterator::get()); }
    QString currentKey() const{ return QGDictIterator::getKeyString(); }
    type *operator()()              { return static_cast<type *>(QGDictIterator::operator()()); }
    type *operator++()              { return static_cast<type *>(QGDictIterator::operator++()); }
    type *operator+=(uint j)  { return static_cast<type *>(QGDictIterator::operator+=(j)); }
};
#endif //Q_COMPAT

#endif // QDICT_H

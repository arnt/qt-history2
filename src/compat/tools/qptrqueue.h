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

#ifndef QPTRQUEUE_H
#define QPTRQUEUE_H

#include "qglist.h"

template<class type>
class QPtrQueue : protected QGList
{
public:
    QPtrQueue()                                {}
    QPtrQueue(const QPtrQueue<type> &q) : QGList(q) {}
    ~QPtrQueue()                        { clear(); }
    QPtrQueue<type>& operator=(const QPtrQueue<type> &q)
        { return static_cast<QPtrQueue<type> &>(QGList::operator=(q)); }
    bool  autoDelete() const                { return QPtrCollection::autoDelete(); }
    void  setAutoDelete(bool del)        { QPtrCollection::setAutoDelete(del); }
    uint  count()   const                { return QGList::count(); }
    bool  isEmpty() const                { return QGList::count() == 0; }
    void  enqueue(const type *d)        { QGList::append(Item(d)); }
    type *dequeue()                        { return static_cast<type *>(QGList::takeFirst());}
    bool  remove()                        { return QGList::removeFirst(); }
    void  clear()                        { QGList::clear(); }
    type *head()    const                { return static_cast<type *>(QGList::cfirst()); }
          operator type *() const        { return static_cast<type *>(QGList::cfirst()); }
    type *current() const                { return static_cast<type *>(QGList::cfirst()); }

#ifdef qdoc
protected:
    virtual QDataStream& read(QDataStream&, QPtrCollection::Item&);
    virtual QDataStream& write(QDataStream&, QPtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrQueue<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QPtrQueue<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type *>(d);
}

#endif // QPTRQUEUE_H

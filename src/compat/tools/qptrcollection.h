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

#ifndef QPTRCOLLECTION_H
#define QPTRCOLLECTION_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


class QGVector;
class QGList;
class QGDict;


class Q_COMPAT_EXPORT QPtrCollection                        // inherited by all collections
{
public:
    bool autoDelete()        const               { return del_item; }
    void setAutoDelete(bool enable)  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;                        // delete all objects

    typedef void *Item;                                // generic collection item

protected:
    QPtrCollection() { del_item = false; }                // no deletion of objects
    QPtrCollection(const QPtrCollection &) { del_item = false; }
    virtual ~QPtrCollection() {}

    bool del_item;                                // default false

    virtual Item     newItem(Item);                // create object
    virtual void     deleteItem(Item) = 0;        // delete object
};

#endif // QPTRCOLLECTION_H

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

#ifndef Q3PTRCOLLECTION_H
#define Q3PTRCOLLECTION_H

#include "qglobal.h"

class Q3GVector;
class Q3GList;
class Q3GDict;

class Q_COMPAT_EXPORT Q3PtrCollection			// inherited by all collections
{
public:
    bool autoDelete()	const	       { return del_item; }
    void setAutoDelete(bool enable)  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;			// delete all objects

    typedef void *Item;				// generic collection item

protected:
    Q3PtrCollection() { del_item = false; }		// no deletion of objects
    Q3PtrCollection(const Q3PtrCollection &) { del_item = false; }
    virtual ~Q3PtrCollection() {}

    bool del_item;				// default false

    virtual Item     newItem(Item);		// create object
    virtual void     deleteItem(Item) = 0;	// delete object
};


#ifdef QT_COMPAT
#define QCollection QPtrCollection
#endif

#endif // Q3PTRCOLLECTION_H

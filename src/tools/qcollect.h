/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollect.h#9 $
**
** Definition of base class for all collection classes
**
** Created : 920629
**
** Copyright (C) 1992-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCOLLECT_H
#define QCOLLECT_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


typedef void *GCI;				// generic collection item
typedef int (*GCF)(GCI,void*);			// generic collection function

class QGVector;
class QGList;
class QGDict;


class QCollection				// inherited by all collections
{
public:
    bool autoDelete()	const	       { return del_item; }
    void setAutoDelete( bool enable )  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;			// delete all objects

protected:
    QCollection() { del_item = FALSE; }		// no deletion of objects
    virtual ~QCollection() {}

    bool del_item;				// default FALSE

    virtual GCI	     newItem( GCI );		// create object
    virtual void     deleteItem( GCI );		// delete object
};


#endif // QCOLLECT_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollection.h#7 $
**
** Definition of base class for all collection classes
**
** Created : 920629
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCOLLECT_H
#define QCOLLECT_H

#include "qglobal.h"


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

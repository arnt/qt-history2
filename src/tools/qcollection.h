/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollection.h#15 $
**
** Definition of base class for all collection classes
**
** Created : 920629
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCOLLECTION_H
#define QCOLLECTION_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


typedef void *GCI;				// generic collection item
typedef int (*GCF)(GCI,void*);			// generic collection function

class QGVector;
class QGList;
class QGDict;


class Q_EXPORT QCollection			// inherited by all collections
{
public:
    bool autoDelete()	const	       { return del_item; }
    void setAutoDelete( bool enable )  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;			// delete all objects

protected:
    QCollection() { del_item = FALSE; }		// no deletion of objects
    QCollection(const QCollection &) { del_item = FALSE; }
    virtual ~QCollection() {}

    bool del_item;				// default FALSE

    virtual GCI	     newItem( GCI );		// create object
    virtual void     deleteItem( GCI );		// delete object
};


#endif // QCOLLECTION_H

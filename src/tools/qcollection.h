/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollection.h#18 $
**
** Definition of base class for all collection classes
**
** Created : 920629
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCOLLECTION_H
#define QCOLLECTION_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


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

    typedef void *Item;				// generic collection item

protected:
    QCollection() { del_item = FALSE; }		// no deletion of objects
    QCollection(const QCollection &) { del_item = FALSE; }
    virtual ~QCollection() {}

    bool del_item;				// default FALSE

    virtual Item	     newItem( Item );		// create object
    virtual void     deleteItem( Item );		// delete object
};


#endif // QCOLLECTION_H

/****************************************************************************
** $Id: $
**
** Implementation of extra QUcom classes
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qucomextra_p.h"
#include <qvariant.h>


#ifndef QT_NO_VARIANT
// 6dc75d58-a1d9-4417-b591-d45c63a3a4ea
const QUuid TID_QUType_QVariant( 0x6dc75d58, 0xa1d9, 0x4417, 0xb5, 0x91, 0xd4, 0x5c, 0x63, 0xa3, 0xa4, 0xea );
static QUType_QVariant static_QUType_QVariant;
QUType_QVariant *pQUType_QVariant = &static_QUType_QVariant;
const QUuid *QUType_QVariant::uuid() const { return &TID_QUType_QVariant; }
const char *QUType_QVariant::desc() const { return "QVariant"; }

void QUType_QVariant::set( QUObject *o, const QVariant& v )
{
    o->payload.ptr = new QVariant( v );
    o->type = this;
}

QVariant &QUType_QVariant::get( QUObject * o )
{
    return *(QVariant*)o->payload.ptr;
}

bool QUType_QVariant::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_QVariant::canConvertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

bool QUType_QVariant::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_QVariant::convertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

void QUType_QVariant::clear( QUObject *o )
{
    delete (QVariant*)o->payload.ptr;
    o->payload.ptr = 0;
}

int QUType_QVariant::serializeTo( QUObject *, QUBuffer * )
{
    return 0;
}

int QUType_QVariant::serializeFrom( QUObject *, QUBuffer * )
{
    return 0;
}


#endif


/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpropertyinfo.cpp $
**
** Implementation of the QPropertyInfo class
**
** Created : 991212
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpropertyinfo.h"
#include "qmetaobject.h"

QPropertyInfo::QPropertyInfo( QMetaProperty* m )
    : meta( m )
{
}

QPropertyInfo::QPropertyInfo( const QPropertyInfo& other )
{
    meta = other.meta;
}

const char* QPropertyInfo::name() const
{
    if ( !meta )
	return 0;
    return meta->name;
}

const char* QPropertyInfo::type() const
{
    if ( !meta )
	return 0;
    return meta->type;
}

bool QPropertyInfo::isEnumType() const
{
    if ( !meta )
	return FALSE;
    return meta->enumType != 0;
}

QStrList QPropertyInfo::enumNames() const
{
     QStrList l( FALSE );
     if ( isEnumType() ) {
	 for( uint i = 0; i < meta->enumType->count; ++i )
	     l.append( meta->enumType->items[i].name );
     }
     return l;
}

bool QPropertyInfo::readable() const
{
    if ( !meta )
	return FALSE;
    return meta->readable();
}

bool QPropertyInfo::writeable() const
{
    if ( !meta )
	return FALSE;
    return meta->writeable();
}


/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty.cpp#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qwsproperty.h"
#include "qwscommand.h"
#include "qws.h"

#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
}

int QWSPropertyManager::atom( const QString &name )
{
    if ( atoms.contains( name ) )
	return atoms[ name ];
    return -1;
}

bool QWSPropertyManager::setProperty( int winId, int property, int mode, const QByteArray &data )
{
    PropertyKey k( winId, property );
    if ( !properties.contains( k ) )
	return FALSE;

    Mode m = (Mode)mode;
    switch ( m ) {
    case PropReplace:
	properties[ k ] = data.copy();
	break;
    case PropPrepend: {
	QByteArray oldData = properties[ k ];
	QByteArray newData = data.copy();
	oldData.resize( oldData.size() + newData.size() );
	memcpy( oldData.data() + oldData.size(), newData.data(), newData.size() );
	properties[ k ] = oldData;
    } break;
    case PropAppend: {
	QByteArray oldData = properties[ k ];
	QByteArray newData = data.copy();
	newData.resize( oldData.size() + newData.size() );
	memcpy( newData.data() + newData.size(), oldData.data(), oldData.size() );
	properties[ k ] = newData;
    } break;
    }

    return TRUE;
}

bool QWSPropertyManager::hasProperty( int winId, int property )
{
    PropertyKey k( winId, property );
    return (bool)properties.contains( k );
}

bool QWSPropertyManager::removeProperty( int winId, int property )
{
    PropertyKey k( winId, property );
    if ( !properties.contains( k ) )
	return FALSE;

    properties.remove( k );
    return TRUE;
}

bool QWSPropertyManager::addProperty( int winId, int property )
{
    PropertyKey k( winId, property );
    if ( properties.contains( k ) )
	return FALSE;

    properties[ k ] = QByteArray();
    return TRUE;
}


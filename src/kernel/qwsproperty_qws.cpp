/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty_qws.cpp#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwsproperty_qws.h"

#ifndef QT_NO_QWS_PROPERTIES
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"

#include <stdlib.h>
#include <stdio.h>

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    properties.setAutoDelete( TRUE );
}


//##### Memory leak? We do not delete prop->data

//#define QWS_PROPERTY_DEBUG


bool QWSPropertyManager::setProperty( int winId, int property, int mode, const char *data, int len )
{
    if ( !hasProperty( winId, property ) )
	return FALSE;
    char *key = createKey( winId, property );

    switch ( mode ) {
    case PropReplace: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropReplace" );
#endif
	char *d = new char[len]; //###Must make sure this is deleted
	memcpy(d, data, len );
	Property *prop = new Property;
	prop->len = len;
	prop->data = d;
	properties.replace( key, prop );
    } break;
    case PropAppend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropAppend" );
#endif
	Property *orig = properties[ key ];
	int origLen = 0;
	if ( orig )
	    origLen = orig->len;
	char *d = new char[ len + origLen ];
	Property *prop = new Property;
	prop->len = len + origLen;
	if ( orig )
	    memcpy( &d[ 0 ], orig->data, origLen );
	memcpy( &d[ origLen ], data, len );
	prop->data = d;
	properties.replace( key, prop );
    } break;
    case PropPrepend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropPrepend" );
#endif
	Property *orig = properties[ key ];
	int origLen = 0;
	if ( orig )
	    origLen = orig->len;
	char *d = new char[ len + origLen ];
	Property *prop = new Property;
	prop->len = len + origLen;
	memcpy( &d[ 0 ], data, len );
	if ( orig )
	    memcpy( &d[ len ], orig->data, origLen );
	prop->data = d;
	properties.replace( key, prop );
    } break;
    }
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::setProperty: %d %d (%s) to %s", winId, property, key,
	    properties.find( key )->data );
#endif
    delete [] key;

    return TRUE;
}

bool QWSPropertyManager::hasProperty( int winId, int property )
{
    char *key = createKey( winId, property );
    bool b = (bool)properties.find( key );
    delete [] key;
    return b;
}

bool QWSPropertyManager::removeProperty( int winId, int property )
{
    char *key = createKey( winId, property );
    if ( !properties.find( key ) ) {
	delete [] key;
	return FALSE;
    }
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::removeProperty %d %d (%s)", winId, property, key );
#endif    
    properties.remove( key );
    delete [] key;
    return TRUE;
}

bool QWSPropertyManager::addProperty( int winId, int property )
{
    char *key = createKey( winId, property );
    if ( properties.find( key ) )
	return FALSE;

    Property *prop = new Property;
    prop->len = -1;
    prop->data = 0;
    properties.insert( key, prop );
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::addProperty: %d %d (%s)", winId, property, key );
#endif
    return TRUE;
}

bool QWSPropertyManager::getProperty( int winId, int property, char *&data, int &len )
{
    char *key = createKey( winId, property );
    if ( !properties.find( key ) ) {
	delete [] key;
	data = 0;
	len = -1;
	return FALSE;
    }

    Property *prop = properties[ key ];
    len = prop->len;
    data = prop->data;
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::getProperty: %d %d (%s) %d", winId, property, 
	    key, len );
    if ( len > 0 ) {
	for ( int i = 0; i < len; i++ )
	    printf( "%c",data[i] );
	printf( "\n" );
    }
#endif    
    delete [] key;

    return TRUE;
}

char *QWSPropertyManager::createKey( int winId, int property ) const
{
    char *key = new char[ 21 ];
    sprintf( key, "%010d%010d", winId, property );
    key[ 20 ] = '\0';
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::createKey: %s", key );
#endif
    return key;
}
#endif //QT_NO_QWS_PROPERTIES

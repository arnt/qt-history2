/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include "qwsproperty.h"
#include "qwscommand.h"
#include "qws.h"

#include <stdlib.h>
#include <stdio.h>

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    properties.setAutoDelete( true );
}

bool QWSPropertyManager::setProperty( int winId, int property, int mode, const char *data, int len )
{
    if ( !hasProperty( winId, property ) )
	return false;
    char *key = createKey( winId, property );

    switch ( mode ) {
    case PropReplace: {
	qDebug( "PropReplace" );
	char *d = qstrdup( data );
	Property *prop = new Property;
	prop->len = len;
	prop->data = d;
	properties.replace( key, prop );
    } break;
    case PropAppend: {
	qDebug( "PropAppend" );
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
	qDebug( "PropPrepend" );
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

    qDebug( "QWSPropertyManager::setProperty: %d %d (%s) to %s", winId, property, key,
	    properties.find( key )->data );

    delete [] key;

    return true;
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
	return false;
    }

    qDebug( "QWSPropertyManager::removeProperty %d %d (%s)", winId, property, key );
    properties.remove( key );
    delete [] key;
    return true;
}

bool QWSPropertyManager::addProperty( int winId, int property )
{
    char *key = createKey( winId, property );
    if ( properties.find( key ) )
	return false;

    Property *prop = new Property;
    prop->len = -1;
    prop->data = 0;
    properties.insert( key, prop );
    qDebug( "QWSPropertyManager::addProperty: %d %d (%s)", winId, property, key );
    return true;
}

bool QWSPropertyManager::getProperty( int winId, int property, char *&data, int &len )
{
    char *key = createKey( winId, property );
    if ( !properties.find( key ) ) {
	delete [] key;
	data = 0;
	len = -1;
	return false;
    }

    Property *prop = properties[ key ];
    len = prop->len;
    data = prop->data;
    delete [] key;

    return true;
}

char *QWSPropertyManager::createKey( int winId, int property ) const
{
    char *key = new char[ 21 ];
    sprintf( key, "%010d%010d", winId, property );
    key[ 20 ] = '\0';
    qDebug( "QWSPropertyManager::createKey: %s", key );

    return key;
}

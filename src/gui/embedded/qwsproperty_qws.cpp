/****************************************************************************
**
** Implementation of Qt/FB central server.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwsproperty_qws.h"

#ifndef QT_NO_QWS_PROPERTIES
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"
#include "qhash.h"
#include "qalgorithms.h"

#include <stdlib.h>
#include <stdio.h>

class QWSPropertyManager::Data {
public:
    struct Property {
	~Property() { if ( data ) delete [] data; }
	int len;
	char *data;
    };
    Property* find( int winId, int property )
    {
	QHash<int,Property*>* wp = properties.value(winId);
	if ( !wp ) return 0;
	Property* prop = wp->value(property);
	return prop;
    }

    QHash<int,QHash<int,Property*>*> properties;
};

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    d = new Data;
}

QWSPropertyManager::~QWSPropertyManager()
{

    qDeleteAll(d->properties);
    delete d;
}


//#define QWS_PROPERTY_DEBUG


bool QWSPropertyManager::setProperty( int winId, int property, int mode, const char *data, int len )
{
    Data::Property* prop = d->find(winId,property);
    if ( !prop ) return FALSE;

    switch ( mode ) {
    case PropReplace: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropReplace" );
#endif
	delete [] prop->data;
	char *nd = new char[len]; //###Must make sure this is deleted
	memcpy(nd, data, len );
	prop->len = len;
	prop->data = nd;
    } break;
    case PropAppend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropAppend" );
#endif
	int origLen = prop->len;
	char *nd = new char[ len + origLen ];
	memcpy( nd, prop->data, origLen );
	memcpy( nd+origLen, data, len );
	delete [] prop->data;
	prop->len = len + origLen;
	prop->data = nd;
    } break;
    case PropPrepend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropPrepend" );
#endif
	int origLen = origLen = prop->len;
	char *nd = new char[ len + origLen ];
	memcpy( nd, data, len );
	memcpy( nd+len, prop->data, origLen );
	delete [] prop->data;
	prop->len = len + origLen;
	prop->data = nd;
    } break;
    }
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::setProperty: %d %d (%s) to %s", winId, property, key,
	    d->properties.find( key )->data );
#endif

    return TRUE;
}

bool QWSPropertyManager::hasProperty( int winId, int property )
{
    Data::Property* prop = d->find(winId,property);
    return !!prop;
}

bool QWSPropertyManager::removeProperty( int winId, int property )
{
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::removeProperty %d %d (%s)", winId, property, key );
#endif
    QHash<int,Data::Property*>* wp = d->properties.value(winId);
    if (!wp) return false;
    Data::Property* prop = wp->value(property);
    if (!prop) return false;
    delete wp->take(property);
    if (wp->count() == 0)
	delete d->properties.take(winId);
    return TRUE;
}

bool QWSPropertyManager::addProperty( int winId, int property )
{
    QHash<int,Data::Property*>* wp = d->properties.value(winId);
    if (!wp) {
	d->properties.insert(winId,wp = new QHash<int,Data::Property*>);
    }
    Data::Property* prop = wp->value(property);
    if (prop) return false;
    wp->insert(property, prop = new Data::Property);

    prop->len = -1;
    prop->data = 0;
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::addProperty: %d %d (%s)", winId, property, key );
#endif
    return TRUE;
}

bool QWSPropertyManager::getProperty( int winId, int property, char *&data, int &len )
{
    Data::Property* prop = d->find(winId,property);
    if ( !prop ) {
	data = 0;
	len = -1;
	return FALSE;
    }

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

    return TRUE;
}

bool QWSPropertyManager::removeProperties( int winId )
{
    QHash<int,Data::Property*> *wp = d->properties.take(winId);

    if (wp) {
	qDeleteAll(*wp);
	delete wp;
    }
    return wp != 0;
}

#endif //QT_NO_QWS_PROPERTIES

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

/*********************************************************************
 *
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

/*
  Command character: S

  The format of a set property command is:
  ABCDdata

  A .... winId (4 bytes - hex)
  B .... property (4 bytes - hex)
  C .... mode (4 bytes - hex)
  D .... length of data (4 bytes - hex)
  data .... D bytes of data
*/

QWSSetPropertyCommand::QWSSetPropertyCommand( QWSServer *s, QWSClient *c )
    : QWSCommand( s, c )
{
}

QWSSetPropertyCommand::~QWSSetPropertyCommand()
{
}

void QWSSetPropertyCommand::readData()
{
    winId = qws_read_uint( client );
    property = qws_read_uint( client );
    mode = qws_read_uint( client );
    int len = qws_read_uint( client );

    if ( len > 0 ) {
	data.resize( len );
	client->readBlock( data.data(), len );
    }
    qDebug( "QWSSetPropertyCommand::readData: %d %d %d %d %s",
	    winId, property, mode, len, data.data() );
}

void QWSSetPropertyCommand::execute()
{
    if ( server->properties()->setProperty( winId, property, mode, data ) )
	qDebug( "set property successful" );
    else
	qDebug( "setting property failed" );
}

/*********************************************************************
 *
 * Class: QWSAddPropertyCommand
 *
 *********************************************************************/

/*
  Command character: A

  The format of a add property command is:
  AB

  A .... winId (4 bytes - hex)
  B .... property (4 bytes - hex)
*/

QWSAddPropertyCommand::QWSAddPropertyCommand( QWSServer *s, QWSClient *c )
    : QWSCommand( s, c )
{
}

QWSAddPropertyCommand::~QWSAddPropertyCommand()
{
}

void QWSAddPropertyCommand::readData()
{
    winId = qws_read_uint( client );
    property = qws_read_uint( client );
    qDebug( "QWSAddPropertyCommand::readData: %d %d", winId, property );
}

void QWSAddPropertyCommand::execute()
{
    if ( server->properties()->addProperty( winId, property ) )
	qDebug( "add property successful" );
    else
	qDebug( "adding property failed" );
}

/*********************************************************************
 *
 * Class: QWSRemovePropertyCommand
 *
 *********************************************************************/

/*
  Command character: R

  The format of a add property command is:
  AB

  A .... winId (4 bytes - hex)
  B .... property (4 bytes - hex)
*/

QWSRemovePropertyCommand::QWSRemovePropertyCommand( QWSServer *s, QWSClient *c )
    : QWSCommand( s, c )
{
}

QWSRemovePropertyCommand::~QWSRemovePropertyCommand()
{
}

void QWSRemovePropertyCommand::readData()
{
    winId = qws_read_uint( client );
    property = qws_read_uint( client );
    qDebug( "QWSRemovePropertyCommand::readData: %d %d", winId, property );
}

void QWSRemovePropertyCommand::execute()
{
    if ( server->properties()->removeProperty( winId, property ) )
	qDebug( "remove property successful" );
    else
	qDebug( "removing property failed" );
}

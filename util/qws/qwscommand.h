/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwscommand.cpp#3 $
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

#ifndef QWSCOMMAND_H
#define QWSCOMMAND_H

#include "qwsutils.h"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/

static void qws_write_command( QSocket *socket, int type, 
			       char *simpleData, int simpleLen, char *rawData, int rawLen )
{
    qws_write_uint( socket, type );
    qws_write_uint( socket, rawLen == -1 ? 0 : rawLen );
    if ( simpleData && simpleLen )
	socket->writeBlock( simpleData, simpleLen );
    if ( rawLen && rawData )
	socket->writeBlock( rawData, rawLen );
}

static bool qws_read_command( QSocket *socket, char *simpleData, int &simpleLen,
			      char *&rawData, int &rawLen,
			      int &bytesRead )
{
    if ( rawLen == -1 ) {
	if ( (uint)socket->bytesAvailable() < sizeof( rawLen ) )
	    return FALSE;
	rawLen = qws_read_uint( socket );
    }

    if ( !bytesRead ) {
	if ( simpleLen ) {
	    if ( socket->bytesAvailable() < simpleLen )
		return FALSE;
	    bytesRead = socket->readBlock( simpleData, simpleLen );
	} else
	    bytesRead = 1; // hack!
    }
	
    if ( bytesRead ) {
	if ( !rawLen )
	    return TRUE;
	if ( socket->bytesAvailable() < rawLen )
	    return FALSE;
	rawData = new char[ rawLen ];
	bytesRead += socket->readBlock( rawData, rawLen );
	return TRUE;
    }
    return FALSE;
}

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/

struct QWSCommand 
{
    // ctor - dtor
    QWSCommand( int t, int len, char *ptr ) : type( t ),
	simpleLen( len ), rawLen( -1 ), rawData( 0 ), 
	simpleDataPtr( ptr ), bytesRead( 0 ) {}
    ~QWSCommand() { delete rawData; }

    enum Type {
	Unknown = 0,
	Create, 
	Destroy,
	Region,
	SetProperty,
	AddProperty,
	RemoveProperty
    };

    // data
    int type;
    int simpleLen;
    int rawLen;
    char *rawData;
    
    // functions
    void write( QSocket *s ) {
	qws_write_command( s, type, simpleDataPtr, simpleLen, rawData, rawLen );
    }
    bool read( QSocket *s ) {
	bool b = qws_read_command( s, simpleDataPtr, simpleLen,
				 rawData, rawLen, bytesRead );
	return b;
    }
	    
    // temp variables
    char *simpleDataPtr;
    int bytesRead;

};

/*********************************************************************
 *
 * Commands
 *
 *********************************************************************/

struct QWSCreateCommand : public QWSCommand
{
    QWSCreateCommand() : 
	QWSCommand( QWSCommand::Create, sizeof( simpleData ), (char*)&simpleData ) {} 

    struct SimpleData {
    } simpleData;
    
};

struct QWSAddPropertyCommand : public QWSCommand
{
    QWSAddPropertyCommand() : 
	QWSCommand( QWSCommand::AddProperty, sizeof( simpleData ), (char*)&simpleData ) {} 

    struct SimpleData {
	int winId, property;
    } simpleData;
    
};

struct QWSSetPropertyCommand : public QWSCommand
{
    QWSSetPropertyCommand() : 
	QWSCommand( QWSCommand::SetProperty, sizeof( simpleData ), (char*)&simpleData ) {} 

    struct SimpleData {
	int winId, property, mode;
    } simpleData;
    
};

struct QWSRemovePropertyCommand : public QWSCommand
{
    QWSRemovePropertyCommand() : 
	QWSCommand( QWSCommand::RemoveProperty, sizeof( simpleData ), (char*)&simpleData ) {} 

    struct SimpleData {
	int winId, property;
    } simpleData;
    
};

#endif

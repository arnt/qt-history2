/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwscommand_qws.cpp#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qwscommand_qws.h"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/

void qws_write_command( QSocket *socket, int type,
			       char *simpleData, int simpleLen, char *rawData, int rawLen )
{
    qws_write_uint( socket, type );
    qws_write_uint( socket, rawLen == -1 ? 0 : rawLen );
    if ( simpleData && simpleLen )
	socket->writeBlock( simpleData, simpleLen );
    if ( rawLen && rawData )
	socket->writeBlock( rawData, rawLen );
}

bool qws_read_command( QSocket *socket, char *&simpleData, int &simpleLen,
			      char *&rawData, int &rawLen,
			      int &bytesRead )
{
    if ( rawLen == -1 ) {
	if ( socket->size() < sizeof( rawLen ) )
	    return FALSE;
	rawLen = qws_read_uint( socket );
    }

    if ( !bytesRead ) {
	if ( simpleLen ) {
	    if ( socket->size() < uint(simpleLen) )
		return FALSE;
	    bytesRead = socket->readBlock( simpleData, simpleLen );
	} else
	    bytesRead = 1; // hack!
    }
	
    if ( bytesRead ) {
	if ( !rawLen )
	    return TRUE;
	if ( socket->size() < uint(rawLen) )
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


QWSProtocolItem::~QWSProtocolItem() { if (deleteRaw) delete [] rawDataPtr; }

void QWSProtocolItem::write( QSocket *s ) {
    //qDebug( "sending type %d", type );
    qws_write_command( s, type, simpleDataPtr, simpleLen, rawDataPtr, rawLen );
}

bool QWSProtocolItem::read( QSocket *s ) {
    bool b = qws_read_command( s, simpleDataPtr, simpleLen,
			       rawDataPtr, rawLen, bytesRead );
    if ( b ) {
	setData(rawDataPtr, rawLen, FALSE);
	deleteRaw = TRUE;
    }
    return b;
}
    
void QWSProtocolItem::copyFrom( const QWSProtocolItem *item ) {
    if ( this == item )
	return;
    simpleLen = item->simpleLen;
    memcpy( simpleDataPtr, item->simpleDataPtr, simpleLen );
    setData( item->rawDataPtr, item->rawLen );
}

void QWSProtocolItem::setData( char *data, int len, bool allocateMem ) {
    if ( !data && !len ) {
	rawDataPtr = 0;
	rawLen = 0;
	return;
    }
    if ( len < 0 )
	return;
    if ( deleteRaw )
	delete [] rawDataPtr;
    if ( allocateMem ) {
	rawDataPtr = new char[ len ];
	if (data)
	    memcpy( rawDataPtr, data, len );
	deleteRaw = TRUE;
    } else {
	rawDataPtr = data;
	deleteRaw = FALSE;
    }
    rawLen = len;
}

QWSCommand *QWSCommand::factory( int type )
{
    QWSCommand *command = 0;
    switch ( type ) {
    case QWSCommand::Create:
	command = new QWSCreateCommand;
	break;
    case QWSCommand::Region:
	command = new QWSRegionCommand;
	break;
    case QWSCommand::RegionMove:
	command = new QWSRegionMoveCommand;
	break;
    case QWSCommand::RegionDestroy:
	command = new QWSRegionDestroyCommand;
	break;
    case QWSCommand::AddProperty:
	command = new QWSAddPropertyCommand;
	break;
    case QWSCommand::SetProperty:
	command = new QWSSetPropertyCommand;
	break;
    case QWSCommand::RemoveProperty:
	command = new QWSRemovePropertyCommand;
	break;
    case QWSCommand::GetProperty:
	command = new QWSGetPropertyCommand;
	break;
    case QWSCommand::SetSelectionOwner:
	command = new QWSSetSelectionOwnerCommand;
	break;
    case QWSCommand::RequestFocus:
	command = new QWSRequestFocusCommand;
	break;
    case QWSCommand::ChangeAltitude:
	command = new QWSChangeAltitudeCommand;
	break;
    case QWSCommand::DefineCursor:
	command = new QWSDefineCursorCommand;
	break;
    case QWSCommand::SelectCursor:
	command = new QWSSelectCursorCommand;
	break;
    case QWSCommand::GrabMouse:
	command = new QWSGrabMouseCommand;
	break;
    case QWSCommand::PlaySound:
	command = new QWSPlaySoundCommand;
	break;
    default:
	qDebug( "QWSCommand::factory : Type error - got %08x!", type );
    }   
    return command;
}

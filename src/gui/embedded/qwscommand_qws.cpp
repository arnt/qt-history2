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

#include "qwscommand_qws.h"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/

//#define QWSCOMMAND_DEBUG
#ifdef QWSCOMMAND_DEBUG
#include <stdio.h>

static void hexDump( char * data, int len )
{
    fprintf( stderr, "(%d bytes) ", len );
      
    for ( int i = 0; i < len; i++ ) {
	uint c = (uchar)data[i];
	fprintf( stderr, "%02x %c ", c, (c >' ' && c < '~') ? c : ' ' );
    }
    fprintf( stderr, "\n" );
}
#endif

#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command( QWSSocket *socket, int type,
			       char *simpleData, int simpleLen, char *rawData, int rawLen )
{
#ifdef QWSCOMMAND_DEBUG
    fprintf( stderr, "writing type %d\n", type );
    fprintf( stderr, "simpleData " ); hexDump( simpleData, simpleLen );
    fprintf( stderr, "rawData " ); hexDump( rawData, rawLen );
#endif
    qws_write_uint( socket, type );
    qws_write_uint( socket, rawLen == -1 ? 0 : rawLen );
    if ( simpleData && simpleLen ) {
	socket->writeBlock( simpleData, simpleLen );
#ifdef QWSCOMMAND_DEBUG
	qDebug( "writing simpleLen %d", simpleLen );
#endif
    }
    if ( rawLen && rawData ) {
	socket->writeBlock( rawData, rawLen );
#ifdef QWSCOMMAND_DEBUG
	qDebug( "writing rawLen %d", rawLen );
#endif
    }
}

bool qws_read_command( QWSSocket *socket, char *&simpleData, int &simpleLen,
			      char *&rawData, int &rawLen,
			      int &bytesRead )
{
    if ( rawLen == -1 ) {
	if ( socket->size() < sizeof( rawLen ) )
	    return FALSE;
	rawLen = qws_read_uint( socket );
#ifdef QWSCOMMAND_DEBUG
	fprintf( stderr, "qws_read_command rawLen %d\n", rawLen );
#endif
    }

    if ( !bytesRead ) {
	if ( simpleLen ) {
	    if ( socket->size() < uint(simpleLen) )
		return FALSE;
	    bytesRead = socket->readBlock( simpleData, simpleLen );
#ifdef QWSCOMMAND_DEBUG
	    fprintf( stderr, "simpleData " );
	    hexDump( simpleData, bytesRead );
#endif
	} else
	    bytesRead = 1; // hack!
#ifdef QWSCOMMAND_DEBUG
	fprintf( stderr, "simpleLen %d, bytesRead %d\n", simpleLen, bytesRead );
#endif
    }

    if ( bytesRead ) {
	if ( !rawLen )
	    return TRUE;
	if ( socket->size() < uint(rawLen) )
	    return FALSE;
	rawData = new char[ rawLen ];
	bytesRead += socket->readBlock( rawData, rawLen );
#ifdef QWSCOMMAND_DEBUG
	fprintf( stderr, "rawData " );
	hexDump( rawData, rawLen );
	fprintf( stderr, "==== bytesRead %d\n", bytesRead );
#endif
	return TRUE;
    }
    return FALSE;
}
#endif
/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/


QWSProtocolItem::~QWSProtocolItem() { if (deleteRaw) delete [] rawDataPtr; }

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSProtocolItem::write( QWSSocket *s ) {
#ifdef QWSCOMMAND_DEBUG
    qDebug( "QWSProtocolItem::write sending type %d", type );
#endif
    qws_write_command( s, type, simpleDataPtr, simpleLen, rawDataPtr, rawLen );
}

bool QWSProtocolItem::read( QWSSocket *s ) {
    bool b = qws_read_command( s, simpleDataPtr, simpleLen,
			       rawDataPtr, rawLen, bytesRead );
    if ( b ) {
	setData(rawDataPtr, rawLen, FALSE);
	deleteRaw = TRUE;
    }
    return b;
}
#endif
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
    case QWSCommand::GrabKeyboard:
	command = new QWSGrabKeyboardCommand;
	break;
#ifndef QT_NO_SOUND
    case QWSCommand::PlaySound:
	command = new QWSPlaySoundCommand;
	break;
#endif
#ifndef QT_NO_COP
    case QWSCommand::QCopRegisterChannel:
	command = new QWSQCopRegisterChannelCommand;
	break;
    case QWSCommand::QCopSend:
	command = new QWSQCopSendCommand;
	break;
#endif
    case QWSCommand::RegionName:
	command = new QWSRegionNameCommand;
	break;
    case QWSCommand::Identify:
	command = new QWSIdentifyCommand;
	break;
    case QWSCommand::RepaintRegion:
	command = new QWSRepaintRegionCommand;
	break;
#ifndef QT_NO_QWS_IM
    case QWSCommand::SetIMInfo:
	command = new QWSSetIMInfoCommand;
	break;
    case QWSCommand::ResetIM:
	command = new QWSResetIMCommand;
	break;
    case QWSCommand::SetIMFont:
	command = new QWSSetIMFontCommand;
	break;
    case QWSCommand::IMMouse:
	command = new QWSIMMouseCommand;
	break;
#endif
    default:
	qWarning( "QWSCommand::factory : Type error - got %08x!", type );
    }
    return command;
}

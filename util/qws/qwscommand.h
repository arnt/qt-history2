/****************************************************************************
**
** Implementation of Qt/FB central server.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

static bool qws_read_command( QSocket *socket, char *&simpleData, int &simpleLen,
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

struct QWSCommand
{
    // ctor - dtor
    QWSCommand( int t, int len, char *ptr ) : type( t ),
	simpleLen( len ), rawLen( -1 ), simpleDataPtr( ptr ),
	rawDataPtr( 0 ), bytesRead( 0 ) {}
    virtual ~QWSCommand() { delete rawDataPtr; }

    enum Type {
	Unknown = 0,
	Create,
	Destroy,
	Region,
	SetProperty,
	AddProperty,
	RemoveProperty,
	GetProperty,
	SetSelectionOwner,
	ConvertSelection,
	RegionAck,
	ChangeAltitude
    };

    // data
    int type;
    int simpleLen;
    int rawLen;

    // functions
    void write( QSocket *s ) {
	qws_write_command( s, type, simpleDataPtr, simpleLen, rawDataPtr, rawLen );
    }
    bool read( QSocket *s ) {
	bool b = qws_read_command( s, simpleDataPtr, simpleLen,
				 rawDataPtr, rawLen, bytesRead );
	setData( rawDataPtr, rawLen, FALSE );
	return b;
    }
	
    virtual void setData( char *data, int len, bool allocateMem = TRUE ) {
	if ( !data && !len ) {
	    rawDataPtr = 0;
	    rawLen = 0;
	    return;
	}
	if ( len < 0 )
	    return;
	if ( allocateMem )
	    rawDataPtr = new char[ len ];
	memcpy( rawDataPtr, data, len );
	rawLen = len;
    }

    // temp variables
    char *simpleDataPtr;
    char *rawDataPtr;
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
	QWSCommand( QWSCommand::Create, 0, 0 ) {}

};

struct QWSRegionCommand : public QWSCommand
{
    QWSRegionCommand() :
	QWSCommand( QWSCommand::Region, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	rectangles = (Rectangle*)rawDataPtr;
    }

    struct SimpleData {
	int windowid;
	int nrectangles;
    } simpleData;

    struct Rectangle {
        int x, y, width, height;
    } *rectangles;

};

struct QWSRegionAckCommand : public QWSCommand
{
    QWSRegionAckCommand() :
	QWSCommand( QWSCommand::RegionAck, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int eventid;
    } simpleData;

};


struct QWSChangeAltitudeCommand : public QWSCommand
{
    QWSChangeAltitudeCommand() :
	QWSCommand( QWSCommand::ChangeAltitude, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int altitude;
    } simpleData;

};


struct QWSAddPropertyCommand : public QWSCommand
{
    QWSAddPropertyCommand() :
	QWSCommand( QWSCommand::AddProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSSetPropertyCommand : public QWSCommand
{
    QWSSetPropertyCommand() :
	QWSCommand( QWSCommand::SetProperty, sizeof( simpleData ),
		    (char*)&simpleData ) { data = 0; }

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	data = rawDataPtr;
    }

    struct SimpleData {
	int windowid, property, mode;
    } simpleData;

    char *data;
};

struct QWSRemovePropertyCommand : public QWSCommand
{
    QWSRemovePropertyCommand() :
	QWSCommand( QWSCommand::RemoveProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSGetPropertyCommand : public QWSCommand
{
    QWSGetPropertyCommand() :
	QWSCommand( QWSCommand::GetProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSSetSelectionOwnerCommand : public QWSCommand
{
    QWSSetSelectionOwnerCommand() :
	QWSCommand( QWSCommand::SetSelectionOwner, 
		    sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int hour, minute, sec, ms; // time
    } simpleData;

};

struct QWSConvertSelectionCommand : public QWSCommand
{
    QWSConvertSelectionCommand() :
	QWSCommand( QWSCommand::ConvertSelection, 
		    sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int requestor; // requestor window of the selection
	int selection; // property on requestor into which the selection should be stored
	int mimeTypes; // property ion requestor in which the mimetypes, in which the selection may be, are stored
    } simpleData;

};

#endif

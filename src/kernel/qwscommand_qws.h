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

#ifndef QWSCOMMAND_H
#define QWSCOMMAND_H

#ifndef QT_H
#include "qwsutils_qws.h"
#endif // QT_H

#define QTE_PIPE "/tmp/.QtEmbedded"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/

void qws_write_command( QSocket *socket, int type, char *simpleData, int simpleLen, char *rawData, int rawLen );
bool qws_read_command( QSocket *socket, char *&simpleData, int &simpleLen, char *&rawData, int &rawLen, int &bytesRead );

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/


struct QWSProtocolItem
{
    // ctor - dtor
    QWSProtocolItem( int t, int len, char *ptr ) : type( t ),
	simpleLen( len ), rawLen( -1 ), deleteRaw(FALSE), simpleDataPtr( ptr ),
	rawDataPtr( 0 ), bytesRead( 0 ) { }
    virtual ~QWSProtocolItem();


    // data
    int type;
    int simpleLen;
    int rawLen;
    bool deleteRaw;

    // functions
    void write( QSocket *s );
    bool read( QSocket *s );
    void copyFrom( const QWSProtocolItem *item );
    
    virtual void setData( char *data, int len, bool allocateMem = TRUE );

    char *simpleDataPtr;
    char *rawDataPtr;
    // temp variables
    int bytesRead;
};


struct QWSCommand : QWSProtocolItem
{
    QWSCommand( int t, int len, char *ptr ) : QWSProtocolItem(t,len,ptr) {}

    enum Type {
	Unknown = 0,
	Create,
	Destroy,
	Region,
	RegionMove,
	RegionDestroy,
	SetProperty,
	AddProperty,
	RemoveProperty,
	GetProperty,
	SetSelectionOwner,
	ConvertSelection,
	RequestFocus,
	ChangeAltitude,
	DefineCursor,
	SelectCursor,
	GrabMouse,
	PlaySound
    };
    static QWSCommand *factory( int type );
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
	rectangles = (QRect*)rawDataPtr;
    }

    struct SimpleData {
	int windowid;
	int nrectangles;
    } simpleData;

    QRect *rectangles;

};

struct QWSRegionMoveCommand : public QWSCommand
{
    QWSRegionMoveCommand() :
	QWSCommand( QWSCommand::RegionMove, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int dx;
	int dy;
    } simpleData;

};

struct QWSRegionDestroyCommand : public QWSCommand
{
    QWSRegionDestroyCommand() :
	QWSCommand( QWSCommand::RegionDestroy, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
    } simpleData;

};

struct QWSRequestFocusCommand : public QWSCommand
{
    QWSRequestFocusCommand() :
	QWSCommand( QWSCommand::RequestFocus, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int flag;
    } simpleData;
};

struct QWSChangeAltitudeCommand : public QWSCommand
{
    QWSChangeAltitudeCommand() :
	QWSCommand( QWSCommand::ChangeAltitude, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int altitude;
	bool fixed;
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

struct QWSDefineCursorCommand : public QWSCommand
{
    QWSDefineCursorCommand() :
	QWSCommand( QWSCommand::DefineCursor,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	data = (unsigned char *)rawDataPtr;
    }

    struct SimpleData {
	int width;
	int height;
	int hotX;
	int hotY;
	int id;
    } simpleData;

    unsigned char *data;
};

struct QWSSelectCursorCommand : public QWSCommand
{
    QWSSelectCursorCommand() :
	QWSCommand( QWSCommand::SelectCursor,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int id;
    } simpleData;
};

struct QWSGrabMouseCommand : public QWSCommand
{
    QWSGrabMouseCommand() :
	QWSCommand( QWSCommand::GrabMouse,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
	bool grab;  // grab or ungrab?
    } simpleData;
};

#ifndef QT_NO_SOUND
struct QWSPlaySoundCommand : public QWSCommand
{
    QWSPlaySoundCommand() :
	QWSCommand( QWSCommand::PlaySound,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	filename = QString((QChar*)rawDataPtr,len/2);
    }
    void setFilename( const QString& n )
    {
	setData( (char*)n.unicode(), n.length()*2, TRUE );
    }

    struct SimpleData {
	int windowid;
    } simpleData;
    QString filename;
};
#endif


#endif

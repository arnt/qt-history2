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
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

/*
  The format of a set property command is:
  A,B,C,D:data

  A .... winId
  B .... property
  C .... mode
  D .... length of data
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
    QString s;
    QStringList lst;
    char c;
    while ( ( c = client->getch() ) != ':' ) {
	qDebug( "got: %c", c );
	if ( c == ',' ) {
	    lst.append( s );
	    s = QString::null;
	    continue;
	}
	s += c;
    }
    lst.append( s );

    if ( lst.count() != 4 )
	qFatal( "QWSSetPropertyCommand::readData: Protocol error" );

    int len = -1;

    winId = lst[ 0 ].toInt();
    property = lst[ 1 ].toInt();
    mode = lst[ 2 ].toInt();
    len = lst[ 3 ].toInt();

    qDebug( "%d %d %d %d", winId, property, mode, len );

    if ( len > 0 ) {
	data.resize( len );
	client->readBlock( data.data(), len );
    }
}

void QWSSetPropertyCommand::execute()
{
    qDebug( "QWSSetPropertyCommand::execute: set data:" );
    qDebug( "%s", data.data() );
}

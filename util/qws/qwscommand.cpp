/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwcommand.cpp#3 $
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

#include "qwscommand.h"
#include "qwsevent.h"
#include "qws.h"

#include <stdlib.h>

#include <qstring.h>

QWSCommandDict *qwsCommandRegister = 0;

void qwsRegisterCommands()
{
    QWSCommand::registerCommand( QWSCommand::Create, new QWSCommandFactory< QWSCreateCommand > );
    QWSCommand::registerCommand( QWSCommand::SetProperty, new QWSCommandFactory< QWSSetPropertyCommand > );
    QWSCommand::registerCommand( QWSCommand::AddProperty, new QWSCommandFactory< QWSAddPropertyCommand > );
    QWSCommand::registerCommand( QWSCommand::RemoveProperty, new QWSCommandFactory< QWSRemovePropertyCommand > );
}

/*********************************************************************
 *
 * Class: QWSCommand
 *
 *********************************************************************/

QWSCommand::QWSCommand( QWSServer *s, QWSClient *c )
    : server( s ), client( c )
{
}

QWSCommand::~QWSCommand()
{
}

void QWSCommand::readData()
{
    qFatal( "reimplement QWSCommand::readData!" );
}

void QWSCommand::execute()
{
    qFatal( "reimplement QWSCommand::execute!" );
}

void QWSCommand::registerCommand( Type cmd, QWSCommandFactoryBase *commandFactory )
{
    if ( !qwsCommandRegister )
	qwsCommandRegister = new QWSCommandDict;

    qwsCommandRegister->insert( cmd, commandFactory );
}

QWSCommand *QWSCommand::getCommand( Type cmd, QWSServer *server, QWSClient *client )
{
    if ( !qwsCommandRegister )
	qwsCommandRegister = new QWSCommandDict;

    QWSCommandDict::Iterator it = qwsCommandRegister->find( cmd );
    if ( it != qwsCommandRegister->end() ) {
	QWSCommandFactoryBase *factory = ( *it );
	if ( factory )
	    return factory->createCommand( server, client );
    }

    return 0;
}

/*********************************************************************
 *
 * Class: QWSCreateCommand
 *
 *********************************************************************/

QWSCreateCommand::QWSCreateCommand( QWSServer *s, QWSClient *c )
    : QWSCommand( s, c )
{
}

QWSCreateCommand::~QWSCreateCommand()
{
}

void QWSCreateCommand::readData()
{
    client->readBlock((char*)&command,sizeof(command));
}

static int get_object_id()
{
    static int next=1000;
    return next++;
}

void QWSCreateCommand::execute()
{
    QWSCreationEvent event;
    event.type = QWSEvent::Creation;
    event.objectid = get_object_id();
    client->writeBlock((char*)&event,sizeof(event));
}

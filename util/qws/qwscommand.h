/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#3 $
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

#include <qmap.h>
#include <qcstring.h>

class QWSCommand;
class QWSServer;
class QWSClient;

class QWSCommandFactoryBase
{
public:
   virtual QWSCommand *createCommand( QWSServer *server, QWSClient *client ) = 0;

};

template< class Command >
class QWSCommandFactory : public QWSCommandFactoryBase
{
public:
    QWSCommand *createCommand( QWSServer *server, QWSClient *client ) {
	return new Command( server, client );
    }

};

typedef QMap< int, QWSCommandFactoryBase* > QWSCommandDict;
extern QWSCommandDict *qwsCommandRegister;

void qwsRegisterCommands();

/*********************************************************************
 *
 * Class: QWSCommand
 *
 *********************************************************************/

class QWSCommand
{
public:
    QWSCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSCommand();
  
    virtual void readData();
    virtual void execute();
    
    static void registerCommand( int cmd, QWSCommandFactoryBase *commandFactory );
    static QWSCommand *getCommand( int cmd, QWSServer *server, QWSClient *c );

protected:
    QWSServer *server;
    QWSClient *client;
    
};

/*********************************************************************
 *
 * Class: QWSNewWindowCommand
 *
 *********************************************************************/

class QWSNewWindowCommand : public QWSCommand
{
public:
    QWSNewWindowCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSNewWindowCommand();
  
    virtual void readData();
    virtual void execute();

private:
    ushort x, y, w, h;
    ushort flags;
    
};

/*********************************************************************
 *
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

class QWSSetPropertyCommand : public QWSCommand
{
public:
    enum Mode {
	PropReplace,
	PropPrepend,
	PropAppend
    };
	
    QWSSetPropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSSetPropertyCommand();
  
    virtual void readData();
    virtual void execute();

private:
    int winId, property, mode;
    QByteArray data;
    
};

#endif

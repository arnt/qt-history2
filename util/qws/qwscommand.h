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

#include <qmap.h>

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
    enum Type {
	Unknown = 0,
	Create, Destroy,
	Region,
	SetProperty,
	AddProperty,
	RemoveProperty
    };
    
    QWSCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSCommand();

    virtual Type type() { return Unknown; }
    
    virtual void readData();
    virtual void execute();

    static void registerCommand( Type cmd, QWSCommandFactoryBase *commandFactory );
    static QWSCommand *getCommand( Type cmd, QWSServer *server, QWSClient *c );

protected:
    QWSServer *server;
    QWSClient *client;

};

/*********************************************************************
 *
 * Class: QWSCreateCommand
 *
 *********************************************************************/

struct QWSCreate {
    int type;
};

class QWSCreateCommand : public QWSCommand
{
public:
    QWSCreateCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSCreateCommand();

    virtual Type type() { return Create; }

    virtual void readData();
    virtual void execute();

private:
    QWSCreate command;
};

/*********************************************************************
 *
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

class QWSSetPropertyCommand : public QWSCommand
{
public:
    QWSSetPropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSSetPropertyCommand();

    virtual Type type() { return SetProperty; }

    virtual void readData();
    virtual void execute();

private:
    int winId, property, mode;
    QByteArray data;

};

/*********************************************************************
 *
 * Class: QWSAddPropertyCommand
 *
 *********************************************************************/

class QWSAddPropertyCommand : public QWSCommand
{
public:
    QWSAddPropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSAddPropertyCommand();

    virtual Type type() { return AddProperty; }

    virtual void readData();
    virtual void execute();

private:
    int winId, property;

};

/*********************************************************************
 *
 * Class: QWSRemovePropertyCommand
 *
 *********************************************************************/

class QWSRemovePropertyCommand : public QWSCommand
{
public:
    QWSRemovePropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSRemovePropertyCommand();

    virtual Type type() { return RemoveProperty; }

    virtual void readData();
    virtual void execute();

private:
    int winId, property;
};

#endif

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetprotocol.cpp#4 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qnetprotocol.h"

QMap< QString, QNetworkProtocol* > *qNetworkProtocolRegister = 0;
void qRegisterNetworkProtocol( const QString &protocol, QNetworkProtocol *nprotocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    qNetworkProtocolRegister->insert( protocol, nprotocol );
}

QNetworkProtocol *qGetNetworkProtocol( const QString &protocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    QMap< QString, QNetworkProtocol*>::Iterator it = qNetworkProtocolRegister->find( protocol );
    if ( it == qNetworkProtocolRegister->end() )
	return 0;

    return ( *it )->copy();
}

/*!
  \class QNetworkProtocol qnetprotocol.h

  This is a baseclass which should be used for implementations
  of network protocols which can then be used in Qt (e.g.
  in the filedialog).
  
  Most of the methodes below should be reimplemented. To be able
  to make Qt using a new network protocol derived from this class
  following two functions have to be reimplemented
  
    virtual QNetworkProtocol *copy() const;
    virtual QString toString() const;
    
  All other methodes should be implemented if it makes sense for
  the protocol. But e.g. openConnection(), close(), isOpen() and 
  some others should be always reimplemented.

  See also the QFtp implementation in the network extension library
  for more information.
  
  To dynamically register a new protocol in Qt, just do following
  (let's assume the new protocol class is called QtHttp and implements
  an HTTP protocol)
  
    qRegisterNetworkProtocol( "http", new QtHttp );
    
  If you later need an instance of the network protocol implementation
  do:
  
    QtHttp *http = qGetNetworkProtocol( "http" );
    
  If an implementation for this protocol could be found, it returns an
  instance and transferes the ownership of the instace to you (this means,
  you have to delete it later yourself). Else it returns 0.
*/

/*!
  #### todo
*/

QNetworkProtocol::QNetworkProtocol()
    : url( 0 )
{
}

/*!
  #### todo
*/

QNetworkProtocol::~QNetworkProtocol()
{
    url = 0;
}

/*!
  Open connection.
*/

void QNetworkProtocol::openConnection( QUrl *u )
{
    setUrl( u );
}

/*!
  #### todo
*/

bool QNetworkProtocol::isOpen()
{
    return FALSE;
}

/*!
  #### todo
*/

void QNetworkProtocol::close()
{
}

/*!
  #### todo
*/

void QNetworkProtocol::setUrl( QUrl *u )
{
    url = u;
}

/*!
  #### todo
*/

void QNetworkProtocol::listEntries( const QString &, int, int)
{
}

/*!
  #### todo
*/

void QNetworkProtocol::mkdir(const QString & )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::remove( const QString & )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::rename( const QString &, const QString & )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::copy( const QStringList &, const QString &, bool )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::get( const QString & )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::put( const QString & )
{
}

/*!
  #### todo
*/

void QNetworkProtocol::isDir()
{
}

/*!
  #### todo
*/

void QNetworkProtocol::isFile()
{
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkProtocol::copy() const
{
    return new QNetworkProtocol();
}

/*!
  #### todo
*/

QString QNetworkProtocol::toString() const
{
    return QString::null;
}


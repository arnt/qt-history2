/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.cpp#2 $
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

#include "qnetworkprotocol.h"

QMap< QString, QNetworkProtocol* > *qNetworkProtocolRegister = 0;

/*!
  \class QNetworkProtocol qnetworkprotocol.h

  This is a baseclass which should be used for implementations
  of network protocols which can then be used in Qt (e.g.
  in the filedialog).
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

void QNetworkProtocol::put( const QCString & )
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

void QNetworkProtocol::registerNetworkProtocol( const QString &protocol, QNetworkProtocol *nprotocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    qNetworkProtocolRegister->insert( protocol, nprotocol );
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkProtocol::getNetworkProtocol( const QString &protocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    QMap< QString, QNetworkProtocol*>::Iterator it = qNetworkProtocolRegister->find( protocol );
    if ( it == qNetworkProtocolRegister->end() )
	return 0;

    return ( *it )->copy();
}







/*!
  \class QNetworkFileAccess qnetworkprotocol.h

*/

/*!
  #### todo
*/

QNetworkFileAccess::QNetworkFileAccess()
    : QNetworkProtocol()
{
}


/*!
  #### todo
*/

QNetworkFileAccess::~QNetworkFileAccess()
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::listEntries( const QString &, int, int)
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::mkdir(const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::remove( const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::rename( const QString &, const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::copy( const QStringList &, const QString &, bool )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::isUrlDir()
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::isUrlFile()
{
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkFileAccess::copy() const
{
    return new QNetworkFileAccess();
}

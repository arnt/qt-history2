/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetprotocol.cpp#3 $
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
  #### todo
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
  #### todo
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

void QNetworkProtocol::get()
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


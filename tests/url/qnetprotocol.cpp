/****************************************************************************
** $Id: //depot/qt/main/tests/url/qnetprotocol.cpp#3 $
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

QNetworkProtocol::QNetworkProtocol()
    : url( 0 )
{
}

QNetworkProtocol::~QNetworkProtocol()
{
    url = 0;
}

void QNetworkProtocol::openConnection( QUrl *u )
{
    setUrl( u );
}

bool QNetworkProtocol::isOpen()
{
    return FALSE;
}

void QNetworkProtocol::close()
{
}

void QNetworkProtocol::setUrl( QUrl *u )
{
    url = u;
}

void QNetworkProtocol::listEntries( const QString &, int, int)
{
}

void QNetworkProtocol::mkdir(const QString & )
{
}

void QNetworkProtocol::remove( const QString & )
{
}

void QNetworkProtocol::rename( const QString &, const QString & )
{
}

void QNetworkProtocol::copy( const QStringList &, const QString &, bool )
{
}

void QNetworkProtocol::get()
{
}

QUrlInfo QNetworkProtocol::makeInfo() const
{
    return QUrlInfo();
}

QNetworkProtocol *QNetworkProtocol::copy() const
{
    return new QNetworkProtocol();
}

QString QNetworkProtocol::toString() const
{
    return QString::null;
}


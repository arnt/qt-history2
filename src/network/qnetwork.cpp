/****************************************************************************
**
** Implementation of qInitNetworkProtocols function.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qnetwork.h"

#ifndef QT_NO_NETWORK

#include "qnetworkprotocol.h"

// protocols
#include "qftp.h"
#include "qhttp.h"

/*! \file qnetwork.h */
/*!
    \relates QUrlOperator

    This function registers the network protocols for FTP and HTTP.
    You must call this function before you use QUrlOperator for
    these protocols.

    This function is declared in \l qnetwork.h.
*/
void qInitNetworkProtocols()
{
#ifndef QT_NO_NETWORKPROTOCOL_FTP
    QNetworkProtocol::registerNetworkProtocol( "ftp", new QNetworkProtocolFactory< QFtp > );
#endif
#ifndef QT_NO_NETWORKPROTOCOL_HTTP
    QNetworkProtocol::registerNetworkProtocol( "http", new QNetworkProtocolFactory< QHttp > );
#endif
}

#endif // QT_NO_NETWORK

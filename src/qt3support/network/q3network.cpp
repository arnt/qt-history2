/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3network.h"

#ifndef QT_NO_NETWORK

#include "q3networkprotocol.h"

// protocols
#include "q3ftp.h"
#include "q3http.h"

/*!
    \relates Q3UrlOperator

    This function registers the network protocols for FTP and HTTP.
    You must call this function before you use QUrlOperator for
    these protocols.
*/
void q3InitNetworkProtocols()
{
#ifndef QT_NO_NETWORKPROTOCOL_FTP
    Q3NetworkProtocol::registerNetworkProtocol( "ftp", new Q3NetworkProtocolFactory< Q3Ftp > );
#endif
#ifndef QT_NO_NETWORKPROTOCOL_HTTP
    Q3NetworkProtocol::registerNetworkProtocol( "http", new Q3NetworkProtocolFactory< Q3Http > );
#endif
}

#endif // QT_NO_NETWORK

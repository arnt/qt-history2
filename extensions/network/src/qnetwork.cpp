/****************************************************************************
** $Id$
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qnetwork.h"

#ifndef QT_NO_NETWORKPROTOCOL

#include "qnetworkprotocol.h"

// protocols
#include "qftp.h"
#include "qhttp.h"

void qInitNetworkProtocols()
{
#ifndef QT_NO_NETWORKPROTOCOL_FTP
    QNetworkProtocol::registerNetworkProtocol( "ftp", new QNetworkProtocolFactory< QFtp > );
#endif
#ifndef QT_NO_NETWORKPROTOCOL_HTTP
    QNetworkProtocol::registerNetworkProtocol( "http", new QNetworkProtocolFactory< QHttp > );
#endif
};

#endif // QT_NO_NETWORKPROTOCOL

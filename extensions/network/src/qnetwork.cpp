/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qnetwork.cpp#1 $
**
** Implementation of JPEG QImage IOHandler
**
** Created : 970521
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

#include "qnetwork.h"
#include "qnetprotocol.h"

// protocols
#include "qftp.h"

void qInitNetworkProtocols()
{
    qRegisterNetworkProtocol( "ftp", new QFtp );
};

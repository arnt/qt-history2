/****************************************************************************
** $Id: //depot/qt/main/src/tools/qipcomm.cpp#1 $
**
** Implementation of QIPComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qipcomm.h"
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/tools/qipcomm.cpp#1 $")


QIPComm::QIPComm()
{
    sockfd = -1;
}

QIPComm::~QIPComm()
{
}


void QIPComm::setHost( const char *hostname, uint portnum )
{
    QString sap;
    sap.sprintf( "%d", portnum );
    QTComm::setHost( hostname, sap );
}

int QIPComm::portNum() const
{
    return servap.isNull() ? -1 : atoi((const char *)servap);
}

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtcomm.cpp#1 $
**
** Implementation of QTComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtcomm.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qtcomm.cpp#1 $")


QTComm::QTComm()
{
    setType( IO_Sequential );
    readcomp = FALSE;
}

QTComm::~QTComm()
{
}


void QTComm::setHost( const char *hostName, const char *serviceAccessPoint )
{
    host = hostName;
    servap = serviceAccessPoint;
}


int QTComm::getch()				// not working for comm classes
{
#if defined(CHECK_STATE)
    warning( "QTComm::getch: Not implemented for communication classes" );
#endif
    return -1;
}

int QTComm::putch( int ch )
{
    if ( isRaw() ) {				// very inefficient if raw
	char buf[1];
	buf[0] = (char)ch;
	writeBlock( buf, 1 );
    }
    else
	ch = buffer.putch( ch );
    return ch;
}

int QTComm::ungetch( int )			// not working for comm classes
{
#if defined(CHECK_STATE)
    warning( "QTComm::ungetch: Not implemented for communication classes" );
#endif
    return -1;
}

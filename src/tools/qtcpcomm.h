/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtcpcomm.h#1 $
**
** Definition of QTCPComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTCPCOMM_H
#define QTCPCOMM_H

#include "qipcomm.h"


class QTCPComm : public QIPComm			// TCP-based communication
{
public:
    QTCPComm();
   ~QTCPComm();

    bool  listen();
    void  hangup();

    bool  connect( int mode=IO_ReadWrite );

    bool  disconnect();
    void  abort();

    bool  transmit();

    int	  readBlock( char *data, uint len );
    int	  writeBlock( const char *data, uint len );

protected:
    int	  lsockfd;				// listen socket
};


#endif // QTCPCOMM_H

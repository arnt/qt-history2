/****************************************************************************
** $Id: //depot/qt/main/src/tools/qudpcomm.h#1 $
**
** Definition of QTCPComm class
**
** Author  : Haavard Nord
** Created : 940917
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QUDPCOMM_H
#define QUDPCOMM_H

#include "qipcomm.h"


class QUDPComm : public QIPComm			// UDP-based communication
{
public:
    QUDPComm();
   ~QUDPComm();

    bool listen();
    bool connect( int );
    bool disconnect();
    void abort();

    bool transmit();

    int	 readBlock( char *data, uint len );
    int	 writeBlock( const char *data, uint len );
};


#endif // QUDPCOMM_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qnpipe.h#1 $
**
** Definition of QNPipe class
**
** Author  : Haavard Nord
** Created : 940921
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QNPIPE_H
#define QNPIPE_H

#include "qtcomm.h"


class QNPipe : public QTComm			// named pipe class
{
public:
    QNPipe();
   ~QNPipe();

    bool  listen();
    void  hangup();
    bool  connect( int mode=IO_ReadWrite );
    bool  disconnect();
    void  abort();

    bool  transmit();

    int	  readBlock( char *data, uint len );
    int	  writeBlock( const char *data, uint len );

protected:
#if defined(UNIX)
    int	  readfd, writefd;
    bool  isserver;
#endif
};


#endif // QNPIPE_H

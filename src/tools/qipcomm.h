/****************************************************************************
** $Id: //depot/qt/main/src/tools/qipcomm.h#1 $
**
** Definition of QIPComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QIPCOMM_H
#define QIPCOMM_H

#include "qtcomm.h"


class QIPComm : public QTComm			// IP-based communication
{
public:
    QIPComm();
   ~QIPComm();

    void  setHost( const char *host, uint portnum );
    int	  portNum() const;			// returns SAP as a number

protected:
    int	 sockfd;
};


#endif // QIPCOMM_H

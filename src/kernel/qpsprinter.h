/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.h#1 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of internal QPSPrinter class.
** QPSPrinter implements PostScript (tm) output via QPrinter.
**
** Author  : Eirik Eng
** Created : 940927
**
** Copyright (c) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QPSPRN_H
#define QPSPRN_H

#include "qprinter.h"
#include "qtstream.h"


class QPSPrinter : public QPaintDevice
{
private:
    QPSPrinter( QPrinter * );
   ~QPSPrinter();

    bool	cmd ( int, QPainter *, QPDevCmdParam * );

    QPrinter   *printer;
    QIODevice  *device;
    QTextStream stream;
    int		pageCount;
    bool	dirtyMatrix;
    friend class QPrinter;
};


// Additional commands for QPSPrinter

#define PDC_PRT_NEWPAGE	100
#define PDC_PRT_ABORT	101


#endif // QPSPRN_H

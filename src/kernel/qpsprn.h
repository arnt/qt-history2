/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprn.h#11 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of internal QPSPrinter class.
** QPSPrinter implements PostScript (tm) output via QPrinter.
**
** Created : 940927
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QPSPRN_H
#define QPSPRN_H

#ifndef QT_H
#include "qprinter.h"
#include "qtstream.h"
#endif // QT_H


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
    bool	dirtyNewPage;
    bool	epsf;
    QString	fontsUsed;
    friend class QPrinter;

private:	// Disabled copy constructor and operator=
    QPSPrinter( const QPSPrinter & );
    QPSPrinter &operator=( const QPSPrinter & );

    void matrixSetup( QPainter * );
    void orientationSetup();
    void newPageSetup( QPainter * );
};


// Additional commands for QPSPrinter

#define PDC_PRT_NEWPAGE 100
#define PDC_PRT_ABORT	101


#endif // QPSPRN_H

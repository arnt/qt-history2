/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprn.h#13 $
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


struct QPSPrinterPrivate;

class QPSPrinter : public QPaintDevice
{
private:
    // QPrinter uses these
    QPSPrinter( QPrinter *, int );
   ~QPSPrinter();

    bool cmd ( int, QPainter *, QPDevCmdParam * );

    friend class QPrinter;
private:
    // QPrinter does not use these

    QPrinter   *printer;
    QPSPrinterPrivate *d;
    QTextStream stream;
    int		pageCount;
    bool	dirtyMatrix;
    bool	dirtyClipping;
    bool	dirtyNewPage;
    bool	epsf;
    QString	fontsUsed;

    void matrixSetup( QPainter * );
    void clippingSetup( QPainter * );
    void orientationSetup();
    void newPageSetup( QPainter * );
    void emitHeader();
    void setFont( const QFont & );
    void drawImage( QPainter *, const QPoint &, const QImage & );

    // Disabled copy constructor and operator=
    QPSPrinter( const QPSPrinter & );
    QPSPrinter &operator=( const QPSPrinter & );
};


// Additional commands for QPSPrinter

#define PDC_PRT_NEWPAGE 100
#define PDC_PRT_ABORT	101


#endif // QPSPRN_H

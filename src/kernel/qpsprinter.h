/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.h#20 $
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
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPSPRINTER_H
#define QPSPRINTER_H

#ifndef QT_H
#include "qprinter.h"
#include "qtextstream.h"
#endif // QT_H


struct QPSPrinterPrivate;

class Q_EXPORT QPSPrinter : public QPaintDevice
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
    bool	dirtyNewPage;
    bool	epsf;
    QString	fontsUsed;

    void matrixSetup( QPainter * );
    void clippingSetup( QPainter * );
    virtual void setClippingOff( QPainter * );
    void orientationSetup();
    void newPageSetup( QPainter * );
    void resetDrawingTools( QPainter * );
    void emitHeader( bool finished );
    virtual void setFont( const QFont & );
    void drawImage( QPainter *, const QPoint &, const QImage & );

    // Disabled copy constructor and operator=
    QPSPrinter( const QPSPrinter & );
    QPSPrinter &operator=( const QPSPrinter & );
};


// Additional commands for QPSPrinter

#define PDC_PRT_NEWPAGE 100
#define PDC_PRT_ABORT	101


#endif // QPSPRINTER_H

/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter_p.h#1 $
**
** Definition of internal QPSPrinter class.
** QPSPrinter implements PostScript (tm) output via QPrinter.
**
** Created : 940927
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPSPRINTER_P_H
#define QPSPRINTER_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qpsprinter.cpp and qprinter_x11.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qprinter.h"
#include "qtextstream.h"
#endif // QT_H

#ifndef QT_NO_PRINTER

struct QPSPrinterPrivate;

class Q_EXPORT QPSPrinter : public QPaintDevice
{
private:
    // QPrinter uses these
    QPSPrinter( QPrinter *, int );
   ~QPSPrinter();

    bool cmd ( int, QPainter *, QPDevCmdParam * );

    enum { NewPage = 100, AbortPrinting };

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

#endif // QT_NO_PRINTER

#endif // QPSPRINTER_P_H

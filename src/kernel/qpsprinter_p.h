/****************************************************************************
**
** Definition of internal QPSPrinter class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

class QPSPrinterPrivate;

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
    // not used by QPrinter
    QPSPrinterPrivate *d;

    // Disabled copy constructor and operator=
    QPSPrinter( const QPSPrinter & );
    QPSPrinter &operator=( const QPSPrinter & );
};

#endif // QT_NO_PRINTER

#endif // QPSPRINTER_P_H

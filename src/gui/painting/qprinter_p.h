/****************************************************************************
**
** Definition of QPrinter class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPRINTER_P_H
#define QPRINTER_P_H
#ifndef QT_NO_PRINTER

#ifndef QT_H
#include <qshared.h>
#include <qstring.h>
#include <qsize.h>
#endif // QT_H

class QPrinterPrivate
{
public:
    Q_UINT32 printerOptions;
    QPrinter::PrintRange printRange;
};

#endif
#endif

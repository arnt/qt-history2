/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_PRINTER

#ifndef QPRINTER_P_H
#define QPRINTER_P_H

#include <qglobal.h>
#ifdef QT_COMPAT
#include <qprintdialog.h>
#endif

class QPrintEngine;

class QPrinterPrivate
{
public:
    QPrinterPrivate()
        : printEngine(0)
#ifdef QT_COMPAT
        , printDialog(0)
#endif
    {
    }

    QPrintEngine *printEngine;

#ifdef QT_COMPAT
    QPrintDialog *printDialog;
#endif
};


#endif
#endif // QT_NO_PRINTER

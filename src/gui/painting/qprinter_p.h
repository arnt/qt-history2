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

#ifndef QPRINTER_P_H
#define QPRINTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "QtCore/qglobal.h"

#ifndef QT_NO_PRINTER

#include "QtGui/qprinter.h"
#ifdef QT3_SUPPORT
#include "QtCore/qpointer.h"
#include "QtGui/qprintdialog.h"
#endif

class QPrintEngine;

class QPrinterPrivate
{
public:
    QPrinterPrivate()
        : printEngine(0)
#if defined(QT3_SUPPORT) && !(defined(QT_NO_PRINTDIALOG))
        , printDialog(0)
#endif
    {
    }

    QPrinter::PrinterMode printerMode;
    QPrinter::OutputFormat outputFormat;
    QPrintEngine *printEngine;

#if defined(QT3_SUPPORT) && !(defined(QT_NO_PRINTDIALOG))
    QPointer<QPrintDialog> printDialog;
#endif
};

#endif // QT_NO_PRINTER

#endif // QPRINTER_P_H

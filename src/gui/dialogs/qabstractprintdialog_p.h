/****************************************************************************
**
** Definition of QAbstractPrintDialogPrivate class.
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

#ifndef QABSTRACTPRINTDIALOG_P_H
#define QABSTRACTPRINTDIALOG_P_H

class QPrinter;

#include <private/qdialog_p.h>
#include "qabstractprintdialog.h"

class QAbstractPrintDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QAbstractPrintDialog)
public:
    QAbstractPrintDialogPrivate()
        :
        printer(0),
        options(QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintPageRange),
        minPage(1),
        maxPage(1),
        fromPage(0),
        toPage(0)
    {
    }

    QPrinter *printer;
    QAbstractPrintDialog::PrintDialogOptions options;
    QAbstractPrintDialog::PrintRange printRange;
    int minPage, maxPage, fromPage, toPage;
};

#endif // QABSTRACTPRINTDIALOG_P_H


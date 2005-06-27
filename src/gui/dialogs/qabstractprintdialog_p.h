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

#ifndef QABSTRACTPRINTDIALOG_P_H
#define QABSTRACTPRINTDIALOG_P_H

#ifndef QT_NO_PRINTDIALOG

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
        printRange(QAbstractPrintDialog::AllPages),
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

#endif // QT_NO_PRINTDIALOG

#endif // QABSTRACTPRINTDIALOG_P_H

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

#ifndef QPRINTDIALOG_WIN_H
#define QPRINTDIALOG_WIN_H

#ifdef Q_WS_WIN

#include "qabstractprintdialog.h"

class QPrintDialogWinPrivate;

class Q_GUI_EXPORT QPrintDialogWin : public QAbstractPrintDialog
{
    Q_DECLARE_PRIVATE(QPrintDialogWin)
public:
    QPrintDialogWin(QPrinter *printer, QWidget *parent = 0);

    int exec();
private:
#if defined(Q_DISABLE_COPY)
    QPrintDialogWin(const QPrintDialogWin &);
    QPrintDialogWin &operator=(const QPrintDialogWin &);
#endif
};

#endif // Q_WS_WIN

#endif QPRINTDIALOG_WIN_H

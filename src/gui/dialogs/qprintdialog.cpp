/****************************************************************************
**
** Definition of QPrintDialog class.
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

#include "qprintdialog.h"

#if defined Q_WS_WIN
#  include "qprintdialog_win.h"
#elif defined Q_WS_MAC
#  include "qprintdialog_mac.h"
#else
#  include "qprintdialog_unix.h"
#endif

#include <private/qabstractprintdialog_p.h>

class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
public:
    QPrintDialogPrivate()
        : platformDialog(0)
    {
    }

    QAbstractPrintDialog *platformDialog;
};

#define d d_func()

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogPrivate), printer, parent)
{
}

int QPrintDialog::exec()
{
    if (!d->platformDialog) {
#if defined (Q_WS_WIN)
        d->platformDialog = new QPrintDialogWin(d->printer, parentWidget());
#elif defined (Q_WS_MAC)
        d->platformDialog = new QPrintDialogMac(d->printer, parentWidget());
#else
//        d->platformDialog = new QPrintDialogUnix(d->printer, parentWidget());
#endif
    }

    d->platformDialog->setMinMax(d->minPage, d->maxPage);
    d->platformDialog->setFromTo(d->fromPage, d->toPage);
    d->platformDialog->setEnabledOptions(d->options);
    d->platformDialog->setPrintRange(d->printRange);

    int result = d->platformDialog->exec();

    setMinMax(d->platformDialog->minPage(), d->platformDialog->maxPage());
    setFromTo(d->platformDialog->fromPage(), d->platformDialog->toPage());
    setEnabledOptions(d->platformDialog->enabledOptions());
    setPrintRange(d->platformDialog->printRange());

    return result;
}


/****************************************************************************
**
** Implementation of QPrintDialogMac class.
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

#include "qprintdialog_mac.h"
#include "qt_mac.h"

#include <qprintengine_mac.h>
#include <private/qabstractprintdialog_p.h>
#include <private/qprintengine_mac_p.h>

class QPrintDialogMacPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialogMac)
public:
    QPrintDialogMacPrivate() : ep(0) { }

    QMacPrintEnginePrivate *ep;
};

#define d d_func()

QPrintDialogMac::QPrintDialogMac(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogMacPrivate), printer, parent)
{
    d->ep = static_cast<QMacPrintEngine *>(printer->paintEngine())->d;
}

int QPrintDialogMac::exec()
{
    QMacBlockingFunction func;
    Boolean result;
    // Carbon's documentation lies.
    // It seems the only way that Carbon lets you use all is if the minimum
    // for the page range is 1. This _kind of_ makes sense if you think about
    // it. However, calling setFirstPage or setLastPage always enforces the range.
    PMSetPageRange(d->ep->settings, d->minPage, d->maxPage);
    if (d->printRange == PageRange) {
        PMSetFirstPage(d->ep->settings, d->fromPage, false);
        PMSetLastPage(d->ep->settings, d->toPage, false);
    }
    PMSessionPrintDialog(d->ep->session, d->ep->settings, d->ep->format, &result);
    if (result) {
        UInt32 page;
        PMGetFirstPage(d->ep->settings, &page);
        d->fromPage = qMin(uint(INT_MAX), page);
        PMGetLastPage(d->ep->settings, &page);
        d->toPage = qMin(uint(INT_MAX), page);
        // Carbon hands us back a very large number here even for ALL, set it to max
        // in that case to follow the behavior of the other print dialogs.
        if (d->maxPage < d->toPage)
            d->toPage = d->maxPage;
    }
    return result;
}
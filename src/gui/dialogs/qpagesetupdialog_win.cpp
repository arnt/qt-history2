/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpagesetupdialog.h"

#include <qapplication.h>

#include <private/qprintengine_win_p.h>
#include <private/qabstractpagesetupdialog_p.h>

class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
};

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{

}

int QPageSetupDialog::exec()
{
    Q_D(QPageSetupDialog);

    if (d->printer->outputFormat() != QPrinter::NativeFormat)
        return Rejected;

    QWin32PrintEngine *engine = static_cast<QWin32PrintEngine*>(d->printer->paintEngine());
    QWin32PrintEnginePrivate *ep = static_cast<QWin32PrintEnginePrivate *>(engine->d_ptr);

    PAGESETUPDLG psd;
    memset(&psd, 0, sizeof(PAGESETUPDLG));
    psd.lStructSize = sizeof(PAGESETUPDLG);
    psd.hDevMode = ep->devMode;
    HGLOBAL *tempDevNames = ep->createDevNames();
    psd.hDevNames = tempDevNames;

    QWidget *parent = parentWidget();
    parent = parent ? parent->window() : qApp->activeWindow();
    Q_ASSERT(!parent ||parent->testAttribute(Qt::WA_WState_Created));
    psd.hwndOwner = parent ? parent->winId() : 0;

    QRect paperRect = d->printer->paperRect();
    QRect pageRect = d->printer->pageRect();

    QRect marginRect = ep->getPageMargins();
    psd.rtMargin.left   = marginRect.left();
    psd.rtMargin.top    = marginRect.top();
    psd.rtMargin.right  = marginRect.width();
    psd.rtMargin.bottom = marginRect.height();

    psd.Flags = PSD_INHUNDREDTHSOFMILLIMETERS
                | PSD_MARGINS;

    bool result = PageSetupDlg(&psd);

    // ### margins too...

    if (result) {
        ep->readDevnames(psd.hDevNames);
        ep->readDevmode(psd.hDevMode);

        QRect theseMargins = QRect(psd.rtMargin.left, psd.rtMargin.top,
                                   psd.rtMargin.right, psd.rtMargin.bottom);

        if (theseMargins != marginRect) {
            ep->setPageMargins(psd.rtMargin.left,
                               psd.rtMargin.top,
                               psd.rtMargin.right,
                               psd.rtMargin.bottom);
        }
    }

    GlobalFree(tempDevNames);

    return result;
}

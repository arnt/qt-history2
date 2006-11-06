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

#ifndef QT_NO_PRINTDIALOG

#include "qprintdialog.h"

#include <qwidget.h>
#include <qapplication.h>
#include <private/qapplication_p.h>

#include <private/qabstractprintdialog_p.h>
#include <private/qprintengine_win_p.h>

extern void qt_win_eatMouseMove();

class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialog)
public:
    QPrintDialogPrivate()
        : ep(0)
    {
    }

    inline void _q_printToFileChanged(int) {}
    inline void _q_rbPrintRangeToggled(bool) {}
    inline void _q_printerChanged(int) {}
    inline void _q_paperSizeChanged(int) {}
    inline void _q_btnBrowseClicked() {}
    inline void _q_btnPropertiesClicked() {}

    QWin32PrintEnginePrivate *ep;
};

#ifndef Q_OS_TEMP
// If you change this function, make sure you also change the unicode equivalent
static PRINTDLGA *qt_win_make_PRINTDLGA(QWidget *parent, QPrintDialog *pdlg, QPrintDialogPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGA *pd = new PRINTDLGA;
    memset(pd, 0, sizeof(PRINTDLGA));
    pd->lStructSize = sizeof(PRINTDLGA);

    if (d->ep->devModeA() != 0) {
        int size = sizeof(DEVMODEA) + d->ep->devModeA()->dmDriverExtra;
        pd->hDevMode = GlobalAlloc(GHND, size);
        {
            void *dest = GlobalLock(pd->hDevMode);
            memcpy(dest, d->ep->devMode, size);
            GlobalUnlock(pd->hDevMode);
        }
    } else {
        pd->hDevMode = NULL;
    }
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!pdlg->isOptionEnabled(QPrintDialog::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (pdlg->isOptionEnabled(QPrintDialog::PrintPageRange)) {
        pd->nMinPage = pdlg->minPage();
        pd->nMaxPage = pdlg->maxPage();
    }

    if(!pdlg->isOptionEnabled(QPrintDialog::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (pdlg->printRange() == QPrintDialog::Selection)
        pd->Flags |= PD_SELECTION;
    else if (pdlg->printRange() == QPrintDialog::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->ep->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    Q_ASSERT(!parent ||parent->testAttribute(Qt::WA_WState_Created));
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = qMax(pdlg->fromPage(), pdlg->minPage());
    pd->nToPage   = qMin(pdlg->toPage(), pdlg->maxPage());
    pd->nCopies = d->ep->num_copies;

    return pd;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_clean_up_PRINTDLGA(PRINTDLGA **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_read_back_PRINTDLGA(PRINTDLGA *pd, QPrintDialog *pdlg, QPrintDialogPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        pdlg->setPrintRange(QPrintDialog::Selection);
        pdlg->setFromTo(0, 0);
    } else if (pd->Flags & PD_PAGENUMS) {
        pdlg->setPrintRange(QPrintDialog::PageRange);
        pdlg->setFromTo(pd->nFromPage, pd->nToPage);
    } else {
        pdlg->setPrintRange(QPrintDialog::AllPages);
        pdlg->setFromTo(0, 0);
    }

    d->ep->printToFile = (pd->Flags & PD_PRINTTOFILE) != 0;

    d->ep->readDevnames(pd->hDevNames);
    d->ep->readDevmode(pd->hDevMode);
}
#endif // Q_OS_TEMP

#ifdef UNICODE
// If you change this function, make sure you also change the ansi equivalent
static PRINTDLGW *qt_win_make_PRINTDLGW(QWidget *parent, QPrintDialog *pdlg, QPrintDialogPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGW *pd = new PRINTDLGW;
    memset(pd, 0, sizeof(PRINTDLGW));
    pd->lStructSize = sizeof(PRINTDLGW);

    if (d->ep->devModeW() != 0) {
        int size = sizeof(DEVMODEW) + d->ep->devModeW()->dmDriverExtra;
        pd->hDevMode = GlobalAlloc(GHND, size);
        {
            void *dest = GlobalLock(pd->hDevMode);
            memcpy(dest, d->ep->devMode, size);
            GlobalUnlock(pd->hDevMode);
        }
    } else {
        pd->hDevMode = NULL;
    }
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!pdlg->isOptionEnabled(QPrintDialog::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (pdlg->isOptionEnabled(QPrintDialog::PrintPageRange)) {
        pd->nMinPage = pdlg->minPage();
        pd->nMaxPage = pdlg->maxPage();
    }

    if(!pdlg->isOptionEnabled(QPrintDialog::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (pdlg->printRange() == QPrintDialog::Selection)
        pd->Flags |= PD_SELECTION;
    else if (pdlg->printRange() == QPrintDialog::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->ep->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    Q_ASSERT(!parent ||parent->testAttribute(Qt::WA_WState_Created));
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = qMax(pdlg->fromPage(), pdlg->minPage());
    pd->nToPage   = qMin(pdlg->toPage(), pdlg->maxPage());
    pd->nCopies = d->ep->num_copies;

    return pd;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_clean_up_PRINTDLGW(PRINTDLGW **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_read_back_PRINTDLGW(PRINTDLGW *pd, QPrintDialog *pdlg, QPrintDialogPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        pdlg->setPrintRange(QPrintDialog::Selection);
        pdlg->setFromTo(0, 0);
    } else if (pd->Flags & PD_PAGENUMS) {
        pdlg->setPrintRange(QPrintDialog::PageRange);
        pdlg->setFromTo(pd->nFromPage, pd->nToPage);
    } else {
        pdlg->setPrintRange(QPrintDialog::AllPages);
        pdlg->setFromTo(0, 0);
    }

    d->ep->printToFile = (pd->Flags & PD_PRINTTOFILE) != 0;

    d->ep->readDevnames(pd->hDevNames);
    d->ep->readDevmode(pd->hDevMode);

}
#endif // UNICODE

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog( *(new QPrintDialogPrivate), printer, parent)
{
    if (printer->outputFormat() == QPrinter::NativeFormat)
        d_func()->ep = static_cast<QWin32PrintEngine *>(printer->paintEngine())->d_func();

    else
        qWarning("QPrintDialog, Only native format supported");
}

QPrintDialog::~QPrintDialog()
{
    //nothing
}

int QPrintDialog::exec()
{
    if (printer()->outputFormat() == QPrinter::PdfFormat) {
        return false;
    }

    Q_D(QPrintDialog);

    QWidget *parent = parentWidget();
    if (parent)
        parent = parent->window();
    else
        parent = qApp->activeWindow();

    QWidget modal_widget;
    modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
    modal_widget.setParent(parent, Qt::Window);
    QApplicationPrivate::enterModal(&modal_widget);

    HGLOBAL *tempDevNames = d->ep->createDevNames();

    bool result;
    void *voidp;
    QT_WA({
        PRINTDLGW *pd = qt_win_make_PRINTDLGW(parent, this, d, tempDevNames);
        voidp = pd; // store until later
        result = PrintDlgW(pd);
        if (result && pd->hDC == 0)
            result = false;
    }, {
        PRINTDLGA *pd = qt_win_make_PRINTDLGA(parent, this, d, tempDevNames);
        voidp = pd; // store until later
        result = PrintDlgA(pd);
        if (result && pd->hDC == 0)
            result = false;
    });

    QApplicationPrivate::leaveModal(&modal_widget);

    qt_win_eatMouseMove();

    // write values back...
    if (result) {
        QT_WA({
            PRINTDLGW *pd = reinterpret_cast<PRINTDLGW *>(voidp);
            qt_win_read_back_PRINTDLGW(pd, this, d);
            qt_win_clean_up_PRINTDLGW(&pd);
        }, {
            PRINTDLGA *pd = reinterpret_cast<PRINTDLGA *>(voidp);
            qt_win_read_back_PRINTDLGA(pd, this, d);
            qt_win_clean_up_PRINTDLGA(&pd);
        });
    }

    // Cleanup...
    GlobalFree(tempDevNames);

    return result;
}

#include "moc_qprintdialog.cpp"

#endif // QT_NO_PRINTDIALOG

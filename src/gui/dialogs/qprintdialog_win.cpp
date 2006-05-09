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
    inline void _q_printerChanged(int index) {}
    inline void _q_paperSizeChanged(int index) {}
    inline void _q_btnBrowseClicked() {}
    inline void _q_btnPropertiesClicked() {}

    QWin32PrintEnginePrivate *ep;
};

#ifndef Q_OS_TEMP
// If you change this function, make sure you also change the unicode equivalent
static PRINTDLGA *qt_win_make_PRINTDLGA(QWidget *parent, QPrintDialogPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGA *pd = new PRINTDLGA;
    memset(pd, 0, sizeof(PRINTDLGA));
    pd->lStructSize = sizeof(PRINTDLGA);

    int size = sizeof(DEVMODEA) + d->ep->devModeA()->dmDriverExtra;
    pd->hDevMode = GlobalAlloc(GHND, size);
    {
        void *dest = GlobalLock(pd->hDevMode);
        memcpy(dest, d->ep->devMode, size);
        GlobalUnlock(pd->hDevMode);
    }
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!(d->options & QPrintDialog::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (d->options & QPrintDialog::PrintPageRange) {
        pd->nMinPage = d->minPage;
        pd->nMaxPage = d->maxPage;
    }

    if(!(d->options & QPrintDialog::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (d->printRange == QPrintDialog::Selection)
        pd->Flags |= PD_SELECTION;
    else if (d->printRange == QPrintDialog::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->ep->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = qMax(d->fromPage, d->minPage);
    pd->nToPage   = qMin(d->toPage, d->maxPage);
    pd->nCopies = d->ep->devModeA()->dmCopies;

    return pd;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_clean_up_PRINTDLGA(PRINTDLGA **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_read_back_PRINTDLGA(PRINTDLGA *pd, QPrintDialogPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        d->printRange = QPrintDialog::Selection;
        d->fromPage = 0;
        d->toPage = 0;
    } else if (pd->Flags & PD_PAGENUMS) {
        d->printRange = QPrintDialog::PageRange;
        d->fromPage = pd->nFromPage;
        d->toPage = pd->nToPage;
    } else {
        d->printRange = QPrintDialog::AllPages;
        d->fromPage = 0;
        d->toPage = 0;
    }

    d->ep->printToFile = (pd->Flags & PD_PRINTTOFILE) != 0;

    d->ep->readDevnames(pd->hDevNames);
    d->ep->readDevmode(pd->hDevMode);
}
#endif // Q_OS_TEMP

#ifdef UNICODE
// If you change this function, make sure you also change the ansi equivalent
static PRINTDLGW *qt_win_make_PRINTDLGW(QWidget *parent, QPrintDialogPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGW *pd = new PRINTDLGW;
    memset(pd, 0, sizeof(PRINTDLGW));
    pd->lStructSize = sizeof(PRINTDLGW);

    int size = sizeof(DEVMODEW) + d->ep->devModeW()->dmDriverExtra;
    pd->hDevMode = GlobalAlloc(GHND, size);
    {
        void *dest = GlobalLock(pd->hDevMode);
        memcpy(dest, d->ep->devMode, size);
        GlobalUnlock(pd->hDevMode);
    }
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!(d->options & QPrintDialog::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (d->options & QPrintDialog::PrintPageRange) {
        pd->nMinPage = d->minPage;
        pd->nMaxPage = d->maxPage;
    }

    if(!(d->options & QPrintDialog::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (d->printRange == QPrintDialog::Selection)
        pd->Flags |= PD_SELECTION;
    else if (d->printRange == QPrintDialog::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->ep->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = qMax(d->fromPage, d->minPage);
    pd->nToPage   = qMin(d->toPage, d->maxPage);
    pd->nCopies = d->ep->devModeW()->dmCopies;

    return pd;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_clean_up_PRINTDLGW(PRINTDLGW **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_read_back_PRINTDLGW(PRINTDLGW *pd, QPrintDialogPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        d->printRange = QPrintDialog::Selection;
        d->fromPage = 0;
        d->toPage = 0;
    } else if (pd->Flags & PD_PAGENUMS) {
        d->printRange = QPrintDialog::PageRange;
        d->fromPage = pd->nFromPage;
        d->toPage = pd->nToPage;
    } else {
        d->printRange = QPrintDialog::AllPages;
        d->fromPage = 0;
        d->toPage = 0;
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
    if (!d->ep->devMode) {
        qWarning("QPrintDialog::exec(), printer not initialized");
        return false;
    }

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
        PRINTDLGW *pd = qt_win_make_PRINTDLGW(parent, d, tempDevNames);
        voidp = pd; // store until later
        result = PrintDlgW(pd);
        if (result && pd->hDC == 0)
            result = false;
    }, {
        PRINTDLGA *pd = qt_win_make_PRINTDLGA(parent, d, tempDevNames);
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
            qt_win_read_back_PRINTDLGW(pd, d);
            qt_win_clean_up_PRINTDLGW(&pd);
        }, {
            PRINTDLGA *pd = reinterpret_cast<PRINTDLGA *>(voidp);
            qt_win_read_back_PRINTDLGA(pd, d);
            qt_win_clean_up_PRINTDLGA(&pd);
        });
    }

    // Cleanup...
    GlobalFree(tempDevNames);

    return result;
}

#include "moc_qprintdialog.cpp"

#endif // QT_NO_PRINTDIALOG

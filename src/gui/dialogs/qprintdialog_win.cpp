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

#include "qabstractprintdialog_p.h"
#include "qprintdialog_win.h"
#include "qprintengine_win.h"

#include <private/qprintengine_win_p.h>

#include <qwidget.h>
#include <qapplication.h>

extern Q_GUI_EXPORT void qt_leave_modal(QWidget *);
extern Q_GUI_EXPORT void qt_enter_modal(QWidget *);

class QPrintDialogWinPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialogWin)
public:
    QPrintDialogWinPrivate()
        : ep(0)
    {
    }

    HGLOBAL *createDevNames();

    QWin32PrintEnginePrivate *ep;
};

#define d d_func()

QPrintDialogWin::QPrintDialogWin(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog( *(new QPrintDialogWinPrivate), printer, parent)
{
    d->ep = static_cast<QWin32PrintEngine *>(printer->paintEngine())->d;
}

int QPrintDialogWin::exec()
{
    QWidget *parent = parentWidget();
    if (parent)
        parent = parent->topLevelWidget();
    else
        parent = qApp->mainWidget();

    if (parent) {
	QEvent e(QEvent::WindowBlocked);
	QApplication::sendEvent(parent, &e);
	qt_enter_modal(parent);
    }

    PRINTDLG pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);

    Q_ASSERT(d->ep->devMode);
    pd.hDevMode = d->ep->devMode;

    HGLOBAL *tempDevNames = d->createDevNames();
    pd.hDevNames  = tempDevNames;

    pd.Flags = PD_RETURNDC;
    pd.Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!(d->options & PrintSelection))
        pd.Flags |= PD_NOSELECTION;
    if (d->options & PrintPageRange) {
        pd.nMinPage = d->minPage;
        pd.nMaxPage = d->maxPage;
    }

    if(!(d->options & PrintToFile))
        pd.Flags |= PD_DISABLEPRINTTOFILE;

    if (d->printRange == Selection)
        pd.Flags |= PD_SELECTION;
    else if (d->printRange == PageRange)
        pd.Flags |= PD_PAGENUMS;
    else
        pd.Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd.nMinPage==0 && pd.nMaxPage==0)
        pd.Flags |= PD_NOPAGENUMS;

    if (d->ep->printToFile)
        pd.Flags |= PD_PRINTTOFILE;
    pd.hwndOwner = parent ? parent->winId() : 0;
    pd.nFromPage = qMax(d->fromPage, d->minPage);
    pd.nToPage   = qMin(d->toPage, d->maxPage);
    pd.nCopies   = d->ep->devMode->dmCopies;

    bool result = PrintDlg(&pd);
    if (result && pd.hDC == 0)
        result = FALSE;

    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    // write values back...
    if (result) {
        if (pd.Flags & PD_SELECTION) {
            d->printRange = Selection;
            d->fromPage = 0;
            d->toPage = 0;
        } else if (pd.Flags & PD_PAGENUMS) {
            d->printRange = PageRange;
            d->fromPage = pd.nFromPage;
            d->toPage = pd.nToPage;
        } else {
            d->printRange = AllPages;
            d->fromPage = d->minPage;
            d->toPage = d->maxPage;
        }

        d->ep->printToFile = (pd.Flags & PD_PRINTTOFILE) != 0;

        if (pd.hDevNames) {
            DEVNAMES *dn = (DEVNAMES*) GlobalLock(pd.hDevNames);
            d->ep->name = QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset);
            d->ep->port = QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset);
            d->ep->program = QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset);
            GlobalUnlock(pd.hDevNames);
        }

        if (pd.hDevMode) {
            DEVMODE *dm = (DEVMODE*) GlobalLock(pd.hDevMode);
            d->ep->release();
            d->ep->globalDevMode = pd.hDevMode;
            d->ep->devMode = dm;
            d->ep->hdc = CreateDC(d->ep->program.utf16(), d->ep->name.utf16(), 0, dm);
            d->ep->setupPrinterMapping();
        }
    }

    // Cleanup...
    GlobalFree(tempDevNames);

    return result;
}

HGLOBAL *QPrintDialogWinPrivate::createDevNames()
{
    int size = sizeof(DEVNAMES)
               + ep->program.length() * 2 + 2
               + ep->name.length() * 2 + 2
               + ep->port.length() * 2 + 2;
    HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
    DEVNAMES *dn = (DEVNAMES*) GlobalLock(hGlobal);

    dn->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
    dn->wDeviceOffset = dn->wDriverOffset + ep->program.length() + 1;
    dn->wOutputOffset = dn->wDeviceOffset + ep->name.length() + 1;

    memcpy((ushort*)dn + dn->wDriverOffset, ep->program.utf16(), ep->program.length() * 2 + 2);
    memcpy((ushort*)dn + dn->wDeviceOffset, ep->name.utf16(), ep->name.length() * 2 + 2);
    memcpy((ushort*)dn + dn->wOutputOffset, ep->port.utf16(), ep->port.length() * 2 + 2);
    dn->wDefault = 0;

#if defined QT_PRINTDIALOG_DEBUG
    printf("QPrintDialogWinPrivate::createDevNames()\n"
           " -> wDriverOffset: %d\n"
           " -> wDeviceOffset: %d\n"
           " -> wOutputOffset: %d\n",
           dn->wDriverOffset,
           dn->wDeviceOffset,
           dn->wOutputOffset);

    printf("QPrintDialogWinPrivate::createDevNames(): %s, %s, %s\n",
           QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset).latin1(),
           QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset).latin1(),
           QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset).latin1());
#endif // QT_PRINTDIALOG_DEBUG

    GlobalUnlock(hGlobal);

    return hGlobal;
}

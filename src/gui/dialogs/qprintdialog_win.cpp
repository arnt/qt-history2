/****************************************************************************
**
** Definition of QPrintDialogWin class.
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
    pd.hDevMode   = d->ep->devMode;
    pd.hDevNames  = d->ep->devNames;

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

    if (d->pageRange == Selection)
        pd.Flags |= PD_SELECTION;
    else if (d->pageRange == Pages)
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
            d->pageRange = Selection;
            d->fromPage = 0;
            d->toPage = 0;
        } else if (pd.Flags & PD_PAGENUMS) {
            d->pageRange = Pages;
            d->fromPage = pd.nFromPage;
            d->toPage = pd.nToPage;
        } else {
            d->pageRange = All;
            d->fromPage = d->minPage;
            d->toPage = d->maxPage;
        }
    }

    return result;
}

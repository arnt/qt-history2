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

    inline void browseClicked() {}
    inline void okClicked() {}
    inline void printerOrFileSelected(QAbstractButton *) {}
    inline void landscapeSelected(int) {}
    inline void paperSizeSelected(int) {}
    inline void orientSelected(int) {}
    inline void pageOrderSelected(QAbstractButton *) {}
    inline void colorModeSelected(QAbstractButton *) {}
    inline void setNumCopies(int) {}
    inline void printRangeSelected(QAbstractButton *) {}
    inline void setFirstPage(int) {}
    inline void setLastPage(int) {}
    inline void fileNameEditChanged(const QString &) {}

    QWin32PrintEnginePrivate *ep;
};

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog( *(new QPrintDialogPrivate), printer, parent)
{
    d_func()->ep = static_cast<QWin32PrintEngine *>(printer->paintEngine())->d_func();
}

QPrintDialog::~QPrintDialog()
{
    //nothing
}

int QPrintDialog::exec()
{
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

    if (parent) {
	QEvent e(QEvent::WindowBlocked);
	QApplication::sendEvent(parent, &e);
        QApplicationPrivate::enterModal(parent);
    }

    PRINTDLG pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);

    pd.hDevMode = d->ep->devMode;

    HGLOBAL *tempDevNames = d->ep->createDevNames();
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
    QT_WA( {
        pd.nCopies = d->ep->devModeW()->dmCopies;
    }, {
        pd.nCopies = d->ep->devModeA()->dmCopies;
    } );

    bool result = PrintDlg(&pd);
    if (result && pd.hDC == 0)
        result = false;

    if (parent) {
        QApplicationPrivate::leaveModal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    qt_win_eatMouseMove();

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

        d->ep->readDevnames(pd.hDevNames);
        d->ep->readDevmode(pd.hDevMode);
    }

    // Cleanup...
    GlobalFree(tempDevNames);

    return result;
}

#include "moc_qprintdialog.cpp"

#endif // QT_NO_PRINTDIALOG

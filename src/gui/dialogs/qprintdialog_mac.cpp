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
#include <private/qt_mac_p.h>

#include <qprintdialog.h>
#include <private/qapplication_p.h>
#include <private/qabstractprintdialog_p.h>
#include <private/qprintengine_mac_p.h>

class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialog)
public:
    QPrintDialogPrivate() : ep(0) { }

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
    inline void fileNameEditChanged(const QString & /*text*/) {}

    QMacPrintEnginePrivate *ep;
};

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogPrivate), printer, parent)
{
    Q_D(QPrintDialog);
    d->ep = static_cast<QMacPrintEngine *>(printer->paintEngine())->d_func();
}

QPrintDialog::~QPrintDialog()
{
    //nothing
}

int QPrintDialog::exec()
{
    Q_D(QPrintDialog);
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
    { //simulate modality
	QWidget modal_widg(0, Qt::Window);
        modal_widg.setObjectName(QLatin1String(__FILE__ "__modal_dlg"));
	QApplicationPrivate::enterModal(&modal_widg);
        PMSessionPrintDialog(d->ep->session, d->ep->settings, d->ep->format, &result);
	QApplicationPrivate::leaveModal(&modal_widg);
    }
    if (result) {
        UInt32 page;
        PMGetFirstPage(d->ep->settings, &page);
        d->fromPage = qMax(UInt32(0), page);
        PMGetLastPage(d->ep->settings, &page);
        d->toPage = qMin(UInt32(INT_MAX), page);
        d->printRange = PageRange; // In a way a lie, but it shouldn't hurt.
        // Carbon hands us back a very large number here even for ALL, set it to max
        // in that case to follow the behavior of the other print dialogs.
        if (d->maxPage < d->toPage)
            d->toPage = d->maxPage;
        // Keep us in sync with file output
        PMDestinationType dest;
        PMSessionGetDestinationType(d->ep->session, d->ep->settings, &dest);
        if (dest == kPMDestinationFile) {
            QCFType<CFURLRef> file;
            PMSessionCopyDestinationLocation(d->ep->session, d->ep->settings, &file);
            UInt8 localFile[255];  // Assuming there's a POSIX file system here.
            CFURLGetFileSystemRepresentation(file, true, localFile, sizeof(localFile));
            d->ep->outputFilename
                = QString::fromLocal8Bit(reinterpret_cast<const char *>(localFile));
        }
    }
    return result;
}

#include "moc_qprintdialog.cpp"

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

#include <qhash.h>
#include <private/qapplication_p.h>
#include <private/qprintengine_mac_p.h>
#include <private/qabstractpagesetupdialog_p.h>

class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
    Q_DECLARE_PUBLIC(QPageSetupDialog)
public:
    QPageSetupDialogPrivate() : ep(0), upp(0), sheetBlocks(false), acceptStatus(false) {}
    ~QPageSetupDialogPrivate() {
        if (upp) {
            DisposePMSheetDoneUPP(upp);
            upp = 0;
        }
        sheetCallbackMap.remove(ep->session);
    }
    static void pageSetupDialogSheetDoneCallback(PMPrintSession printSession, WindowRef /*documentWindow*/, Boolean accepted) {
        QPageSetupDialogPrivate *priv = sheetCallbackMap.value(printSession);
        if (!priv) {
            qWarning("%s:%d: QPageSetupDialog::exec: Could not retrieve data structure, "
                     "you most likely now have an infinite modal loop", __FILE__, __LINE__);
            return;
        }
        priv->sheetBlocks = false;
        priv->acceptStatus = accepted;
    }
    QMacPrintEnginePrivate *ep;
    PMSheetDoneUPP upp;
    bool sheetBlocks;
    Boolean acceptStatus;
    static QHash<PMPrintSession, QPageSetupDialogPrivate*> sheetCallbackMap;
};

QHash<PMPrintSession, QPageSetupDialogPrivate*> QPageSetupDialogPrivate::sheetCallbackMap;

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{
    Q_D(QPageSetupDialog);
    QMacPrintEngine *engine = static_cast<QMacPrintEngine *>(d->printer->paintEngine());
    d->ep = static_cast<QMacPrintEnginePrivate *>(engine->d_ptr);
}

int QPageSetupDialog::exec()
{
    Q_D(QPageSetupDialog);

    if (d->printer->outputFormat() != QPrinter::NativeFormat)
        return Rejected;

    QMacPrintEngine *engine = static_cast<QMacPrintEngine *>(d->printer->paintEngine());
    QMacPrintEnginePrivate *ep = static_cast<QMacPrintEnginePrivate *>(engine->d_ptr);

    // If someone is reusing a QPrinter object, the end released all our old
    // information. In this case, we must reinitialize.
    if (ep->session == 0)
        ep->initialize();

    { //simulate modality
        // First, see if we should use a sheet.
        QWidget *parent = parentWidget();
        if (parent && parent->isVisible()) {
            WindowRef windowRef = qt_mac_window_for(parent);
            WindowClass wclass;
            GetWindowClass(windowRef, &wclass);
            if (!isOptionEnabled(QPageSetupDialog::DontUseSheet)
                    && (wclass == kDocumentWindowClass || wclass == kFloatingWindowClass
                        || wclass == kMovableModalWindowClass)) {
                // Yes, we can use a sheet
                if (!d->upp)
                    d->upp = NewPMSheetDoneUPP(QPageSetupDialogPrivate::pageSetupDialogSheetDoneCallback);
                d->sheetCallbackMap.insert(d->ep->session, d);
                PMSessionUseSheets(d->ep->session, qt_mac_window_for(parentWidget()), d->upp);
                d->sheetBlocks = true;
            }
        }
	QWidget modal_widg(0, Qt::Window);
        modal_widg.setObjectName(QLatin1String(__FILE__ "__modal_dlg"));
        modal_widg.createWinId();
	QApplicationPrivate::enterModal(&modal_widg);
        PMSessionPageSetupDialog(ep->session, ep->format, &d->acceptStatus);
        while (d->sheetBlocks) {
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        }
	QApplicationPrivate::leaveModal(&modal_widg);
    }
    return d->acceptStatus ? Accepted : Rejected;
}
